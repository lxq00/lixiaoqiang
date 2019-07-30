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
		String			framedata;
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
		std::string framedata;
		uint32_t	havesendlen;

		SendDataInfo():havesendlen(0){}
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

			currvideortprecvbuffer.alloc(MAXUDPPACKGELEN);
			videortp->async_recvfrom((char*)currvideortprecvbuffer.c_str(), MAXUDPPACKGELEN, Socket::RecvFromCallback(&rtpOverUdp::socketVideoRTPRecvCallback, this));

			videortcp = UDP::create(work);
			videortcp->bind(isserver ? rtspmedia.videoTransport.rtp.u.server_port2 : rtspmedia.videoTransport.rtp.u.client_port2);
			videortcp->setSocketBuffer(1024 * 1024, 0);

			currvideortcprecvbuffer.alloc(MAXUDPPACKGELEN);
			videortcp->async_recvfrom((char*)currvideortcprecvbuffer.c_str(), MAXUDPPACKGELEN, Socket::RecvFromCallback(&rtpOverUdp::socketVideoRTCPRecvCallback, this));
		}

		if (rtspmedia.media.bHasAudio && rtspmedia.audioTransport.rtp.u.server_port1 != 0 && rtspmedia.audioTransport.rtp.u.client_port1 != 0)
		{
			audiortp = UDP::create(work);
			audiortp->bind(isserver ? rtspmedia.audioTransport.rtp.u.server_port1 : rtspmedia.audioTransport.rtp.u.client_port1);
			audiortp->setSocketBuffer(1024 * 1024, 0);

			curraudiortprecvbuffer.alloc(MAXUDPPACKGELEN);
			audiortp->async_recvfrom((char*)curraudiortprecvbuffer.c_str(), MAXUDPPACKGELEN, Socket::RecvFromCallback(&rtpOverUdp::socketAudioRTPRecvCallback, this));

			audiortcp = UDP::create(work);
			audiortcp->bind(isserver ? rtspmedia.audioTransport.rtp.u.server_port2 : rtspmedia.audioTransport.rtp.u.client_port2);
			audiortcp->setSocketBuffer(1024 * 1024, 0);

			curraudiortcprecvbuffer.alloc(MAXUDPPACKGELEN);
			audiortcp->async_recvfrom((char*)curraudiortcprecvbuffer.c_str(), MAXUDPPACKGELEN, Socket::RecvFromCallback(&rtpOverUdp::socketAudioRTCPRecvCallback, this));
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
	
	void sendData(bool isvideo, uint32_t timestmap, const char* buffer, uint32_t bufferlen, bool mark)
	{
		if (isvideo && !rtspmedia.media.bHasVideo) return;
		if (!isvideo && !rtspmedia.media.bHasAudio) return;

		if (buffer == NULL || bufferlen <= 0) return;

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

			std::string sendbuffer = std::string((const char*)& header, sizeof(RTPHEADER)) + std::string(buffer, cansendlen);

			addFrameAndCheckSend(isvideo, sendbuffer);

			buffer += cansendlen;
			bufferlen -= cansendlen;
		}
	}
	void addFrameAndCheckSend(bool isvideo, const std::string& frame)
	{
		Guard locker(mutex);
		std::list<shared_ptr<SendDataInfo> >& sendlist = isvideo ? sendvideortplist : sendaudiortplist;

		shared_ptr<SendDataInfo> senddata = make_shared<SendDataInfo>();
		senddata->framedata = std::move(frame);

		sendlist.push_back(senddata);

		if (sendlist.size() == 1)
		{
			shared_ptr<SendDataInfo>& framedata = sendlist.front();

			if (isvideo)
			{
				shared_ptr<Socket> socktmp = videortp;
				if (socktmp)
					socktmp->async_sendto(framedata->framedata.c_str(), framedata->framedata.length(),
						NetAddr(dstaddr, isserver ? rtspmedia.videoTransport.rtp.u.client_port1 : rtspmedia.videoTransport.rtp.u.server_port1),
						Socket::SendedCallback(&rtpOverUdp::socketSendVideoRTPCallback,this));
			}
			else
			{
				shared_ptr<Socket> socktmp = audiortp;
				if (socktmp)
					socktmp->async_sendto(framedata->framedata.c_str(), framedata->framedata.length(),
						NetAddr(dstaddr, isserver ? rtspmedia.audioTransport.rtp.u.client_port1 : rtspmedia.audioTransport.rtp.u.server_port1),
						Socket::SendedCallback(&rtpOverUdp::socketSendAudioRTPCallback, this));
			}
		}
	}
	void socketSendVideoRTPCallback(const weak_ptr<Socket>& sock, const char* buffer, int len)
	{
		socketSendAndCheck(true, buffer, len);
	}
	void socketSendAudioRTPCallback(const weak_ptr<Socket>& sock, const char* buffer, int len)
	{
		socketSendAndCheck(false, buffer, len);
	}
	void socketSendAndCheck(bool isvideo, const char* buffer, int len)
	{
		Guard locker(mutex);
		std::list<shared_ptr<SendDataInfo> >& sendlist = isvideo ? sendvideortplist : sendaudiortplist;

		if (sendlist.size() <= 0) return;

		shared_ptr<SendDataInfo>& framedata = sendlist.front();
		if (buffer < framedata->framedata.c_str() || buffer > framedata->framedata.c_str() + framedata->framedata.length())
		{
			assert(0);
			return;
		}
		if (framedata->havesendlen + len > framedata->framedata.length())
		{
			assert(0);
			return;
		}

		framedata->havesendlen += len;
		if (framedata->havesendlen == framedata->framedata.length())
		{
			sendlist.pop_front();

			if (sendlist.size() <= 0) return;

			framedata = sendlist.front();
		}


		if (isvideo)
		{
			shared_ptr<Socket> socktmp = videortp;
			if (socktmp)
				socktmp->async_sendto(framedata->framedata.c_str() + framedata->havesendlen, framedata->framedata.length() - framedata->havesendlen,
					NetAddr(dstaddr, isserver ? rtspmedia.videoTransport.rtp.u.client_port1 : rtspmedia.videoTransport.rtp.u.server_port1),
					Socket::SendedCallback(&rtpOverUdp::socketSendVideoRTPCallback, this));
		}
		else
		{
			shared_ptr<Socket> socktmp = audiortp;
			if (socktmp)
				socktmp->async_sendto(framedata->framedata.c_str() + framedata->havesendlen, framedata->framedata.length() - framedata->havesendlen,
					NetAddr(dstaddr, isserver ? rtspmedia.audioTransport.rtp.u.client_port1 : rtspmedia.audioTransport.rtp.u.server_port1),
					Socket::SendedCallback(&rtpOverUdp::socketSendAudioRTPCallback, this));
		}
	}
	void socketVideoRTPRecvCallback(const weak_ptr<Socket>& sock, const char* buffer, int len,const NetAddr& otearaddr)
	{
		if (buffer != currvideortprecvbuffer.c_str() || len <= 0 || len > MAXUDPPACKGELEN) return;

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
			currvideortprecvbuffer.alloc(MAXUDPPACKGELEN);
			socktmp->async_recvfrom((char*)currvideortprecvbuffer.c_str(), MAXUDPPACKGELEN, Socket::RecvFromCallback(&rtpOverUdp::socketVideoRTPRecvCallback, this));
		}
	}
	void socketVideoRTCPRecvCallback(const weak_ptr<Socket>& sock, const char* buffer, int len, const NetAddr& otearaddr)
	{
		shared_ptr<Socket> socktmp = sock.lock();
		if (socktmp)
		{
			currvideortcprecvbuffer.alloc(MAXUDPPACKGELEN);
			socktmp->async_recvfrom((char*)currvideortcprecvbuffer.c_str(), MAXUDPPACKGELEN, Socket::RecvFromCallback(&rtpOverUdp::socketVideoRTCPRecvCallback, this));
		}
	}
	void socketAudioRTPRecvCallback(const weak_ptr<Socket>& sock, const char* buffer, int len, const NetAddr& otearaddr)
	{
		if (buffer != curraudiortprecvbuffer.c_str() || len <= 0 || len > MAXUDPPACKGELEN) return;

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
			curraudiortprecvbuffer.alloc(MAXUDPPACKGELEN);
			socktmp->async_recvfrom((char*)curraudiortprecvbuffer.c_str(), MAXUDPPACKGELEN, Socket::RecvFromCallback(&rtpOverUdp::socketAudioRTPRecvCallback, this));
		}
	}
	void socketAudioRTCPRecvCallback(const weak_ptr<Socket>& sock, const char* buffer, int len, const NetAddr& otearaddr)
	{
		shared_ptr<Socket> socktmp = sock.lock();
		if (socktmp)
		{
			curraudiortcprecvbuffer.alloc(MAXUDPPACKGELEN);
			socktmp->async_recvfrom((char*)curraudiortcprecvbuffer.c_str(), MAXUDPPACKGELEN, Socket::RecvFromCallback(&rtpOverUdp::socketAudioRTCPRecvCallback, this));
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
				const char* framedataaddr = iter->framedata.c_str();
				size_t framedatasize = iter->framedata.length();

				datacallback(isvideo, iter->tiemstmap, iter->framedata.c_str() + sizeof(RTPHEADER), iter->framedata.length() - sizeof(RTPHEADER), iter->mark);

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

	String					currvideortprecvbuffer;
	String					currvideortcprecvbuffer;

	std::list<FrameInfo>	 videoframelist;
	std::list<shared_ptr<SendDataInfo> >	 sendvideortplist;

	String					curraudiortprecvbuffer;
	String					curraudiortcprecvbuffer;

	std::list<FrameInfo>	 audioframelist;
	std::list<shared_ptr<SendDataInfo> >	 sendaudiortplist;


	std::string				 dstaddr;
};