#include "rtp.h"
#include "RTSPProtocol.h"
#include "RTSP/RTSPStructs.h"

#define  MAXUDPPACKGELEN		56*1024
#define  MAXCACHEFRAMESIZE		10

class rtpOverUdp :public rtp
{
	struct FrameInfo
	{
		std::string		framedata;
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
		:rtp(_isserver,_rtspmedia,_datacallback), rtpvideosn(0), rtpaudiosn(0), prevvideoframesn(0), prevaudioframesn(0), dstaddr(_dstaddr)
	{
		if (rtspmedia.media.bHasVideo && rtspmedia.videoTransport.rtp.u.server_port1 != 0 && rtspmedia.videoTransport.rtp.u.client_port1 != 0)
		{
			videortp = UDP::create(work);
			videortp->bind(isserver ? rtspmedia.videoTransport.rtp.u.server_port1 : rtspmedia.videoTransport.rtp.u.client_port1);
			videortp->async_recvfrom(Socket::RecvFromCallback(&rtpOverUdp::socketVideoRTPRecvCallback, this), MAXUDPPACKGELEN);

			videortcp = UDP::create(work);
			videortcp->bind(isserver ? rtspmedia.videoTransport.rtp.u.server_port2 : rtspmedia.videoTransport.rtp.u.client_port2);
			videortcp->async_recvfrom(Socket::RecvFromCallback(&rtpOverUdp::socketVideoRTCPRecvCallback, this), MAXUDPPACKGELEN);
		}

		if (rtspmedia.media.bHasAudio && rtspmedia.audioTransport.rtp.u.server_port1 != 0 && rtspmedia.audioTransport.rtp.u.client_port1 != 0)
		{
			audiortp = UDP::create(work);
			audiortp->bind(isserver ? rtspmedia.audioTransport.rtp.u.server_port1 : rtspmedia.audioTransport.rtp.u.client_port1);
			audiortp->async_recvfrom(Socket::RecvFromCallback(&rtpOverUdp::socketAudioRTPRecvCallback, this), MAXUDPPACKGELEN);

			audiortcp = UDP::create(work);
			audiortcp->bind(isserver ? rtspmedia.audioTransport.rtp.u.server_port2 : rtspmedia.audioTransport.rtp.u.client_port2);
			audiortcp->async_recvfrom(Socket::RecvFromCallback(&rtpOverUdp::socketAudioRTCPRecvCallback, this), MAXUDPPACKGELEN);
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
	
	void sendData(bool isvideo, uint32_t timesmap, const String& data)
	{
		if (isvideo && !rtspmedia.media.bHasVideo) return;
		if (!isvideo && !rtspmedia.media.bHasAudio) return;

		const char* buffer = data.c_str();
		uint32_t bufferlen = data.length();

		while (bufferlen > 0)
		{
			uint32_t cansendlen = min(MAXRTPPACKETLEN, bufferlen);

			RTPHEADER header;
			memset(&header, 0, sizeof(header));
			header.v = 2;
			header.ts = htonl(timesmap);
			header.seq = htons(isvideo ? rtpvideosn++ : rtpaudiosn++ );
			header.pt = isvideo ? rtspmedia.media.stStreamVideo.nPayLoad : rtspmedia.media.stStreamAudio.nPayLoad;
			header.m = bufferlen == cansendlen;
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
		if (len > sizeof(RTPHEADER))
		{
			Guard locker(mutex);

			if (otearaddr.getPort() != (isserver ? rtspmedia.videoTransport.rtp.u.client_port1 : rtspmedia.videoTransport.rtp.u.server_port1))
			{
				assert(0);
				return;
			}

			RTPHEADER* header = (RTPHEADER*)buffer;

			FrameInfo info;
			info.framedata = std::string(buffer + sizeof(RTPHEADER), len - sizeof(RTPHEADER));
			info.mark = header->m;
			info.sn = ntohs(header->seq);
			info.tiemstmap = ntohl(header->ts);

			videoframelist.push_back(info);

			_checkFramelistData(isserver);
		}
		
		shared_ptr<Socket> socktmp = sock.lock();
		if(socktmp)
			socktmp->async_recvfrom(Socket::RecvFromCallback(&rtpOverUdp::socketVideoRTPRecvCallback, this), MAXUDPPACKGELEN);
	}
	void socketVideoRTCPRecvCallback(const weak_ptr<Socket>& sock, const char* buffer, int len, const NetAddr& otearaddr)
	{

	}
	void socketAudioRTPRecvCallback(const weak_ptr<Socket>& sock, const char* buffer, int len, const NetAddr& otearaddr)
	{
		if(len > sizeof(RTPHEADER))
		{
			Guard locker(mutex);

			if (otearaddr.getPort() != (isserver ? rtspmedia.videoTransport.rtp.u.client_port1 : rtspmedia.videoTransport.rtp.u.server_port1))
			{
				assert(0);
				return;
			}

			RTPHEADER* header = (RTPHEADER*)buffer;

			FrameInfo info;
			info.framedata = std::string(buffer + sizeof(RTPHEADER), len - sizeof(RTPHEADER));
			info.mark = header->m;
			info.sn = ntohs(header->seq);
			info.tiemstmap = ntohl(header->ts);

			audioframelist.push_back(info);

			_checkFramelistData(isserver);
		}

		shared_ptr<Socket> socktmp = sock.lock();
		if (socktmp)
			socktmp->async_recvfrom(Socket::RecvFromCallback(&rtpOverUdp::socketAudioRTPRecvCallback, this), MAXUDPPACKGELEN);
	}
	void socketAudioRTCPRecvCallback(const weak_ptr<Socket>& sock, const char* buffer, int len, const NetAddr& otearaddr)
	{

	}
	void _checkFramelistData(bool isvideo)
	{
		std::list<FrameInfo>& framelist = isvideo ? videoframelist : audioframelist;
		uint32_t& prevframesn = isvideo ? prevvideoframesn : prevaudioframesn;

		framelist.sort();

		while (framelist.size() > 0)
		{
			bool isdatafull = false;
			uint32_t framesize = 0;
			{
				uint32_t checkframesize = 0;
				for (std::list<FrameInfo>::iterator iter = framelist.begin(); iter != framelist.end(); iter++, checkframesize++)
				{
					if (checkframesize >= MAXCACHEFRAMESIZE)
					{
						//当缓存大于10个包，强制反出去
						isdatafull = true;
						break;
					}

					std::list<FrameInfo>::iterator nextiter = iter;
					nextiter++;

					if (nextiter == framelist.end()) break;

					if (prevframesn > iter->sn || prevframesn == 0) prevframesn = iter->sn;
					else if (prevframesn + 1 != nextiter->sn) break;

					framesize += iter->framedata.length();

					if (iter->mark || nextiter->mark)
					{
						isdatafull = true;
						break;
					}
				}
			}
			

			if (!isdatafull) break;

			{
				uint32_t getframenum = 0;
				uint32_t timestmap = 0;
				String framedata;
				framedata.resize(framesize);

				while (framelist.size() > 0)
				{
					bool ismark = false;
					{
						FrameInfo& info = framelist.front();

						framedata += info.framedata;
						prevframesn = info.sn;
						timestmap = info.tiemstmap;
						ismark = info.mark;

						framelist.pop_front();
					}
					
					if (getframenum >= MAXCACHEFRAMESIZE || ismark) break;

					getframenum++;
				}

				if (framedata.length() > 0)
				{
					datacallback(isvideo, timestmap, framedata);
				}
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

	uint32_t				 prevvideoframesn;
	std::list<FrameInfo>	 videoframelist;
	std::list<shared_ptr<SendDataInfo> >	 sendvideortplist;

	uint32_t				 prevaudioframesn;
	std::list<FrameInfo>	 audioframelist;
	std::list<shared_ptr<SendDataInfo> >	 sendaudiortplist;

	std::string				 dstaddr;
};