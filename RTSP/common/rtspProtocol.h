#pragma once
#include "HTTP/HTTPParse.h"
using namespace Public::HTTP;

#define MAXPARSERTSPBUFFERLEN	1024*128
#define RTPOVERTCPMAGIC		'$'
#define RTSPCMDFLAG			"RTSP/1.0"

#define RTSPCONENTTYPESDP	"application/sdp"

class RTSPProtocol:public HTTPParse
{
	struct SendItem
	{
		StringBuffer		buffer;
		uint32_t			havesendlen;

		SendItem():havesendlen(0){}
	};
	struct INTERLEAVEDFRAME
	{
		unsigned int magic : 8;// $
		unsigned int channel : 8; //0-1
		unsigned int rtp_len : 16;
	};
	struct TCPInterleaved
	{
		int dataChannel;
		int contrlChannel;
	};
public:
	typedef Function1<void, const shared_ptr<RTSPCommandInfo>&> CommandCallback;
	typedef Function2<void, uint32_t,const StringBuffer&> ExternDataCallback;
	typedef Function0<void> DisconnectCallback;
public:
	RTSPProtocol(const shared_ptr<Socket>& sock, const CommandCallback& cmdcallback, const DisconnectCallback& disconnectCallback,bool server)
		:HTTPParse(server)
	{
		m_sock = sock;
		m_cmdcallback = cmdcallback;
		
		m_disconnect = disconnectCallback;
		m_prevalivetime = Time::getCurrentMilliSecond();
		m_bodylen = 0;
		m_haveFindHeaderStart = false;

		m_recvBuffer.alloc(MAXPARSERTSPBUFFERLEN);

		m_sock->setSocketBuffer(1024 * 1024 * 4, 1024 * 1024);
		m_sock->setDisconnectCallback(Socket::DisconnectedCallback(&RTSPProtocol::onSocketDisconnectCallback, this));
		m_sock->async_recv((char*)m_recvBuffer.c_str(), MAXPARSERTSPBUFFERLEN, Socket::ReceivedCallback(&RTSPProtocol::onSocketRecvCallback, this));
	}
	~RTSPProtocol()
	{
		m_sock->disconnect();
		m_sock = NULL;
	}

	void setRTPOverTcpCallback(const ExternDataCallback& datacallback)
	{
		m_extdatacallback = datacallback;
	}

	uint64_t prevalivetime() const { return m_prevalivetime; }
	uint32_t sendListSize()
	{
		uint32_t listsize = 0;

		Guard locker(m_mutex);
		for (std::list<shared_ptr<SendItem> >::iterator iter = m_sendList.begin(); iter != m_sendList.end(); iter++)
		{
			listsize += (*iter)->buffer.size();
		}

		return listsize;
	}

	void sendProtocolData(const std::string& cmdstr)
	{
		if (cmdstr.length() == 0) return;

		printf("%s",cmdstr.c_str());

		Guard locker(m_mutex);
		_addAndCheckSendData(StringBuffer(cmdstr));
	}
	void sendMedia(bool isvideo,const char* rtpheader, uint32_t rtpheaderlen, const StringBuffer& buffer)
	{
		if (m_tcpinterleaved == NULL) return;
		INTERLEAVEDFRAME frame;
		frame.magic = RTPOVERTCPMAGIC;
		frame.channel = m_tcpinterleaved->dataChannel;
		frame.rtp_len = htons(rtpheaderlen + (uint16_t)buffer.size());

		StringBuffer rtpovertcp = std::string((char*)&frame, sizeof(frame)) + std::string(rtpheader, rtpheaderlen);

		Guard locker(m_mutex);
		_addAndCheckSendData(rtpovertcp);
		_addAndCheckSendData(buffer);
	}
	void sendContrlData(const char* buffer, uint32_t len)
	{
		if (m_tcpinterleaved == NULL) return;

		INTERLEAVEDFRAME frame;
		frame.magic = RTPOVERTCPMAGIC;
		frame.channel = m_tcpinterleaved->contrlChannel;
		frame.rtp_len = htons(len);

		StringBuffer contorldata = std::string((char*)&frame, sizeof(frame)) + std::string(buffer, len);


		Guard locker(m_mutex);
		_addAndCheckSendData(contorldata);
	}
	void setTCPInterleavedChannel(int datachannel,int contorl)
	{
		m_tcpinterleaved = make_shared<TCPInterleaved>();
		m_tcpinterleaved->dataChannel = datachannel;
		m_tcpinterleaved->contrlChannel = contorl;
	}
private:
	void _addAndCheckSendData(const StringBuffer& buffer)
	{
		shared_ptr<SendItem> item = make_shared<SendItem>();
		item->buffer = buffer;

		m_sendList.push_back(item);
		if (m_sendList.size() == 1 && m_sock != NULL)
		{
			shared_ptr<SendItem> item = m_sendList.front();
			const char* sendbuffer = item->buffer.buffer();
			uint32_t sendbufferlen = item->buffer.size();
			
			m_prevalivetime = Time::getCurrentMilliSecond();

			m_sock->async_send(sendbuffer, sendbufferlen, Socket::SendedCallback(&RTSPProtocol::onSocketSendCallback, this));
		}
	}
	void onSocketSendCallback(const weak_ptr<Socket>& sock, const char* buffer, int len)
	{
		Guard locker(m_mutex);

		shared_ptr<Socket> tmp = sock.lock();
		if (tmp == NULL) return;

		if (len < 0) return;

		if (m_sendList.size() <= 0) return;

		{
			shared_ptr<SendItem> item = m_sendList.front();
			if (buffer < item->buffer.buffer() || buffer > item->buffer.buffer() + len) return;

			if (item->havesendlen + len > item->buffer.size()) return;

			item->havesendlen += len;

			if (item->havesendlen == item->buffer.size())
			{
				m_sendList.pop_front();
			}
		}		

		if (m_sendList.size() > 0)
		{
			shared_ptr<SendItem> item = make_shared<SendItem>();
			const char* sendbuffer = item->buffer.buffer() + item->havesendlen;
			uint32_t sendbufferlen = item->buffer.size() - item->havesendlen;

			m_prevalivetime = Time::getCurrentMilliSecond();

			m_sock->async_send(sendbuffer, sendbufferlen, Socket::SendedCallback(&RTSPProtocol::onSocketSendCallback, this));
		}
	}
	void onSocketDisconnectCallback(const weak_ptr<Socket>&, const std::string&)
	{
		m_disconnect();
	}
	void onSocketRecvCallback(const weak_ptr<Socket>& sock, const char* buffer, int len)
	{
		m_prevalivetime = Time::getCurrentMilliSecond();
		if (len <= 0)
		{
			assert(0);
			return;
		}
		if (m_recvBuffer.c_str() + m_recvBuffer.length() != buffer)
		{
			assert(0);
		}

		m_recvBuffer.resize(m_recvBuffer.length() + len);
		
		const char* buffertmp = m_recvBuffer.c_str();
		uint32_t buffertmplen = m_recvBuffer.length();

		while (buffertmplen > 0)
		{
			if (m_cmdinfo != NULL)
			{
				if (m_bodylen > m_cmdinfo->body.length())
				{
					//数据不够
					if (buffertmplen < m_bodylen - m_cmdinfo->body.length()) break;

					uint32_t needlen = m_bodylen - m_cmdinfo->body.length();

					m_cmdinfo->body.append(buffertmp, needlen);
					buffertmp += needlen;
					buffertmplen -= needlen;
				}
				
				{
					m_cmdcallback(m_cmdinfo);
					m_cmdinfo = NULL;
					m_bodylen = 0;
					m_haveFindHeaderStart = false;
				}
				
			}
			else if (m_cmdinfo == NULL && m_haveFindHeaderStart)
			{
				uint32_t usedlen = 0;
				shared_ptr<HTTPParse::Header> header = HTTPParse::parse(buffertmp, buffertmplen, usedlen);

				if (usedlen > buffertmplen)
				{
					assert(0);
				}

				if (header != NULL)
				{
					printf("%s", std::string(buffertmp,usedlen).c_str());

					m_cmdinfo = make_shared<RTSPCommandInfo>();
					m_cmdinfo->method = header->method;
					m_cmdinfo->url = header->url.href();
					m_cmdinfo->verinfo.protocol = header->verinfo.protocol;
					m_cmdinfo->verinfo.version = header->verinfo.version;
					m_cmdinfo->statuscode = header->statuscode;
					m_cmdinfo->statusmsg = header->statusmsg;
					m_cmdinfo->headers = std::move(header->headers);

					m_cmdinfo->cseq = m_cmdinfo->header("CSeq").readInt();

					m_bodylen = m_cmdinfo->header(Content_Length).readInt();

					if (m_bodylen == 0)
					{
						m_cmdcallback(m_cmdinfo);
						m_cmdinfo = NULL;
						m_bodylen = 0;
						m_haveFindHeaderStart = false;
					}
				}

				buffertmp += usedlen;
				buffertmplen -= usedlen;
			}
			else if (*buffertmp == RTPOVERTCPMAGIC)
			{
				if (buffertmplen < sizeof(INTERLEAVEDFRAME)) break;

				INTERLEAVEDFRAME* frame = (INTERLEAVEDFRAME*)buffertmp;
				uint32_t datalen = ntohs(frame->rtp_len);
				uint32_t datach = frame->channel;
				if (datalen <= 0 || (m_tcpinterleaved && datach != m_tcpinterleaved->dataChannel && datach != m_tcpinterleaved->contrlChannel))
				{
					buffertmp++;
					buffertmplen--;
					continue;
				}

				if (datalen >= 0xffff)
				{
					assert(0);
				}

				if (datalen + sizeof(INTERLEAVEDFRAME) > buffertmplen) break;

				m_extdatacallback(datach, StringBuffer(m_recvBuffer,buffertmp + sizeof(INTERLEAVEDFRAME), datalen));

				buffertmp += datalen + sizeof(INTERLEAVEDFRAME);
				buffertmplen -= datalen + sizeof(INTERLEAVEDFRAME);
			}
			else if(!m_haveFindHeaderStart)
			{
				uint32_t ret = checkIsRTSPCommandStart(buffertmp, buffertmplen);
				if (ret == 0)
				{
					buffertmp++;
					buffertmplen--;
					continue;
				}
				else if (ret == 1)
				{
					break;
				}
				else
				{
					m_haveFindHeaderStart = true;
				}
			}
			else
			{
				assert(0);
				int a = 0;
			}
		}
		String newdatabuffer;
		newdatabuffer.alloc(buffertmplen + MAXPARSERTSPBUFFERLEN);

		if (buffertmplen > 0)
		{
			memcpy((char*)newdatabuffer.c_str(),buffertmp, buffertmplen);
			newdatabuffer.resize(buffertmplen);
		}
		m_recvBuffer = newdatabuffer;


		shared_ptr<Socket> socktmp = sock.lock();
		if (socktmp != NULL)
		{
			socktmp->async_recv((char*)m_recvBuffer.c_str() + buffertmplen, MAXPARSERTSPBUFFERLEN, Socket::ReceivedCallback(&RTSPProtocol::onSocketRecvCallback, this));
		}
	}
	// return 0 not cmonnand,return 1 not sure need wait,return 2 is command
	uint32_t checkIsRTSPCommandStart(const char* buffer, uint32_t len)
	{
		static std::string rtspcomandflag = "rtsp";

		while (len > 0)
		{
			//b不是可现实字符，不是
			if (!isCanShowChar(*buffer)) return 0;

			if (len < rtspcomandflag.length()) break;

			//找到标识，是命令
			if (strncasecmp(buffer, rtspcomandflag.c_str(), rtspcomandflag.length()) == 0) return 2;

			buffer++;
			len--;
		}
		return 1;
	}
	//判断是否是显示字符
	bool isCanShowChar(char ch)
	{
		return ((ch >= 0x20 && ch < 0x7f) || ch == '\r' || ch == '\n');
	}
public:
	shared_ptr<Socket>			m_sock;
private:
	CommandCallback				m_cmdcallback;
	ExternDataCallback			m_extdatacallback;
	DisconnectCallback			m_disconnect;

	String						m_recvBuffer;

	Mutex						m_mutex;
	std::list<shared_ptr<SendItem> > m_sendList;

	uint64_t					m_prevalivetime;


	bool						m_haveFindHeaderStart;
	shared_ptr<RTSPCommandInfo>	m_cmdinfo;
	uint32_t					m_bodylen;
	
	shared_ptr<TCPInterleaved>  m_tcpinterleaved;
};


