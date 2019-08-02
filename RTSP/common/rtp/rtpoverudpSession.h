#pragma once
#include "rtpSession.h"
#include "RTSPProtocol.h"
#include "RTSP/RTSPStructs.h"

#define MAXUDPPACKGELEN		56*1024

#define MAXRTPFRAMESIZE		100

class rtpOverUdpSession:public RTPSession
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
public:
	rtpOverUdpSession(bool _isserver, const shared_ptr<IOWorker>& ioworker,const std::string& _dstaddr, const shared_ptr<STREAM_TRANS_INFO>& _transport, const RTPSession::MediaDataCallback& _datacallback, const RTPSession::ContorlDataCallback& _contorlcallback)
		:RTPSession(_transport,_datacallback,_contorlcallback),isserver(_isserver), rtp_sendrtpsn(0), dstaddr(_dstaddr),rtp_prevframesn(0)
	{
		rtp_sock = UDP::create(ioworker);
		rtp_sock->bind(isserver ? transportinfo->transportinfo.rtp.u.server_port1 : transportinfo->transportinfo.rtp.u.client_port1);
		rtp_sock->setSocketBuffer(1024 * 1024 * 8, 0);
		rtp_sock->setSocketTimeout(1000, 1000);
		rtp_sock->nonBlocking(false);
		
		char* rtprecvbuffertmp = rtp_RecvBuffer.alloc(MAXUDPPACKGELEN);
		rtp_sock->async_recvfrom(rtprecvbuffertmp, MAXUDPPACKGELEN, Socket::RecvFromCallback(&rtpOverUdpSession::RTP_RecvCallback, this));

		rtcp_sock = UDP::create(ioworker);
		rtcp_sock->bind(isserver ? transportinfo->transportinfo.rtp.u.server_port2 : transportinfo->transportinfo.rtp.u.client_port2);
		rtcp_sock->setSocketBuffer(1024 * 1024, 0);
		rtcp_sock->setSocketTimeout(1000, 1000);
		rtcp_sock->nonBlocking(false);

		char* rtcprecvbuffertmp = rtcp_RecvBuffer.alloc(MAXUDPPACKGELEN);
		rtcp_sock->async_recvfrom(rtcprecvbuffertmp, MAXUDPPACKGELEN, Socket::RecvFromCallback(&rtpOverUdpSession::RTCP_RecvCallback, this));
	}
	~rtpOverUdpSession()
	{
		if(rtp_sock) rtp_sock->disconnect();
		if(rtcp_sock) rtcp_sock->disconnect();

		rtp_sock = NULL;
		rtcp_sock = NULL;
	}
	
	void sendContorlData(const shared_ptr<STREAM_TRANS_INFO>& transportinfo, const char*  buffer, uint32_t bufferlen)
	{
		Guard locker(mutex);

		String senddata = std::string(buffer, bufferlen);

		rtcp_sendlist.push_back(senddata);

		_sendAndCheckSend(false);
	}
	void sendMediaData(const shared_ptr<STREAM_TRANS_INFO>& transportinfo, uint32_t timestmap, const char*  buffer, uint32_t bufferlen, bool mark)
	{
		while (bufferlen > 0)
		{
			uint32_t cansendlen =  min(MAXRTPPACKETLEN, bufferlen);

			uint32_t sendtotallen = cansendlen + sizeof(RTPHEADER);

			String senddata;
			senddata.alloc(sendtotallen);
			
			char* senddatabuffer = (char*)senddata.c_str();

			RTPHEADER* header = (RTPHEADER*)senddatabuffer;
			memset(header, 0, sizeof(header));
			header->v = 2;
			header->ts = htonl(timestmap);
			header->seq = htons(rtp_sendrtpsn ++);
			header->pt = transportinfo->streaminfo.nPayLoad;
			header->m = bufferlen == cansendlen ? mark : false;
			header->ssrc = htonl(transportinfo->transportinfo.ssrc);

			memcpy((char*)(senddatabuffer + sizeof(RTPHEADER)), buffer , cansendlen);
			
			senddata.resize(sendtotallen);

			{
				Guard locker(mutex);

				rtp_sendlist.push_back(senddata);

				_sendAndCheckSend(true);
			}

			buffer += cansendlen;
			bufferlen -= cansendlen;
		}
	}
	void _sendAndCheckSend(bool isdata, const char* buffer = NULL,size_t len = 0)
	{
		std::list<String>& sendlist = isdata ? rtp_sendlist : rtcp_sendlist;
		
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
				String& item = sendlist.front();
				if (buffer != item.c_str())
				{
					assert(0);
				}
				sendlist.pop_front();
			}

			needSendData = true;
		}

		if (!needSendData) return;

		if (sendlist.size() <= 0) return;

		String& item = sendlist.front();

		//处理数据的零拷贝问题，添加前置数据		
		const char* sendbuffer = item.c_str();
		uint32_t sendbufferlen = item.length();

		if (isdata)
		{
			shared_ptr<Socket> socktmp = rtp_sock;
			if (socktmp)
			{
				socktmp->async_sendto(sendbuffer, sendbufferlen,
					NetAddr(dstaddr, isserver ? transportinfo->transportinfo.rtp.u.client_port1 : transportinfo->transportinfo.rtp.u.server_port1),
					Socket::SendedCallback(&rtpOverUdpSession::RTP_SendCallback, this));
			}	
		}
		else
		{
			shared_ptr<Socket> socktmp = rtcp_sock;
			if (socktmp)
			{
				socktmp->async_sendto(sendbuffer, sendbufferlen,
					NetAddr(dstaddr, isserver ? transportinfo->transportinfo.rtp.u.client_port2 : transportinfo->transportinfo.rtp.u.server_port2),
					Socket::SendedCallback(&rtpOverUdpSession::RTCP_SendCallback, this));
			}	
		}
	}
	void RTP_SendCallback(const weak_ptr<Socket>& sock, const char* buffer, int len)
	{
		Guard locker(mutex);

		_sendAndCheckSend(true, buffer, len);
	}
	void RTCP_SendCallback(const weak_ptr<Socket>& sock, const char* buffer, int len)
	{
		Guard locker(mutex);

		_sendAndCheckSend(false, buffer, len);
	}
	void RTP_RecvCallback(const weak_ptr<Socket>& sock, const char* buffer, int len,const NetAddr& otearaddr)
	{
		if (buffer != rtp_RecvBuffer.c_str() || len <= 0 || len > MAXUDPPACKGELEN) return;

		if (len > sizeof(RTPHEADER))
		{
			rtp_RecvBuffer.resize(len);

			if (otearaddr.getPort() != (isserver ? transportinfo->transportinfo.rtp.u.client_port1 : transportinfo->transportinfo.rtp.u.server_port1))
			{
				assert(0);
			}

			RTPHEADER* header = (RTPHEADER*)buffer;

			if (header->v == RTP_VERSION)
			{
				FrameInfo info;
				info.framedata = rtp_RecvBuffer;
				info.mark = header->m;
				info.sn = ntohs(header->seq);
				info.tiemstmap = ntohl(header->ts);

				_checkRTPFramelistData(info);
			}
		}
		
		shared_ptr<Socket> socktmp = sock.lock();
		if (socktmp)
		{
			char* buffertmp = rtp_RecvBuffer.alloc(MAXUDPPACKGELEN);
			socktmp->async_recvfrom(buffertmp, MAXUDPPACKGELEN, Socket::RecvFromCallback(&rtpOverUdpSession::RTP_RecvCallback, this));
		}
	}
	void RTCP_RecvCallback(const weak_ptr<Socket>& sock, const char* buffer, int len, const NetAddr& otearaddr)
	{
		if (buffer != rtcp_RecvBuffer.c_str() || len <= 0 || len > MAXUDPPACKGELEN) return;

		contorlcallback(transportinfo, buffer, len);

		shared_ptr<Socket> socktmp = sock.lock();
		if (socktmp)
		{
			socktmp->async_recvfrom((char*)rtcp_RecvBuffer.c_str(), MAXUDPPACKGELEN, Socket::RecvFromCallback(&rtpOverUdpSession::RTCP_RecvCallback, this));
		}
	}
	void _checkRTPFramelistData(const FrameInfo& info)
	{
		//todo：这个函数有些问题，当来一个0后，再来一个65534 这样的数据会出问题
		std::list<FrameInfo>& framelist = rtp_framelist;
		uint16_t& prevsn = rtp_prevframesn;

		//当新数据来了，清空来数据
		if (info.sn == 0 && framelist.size() > 0)
		{
			for (std::list<FrameInfo>::iterator iter = framelist.begin(); iter != framelist.end(); iter++)
			{
				if ((uint16_t)(prevsn + 1) != iter->sn)
				{
					logwarn("RTSP start sn %d to sn :%d loss", prevsn, iter->sn);
				}
				RTPHEADER* header = (RTPHEADER*)iter->framedata.c_str();
				const char* framedataaddr = iter->framedata.c_str() + sizeof(RTPHEADER);
				size_t framedatasize = iter->framedata.length() - sizeof(RTPHEADER);

				datacallback(transportinfo, iter->tiemstmap, framedataaddr, framedatasize, iter->mark);

				prevsn = iter->sn;
			}
			framelist.clear();
			prevsn = 0;
		}

		FrameInfo frametmp = info;
		do 
		{
			if (prevsn == 0 || (uint16_t)(prevsn + 1) == frametmp.sn)
			{
				//连续数据
				RTPHEADER* header = (RTPHEADER*)frametmp.framedata.c_str();
				const char* framedataaddr = frametmp.framedata.c_str() + sizeof(RTPHEADER);
				size_t framedatasize = frametmp.framedata.length() - sizeof(RTPHEADER);

				datacallback(transportinfo, frametmp.tiemstmap,framedataaddr, framedatasize, frametmp.mark);

				prevsn = frametmp.sn;
			}
			else
			{
				//数据序号不连续放入缓存中去
				framelist.push_back(frametmp);

				framelist.sort();
			}

			if (framelist.size() <= 0) break;

			if (framelist.front().sn == (uint16_t)(prevsn + 1) || framelist.size() > MAXRTPFRAMESIZE)
			{
				frametmp = framelist.front();
				
				if (prevsn > frametmp.sn)
				{
					int a = 0;
				}
				
				framelist.pop_front();


				if (frametmp.sn != (uint16_t)(prevsn + 1))
				{
					logwarn("RTSP start sn %d to sn :%d loss", prevsn, frametmp.sn);
				}

				prevsn = 0;
			}
			else
			{
				break;
			}
		} while (1);
	}
private:
	bool					 isserver;

	Mutex					 mutex;
	
	uint16_t				 rtp_sendrtpsn;

	shared_ptr<Socket>		 rtp_sock;
	shared_ptr<Socket>		 rtcp_sock;

	String					 rtp_RecvBuffer;
	String 					 rtcp_RecvBuffer;

	std::list<FrameInfo>	 rtp_framelist;
	uint16_t				 rtp_prevframesn;


	std::list<String>		 rtp_sendlist;
	std::list<String>		 rtcp_sendlist;

	std::string				 dstaddr;
};