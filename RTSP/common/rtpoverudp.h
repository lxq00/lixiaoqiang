#pragma once
#include "rtp.h"
#include "RTSPProtocol.h"
#include "RTSP/RTSPStructs.h"

#define MAXUDPPACKGELEN		56*1024

#define MAXRTPFRAMESIZE		100

class rtpOverUdp :public rtp
{
	struct FrameInfo
	{
		RTSPBuffer		framedata;
		uint16_t		sn;
		uint32_t		tiemstmap;
		bool			mark;

		bool operator < (const FrameInfo& info) const
		{
			return sn < info.sn;
		}
	};
	struct SendDataInfo
	{
		std::string prevdata;
		RTSPBuffer	framedata;

		const char*	bufferaddr;
		uint32_t	bufferlen;

		SendDataInfo():bufferaddr(NULL),bufferlen(0){}
	};
public:
	rtpOverUdp(bool _isserver, const shared_ptr<IOWorker>& work,const std::string& _dstaddr,const RTSP_MEDIA_INFO& _rtspmedia, const RTPDataCallback& _datacallback)
		:rtp(_isserver,_rtspmedia,_datacallback), rtpvideosn(0), rtpaudiosn(0), dstaddr(_dstaddr)
	{
		if (rtspmedia.media.bHasVideo && rtspmedia.videoTransport.rtp.u.server_port1 != 0 && rtspmedia.videoTransport.rtp.u.client_port1 != 0)
		{
			videortp = UDP::create(work);
			videortp->bind(isserver ? rtspmedia.videoTransport.rtp.u.server_port1 : rtspmedia.videoTransport.rtp.u.client_port1);
			videortp->setSocketBuffer(1024*1024, 0);

			char* videortpbuffer = currvideortprecvbuffer.alloc(MAXUDPPACKGELEN);
			videortp->async_recvfrom(videortpbuffer, MAXUDPPACKGELEN, Socket::RecvFromCallback(&rtpOverUdp::socketVideoRTPRecvCallback, this));

			videortcp = UDP::create(work);
			videortcp->bind(isserver ? rtspmedia.videoTransport.rtp.u.server_port2 : rtspmedia.videoTransport.rtp.u.client_port2);
			videortcp->setSocketBuffer(1024 * 1024, 0);

			char* videortcpbuffer = currvideortcprecvbuffer.alloc(MAXUDPPACKGELEN);
			videortcp->async_recvfrom(videortcpbuffer, MAXUDPPACKGELEN, Socket::RecvFromCallback(&rtpOverUdp::socketVideoRTCPRecvCallback, this));
		}

		if (rtspmedia.media.bHasAudio && rtspmedia.audioTransport.rtp.u.server_port1 != 0 && rtspmedia.audioTransport.rtp.u.client_port1 != 0)
		{
			audiortp = UDP::create(work);
			audiortp->bind(isserver ? rtspmedia.audioTransport.rtp.u.server_port1 : rtspmedia.audioTransport.rtp.u.client_port1);
			audiortp->setSocketBuffer(1024 * 1024, 0);

			char* audiortpbuffer = curraudiortprecvbuffer.alloc(MAXUDPPACKGELEN);
			audiortp->async_recvfrom(audiortpbuffer, MAXUDPPACKGELEN, Socket::RecvFromCallback(&rtpOverUdp::socketAudioRTPRecvCallback, this));

			audiortcp = UDP::create(work);
			audiortcp->bind(isserver ? rtspmedia.audioTransport.rtp.u.server_port2 : rtspmedia.audioTransport.rtp.u.client_port2);
			audiortcp->setSocketBuffer(1024 * 1024, 0);

			char* audiortcpbuffer = curraudiortcprecvbuffer.alloc(MAXUDPPACKGELEN);
			audiortcp->async_recvfrom(audiortcpbuffer, MAXUDPPACKGELEN, Socket::RecvFromCallback(&rtpOverUdp::socketAudioRTCPRecvCallback, this));
		}
	}
	~rtpOverUdp()
	{
		if(videortp) videortp->disconnect();
		if(videortcp) videortcp->disconnect();
		if (audiortp) audiortp->disconnect();
		if (audiortcp) audiortcp->disconnect();

		videortp = NULL;
		videortcp = NULL;
		audiortp = NULL;
		audiortcp = NULL;
	}
	
	void sendData(bool isvideo, uint32_t timestmap, const RTSPBuffer& data, bool mark)
	{
		if (isvideo && !rtspmedia.media.bHasVideo) return;
		if (!isvideo && !rtspmedia.media.bHasAudio) return;

		const char* buffer = data.buffer();
		uint32_t bufferlen = data.size();

		while (bufferlen > 0)
		{
			uint32_t cansendlen = min(MAXRTPPACKETLEN, bufferlen);

			RTPHEADER header;
			memset(&header, 0, sizeof(header));
			header.v = 2;
			header.ts = htonl(timestmap);
			header.seq = htons(isvideo ? rtpvideosn++ : rtpaudiosn++ );
			header.pt = isvideo ? rtspmedia.media.stStreamVideo.nPayLoad : rtspmedia.media.stStreamAudio.nPayLoad;
			header.m = bufferlen == cansendlen ? mark : false;
			header.ssrc = htonl(isvideo ? rtspmedia.videoTransport.ssrc : rtspmedia.audioTransport.ssrc);

			std::string prevdata = std::string((const char*)& header, sizeof(RTPHEADER));
			
			{
				Guard locker(mutex);

				shared_ptr<SendDataInfo> senddata = make_shared<SendDataInfo>();
				senddata->framedata = RTSPBuffer(data, buffer, cansendlen);
				senddata->prevdata = prevdata;

				std::list<shared_ptr<SendDataInfo> >& sendlist = isvideo ? sendvideortplist : sendaudiortplist;
				sendlist.push_back(senddata);

				_sendAndCheckSend(isvideo);
			}

			buffer += cansendlen;
			bufferlen -= cansendlen;
		}
	}
	void _sendAndCheckSend(bool isvideo, const char* buffer = NULL,size_t len = 0)
	{
		std::list<shared_ptr<SendDataInfo> >& sendlist = isvideo ? sendvideortplist : sendaudiortplist;


		bool needSendData = false;
		//第一次需要发送数据
		if (buffer == NULL && sendlist.size() == 1)
		{
			needSendData = true;
		}
		else if (buffer)
		{
			if (len < 0) return;

			if (sendlist.size() <= 0) return;

			{
				shared_ptr<SendDataInfo>& item = sendlist.front();
				if (buffer != item->bufferaddr) return;
				sendlist.pop_front();
			}

			needSendData = true;
		}

		if (!needSendData) return;

		if (sendlist.size() <= 0) return;

		shared_ptr<SendDataInfo>& item = sendlist.front();

		//处理数据的零拷贝问题，添加前置数据
		if (item->bufferaddr == NULL)
		{
			item->bufferaddr = item->framedata.buffer();
			item->bufferlen = item->framedata.size();

			if (item->prevdata.length() > 0)
			{
				if (item->bufferaddr - item->framedata.dataptr().c_str() < RTSPBUFFERHEADSPACE)
				{
					assert(0);
				}
				if (item->prevdata.length() > RTSPBUFFERHEADSPACE)
				{
					assert(0);
				}

				item->bufferaddr -= item->prevdata.length();
				item->bufferlen += item->prevdata.length();
				memcpy((char*)item->bufferaddr, item->prevdata.c_str(), item->prevdata.length());
			}
		}

		const char* sendbuffer = item->bufferaddr;
		uint32_t sendbufferlen = item->bufferlen;

		if (isvideo)
		{
			shared_ptr<Socket> socktmp = videortp;
			if (socktmp)
				socktmp->async_sendto(sendbuffer, sendbufferlen,
					NetAddr(dstaddr, isserver ? rtspmedia.videoTransport.rtp.u.client_port1 : rtspmedia.videoTransport.rtp.u.server_port1),
					Socket::SendedCallback(&rtpOverUdp::socketSendVideoRTPCallback, this));
		}
		else
		{
			shared_ptr<Socket> socktmp = audiortp;
			if (socktmp)
				socktmp->async_sendto(sendbuffer, sendbufferlen,
					NetAddr(dstaddr, isserver ? rtspmedia.audioTransport.rtp.u.client_port1 : rtspmedia.audioTransport.rtp.u.server_port1),
					Socket::SendedCallback(&rtpOverUdp::socketSendAudioRTPCallback, this));
		}
	}
	void socketSendVideoRTPCallback(const weak_ptr<Socket>& sock, const char* buffer, int len)
	{
		Guard locker(mutex);

		_sendAndCheckSend(true, buffer, len);
	}
	void socketSendAudioRTPCallback(const weak_ptr<Socket>& sock, const char* buffer, int len)
	{
		Guard locker(mutex);

		_sendAndCheckSend(false, buffer, len);
	}
	void socketVideoRTPRecvCallback(const weak_ptr<Socket>& sock, const char* buffer, int len,const NetAddr& otearaddr)
	{
		if (buffer != currvideortprecvbuffer.buffer() || len <= 0 || len > MAXUDPPACKGELEN) return;

		if (len > sizeof(RTPHEADER))
		{
			currvideortprecvbuffer.resize(len);

			Guard locker(mutex);

			if (otearaddr.getPort() != (isserver ? rtspmedia.videoTransport.rtp.u.client_port1 : rtspmedia.videoTransport.rtp.u.server_port1))
			{
				assert(0);
			}

			RTPHEADER* header = (RTPHEADER*)buffer;

			FrameInfo info;
			info.framedata = currvideortprecvbuffer;
			info.mark = header->m;
			info.sn = ntohs(header->seq);
			info.tiemstmap = ntohl(header->ts);

			videoframelist.push_back(info);

			_checkFramelistData(true);
		}
		
		shared_ptr<Socket> socktmp = sock.lock();
		if (socktmp)
		{
			char* buffertmp = currvideortprecvbuffer.alloc(MAXUDPPACKGELEN);
			socktmp->async_recvfrom(buffertmp, MAXUDPPACKGELEN, Socket::RecvFromCallback(&rtpOverUdp::socketVideoRTPRecvCallback, this));
		}
	}
	void socketVideoRTCPRecvCallback(const weak_ptr<Socket>& sock, const char* buffer, int len, const NetAddr& otearaddr)
	{
		shared_ptr<Socket> socktmp = sock.lock();
		if (socktmp)
		{
			char* buffertmp = currvideortcprecvbuffer.alloc(MAXUDPPACKGELEN);
			socktmp->async_recvfrom(buffertmp, MAXUDPPACKGELEN, Socket::RecvFromCallback(&rtpOverUdp::socketVideoRTCPRecvCallback, this));
		}
	}
	void socketAudioRTPRecvCallback(const weak_ptr<Socket>& sock, const char* buffer, int len, const NetAddr& otearaddr)
	{
		if (buffer != curraudiortprecvbuffer.buffer() || len <= 0 || len > MAXUDPPACKGELEN) return;

		if(len > sizeof(RTPHEADER))
		{
			curraudiortprecvbuffer.resize(len);

			Guard locker(mutex);

			if (otearaddr.getPort() != (isserver ? rtspmedia.audioTransport.rtp.u.client_port1 : rtspmedia.audioTransport.rtp.u.server_port1))
			{
				assert(0);
			}

			RTPHEADER* header = (RTPHEADER*)buffer;

			FrameInfo info;
			info.framedata = curraudiortprecvbuffer;
			info.mark = header->m;
			info.sn = ntohs(header->seq);
			info.tiemstmap = ntohl(header->ts);

			audioframelist.push_back(info);

			_checkFramelistData(false);
		}

		shared_ptr<Socket> socktmp = sock.lock();
		if (socktmp)
		{
			char* buffertmp = curraudiortprecvbuffer.alloc(MAXUDPPACKGELEN);
			socktmp->async_recvfrom(buffertmp, MAXUDPPACKGELEN, Socket::RecvFromCallback(&rtpOverUdp::socketAudioRTPRecvCallback, this));
		}
	}
	void socketAudioRTCPRecvCallback(const weak_ptr<Socket>& sock, const char* buffer, int len, const NetAddr& otearaddr)
	{
		shared_ptr<Socket> socktmp = sock.lock();
		if (socktmp)
		{
			char* buffertmp = curraudiortcprecvbuffer.alloc(MAXUDPPACKGELEN);
			socktmp->async_recvfrom(buffertmp, MAXUDPPACKGELEN, Socket::RecvFromCallback(&rtpOverUdp::socketAudioRTCPRecvCallback, this));
		}
	}
	void _checkFramelistData(bool isvideo)
	{
		std::list<FrameInfo>& framelist = isvideo ? videoframelist : audioframelist;

		framelist.sort();

		for (std::list<FrameInfo>::iterator iter = framelist.begin(); iter != framelist.end();)
		{
			std::list<FrameInfo>::iterator nextiter = iter;
			nextiter++;

			if (nextiter == framelist.end()) break;

			if (iter->sn + 1 == nextiter->sn || framelist.size() >= MAXRTPFRAMESIZE)
			{
				const char* framedataaddr = iter->framedata.buffer() + sizeof(RTPHEADER);
				size_t framedatasize = iter->framedata.size() - sizeof(RTPHEADER);

				datacallback(isvideo, iter->tiemstmap, RTSPBuffer(iter->framedata, framedataaddr, framedatasize), iter->mark);

				framelist.erase(iter++);
			}
			else
			{
				break;
			}
		}
	}
private:
	Mutex					 mutex;
	uint16_t				 rtpvideosn;
	uint16_t				 rtpaudiosn;
	std::string				 mediadata;

	shared_ptr<Socket>		 videortp;
	shared_ptr<Socket>		 videortcp;
	shared_ptr<Socket>		 audiortp;
	shared_ptr<Socket>		 audiortcp;

	TRANSPORT_INFO			 dstvideotransport;
	TRANSPORT_INFO			 dstaudiotransport;

	RTSPBuffer				currvideortprecvbuffer;
	RTSPBuffer				currvideortcprecvbuffer;

	std::list<FrameInfo>	 videoframelist;
	std::list<shared_ptr<SendDataInfo> >	 sendvideortplist;

	RTSPBuffer				curraudiortprecvbuffer;
	RTSPBuffer				curraudiortcprecvbuffer;

	std::list<FrameInfo>	 audioframelist;
	std::list<shared_ptr<SendDataInfo> >	 sendaudiortplist;


	std::string				 dstaddr;
};