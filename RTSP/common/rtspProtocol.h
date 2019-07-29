#pragma once
#include "HTTP/HTTPParse.h"
using namespace Public::HTTP;

#define MAXPARSERTSPBUFFERLEN	1024*16
#define RTPOVERTCPMAGIC		'$'
#define RTSPCMDFLAG			"RTSP/1.0"

#define RTSPCONENTTYPESDP	"application/sdp"

class RTSPProtocol:public HTTPParse
{
	struct SendItem
	{
		std::string			data;
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
	typedef Function3<void, uint32_t,const char*, uint32_t> ExternDataCallback;
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
		m_extdataLen = 0;

		m_recvBuffer = new char[MAXPARSERTSPBUFFERLEN + 100];
		m_recvBufferDataLen = 0;


		m_sock->setDisconnectCallback(Socket::DisconnectedCallback(&RTSPProtocol::onSocketDisconnectCallback, this));
		m_sock->async_recv(m_recvBuffer, MAXPARSERTSPBUFFERLEN, Socket::ReceivedCallback(&RTSPProtocol::onSocketRecvCallback, this));
	}
	~RTSPProtocol()
	{
		m_sock->disconnect();
		m_sock = NULL;
		SAFE_DELETEARRAY(m_recvBuffer);
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
			listsize += (*iter)->data.length();
		}

		return listsize;
	}

	void sendProtocolData(const std::string& cmdstr)
	{
		if (cmdstr.length() == 0) return;

		Guard locker(m_mutex);
		_addAndCheckSendData(cmdstr.c_str(),cmdstr.length());
	}
	void sendMedia(bool isvideo,const char* rtpheader, uint32_t rtpheaderlen, const char* dataaddr, uint32_t datalen)
	{
		if (m_tcpinterleaved == NULL) return;
		INTERLEAVEDFRAME frame;
		frame.magic = RTPOVERTCPMAGIC;
		frame.channel = m_tcpinterleaved->dataChannel;
		frame.rtp_len = htons(rtpheaderlen + datalen);

		std::string rtpovertcp = std::string((char*)&frame, sizeof(frame)) + std::string(rtpheader, rtpheaderlen);

		Guard locker(m_mutex);
		_addAndCheckSendData(rtpovertcp.c_str(), rtpovertcp.length());
		_addAndCheckSendData(dataaddr, datalen);
	}
	void sendContrlData(const char* buffer, uint32_t len)
	{
		if (m_tcpinterleaved == NULL) return;

		INTERLEAVEDFRAME frame;
		frame.magic = RTPOVERTCPMAGIC;
		frame.channel = m_tcpinterleaved->contrlChannel;
		frame.rtp_len = htons(len);

		std::string contorldata = std::string((char*)&frame, sizeof(frame)) + std::string(buffer, len);


		Guard locker(m_mutex);
		_addAndCheckSendData(contorldata.c_str(), contorldata.length());
	}
	void setTCPInterleavedChannel(int datachannel,int contorl)
	{
		m_tcpinterleaved = make_shared<TCPInterleaved>();
		m_tcpinterleaved->dataChannel = datachannel;
		m_tcpinterleaved->contrlChannel = contorl;
	}
private:
	void _addAndCheckSendData(const char* buffer,uint32_t len)
	{
		shared_ptr<SendItem> item = make_shared<SendItem>();
		item->data = std::string(buffer,len);

		m_sendList.push_back(item);
		if (m_sendList.size() == 1 && m_sock != NULL)
		{
			shared_ptr<SendItem> item = m_sendList.front();
			const char* sendbuffer = item->data.c_str();
			uint32_t sendbufferlen = item->data.length();
			
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
			if (buffer < item->data.c_str() || buffer > item->data.c_str() + len) return;

			if (item->havesendlen + len > item->data.length()) return;
			item->havesendlen += len;

			if (item->havesendlen == item->data.length())
			{
				m_sendList.pop_front();
			}
		}		

		if (m_sendList.size() > 0)
		{
			shared_ptr<SendItem> item = make_shared<SendItem>();
			const char* sendbuffer = item->data.c_str() + item->havesendlen;
			uint32_t sendbufferlen = item->data.length() - item->havesendlen;

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
		if (m_recvBuffer + m_recvBufferDataLen != buffer)
		{
			assert(0);
		}
		if (m_recvBufferDataLen + len > MAXPARSERTSPBUFFERLEN)
		{
			assert(0);
			return;
		}

		m_recvBufferDataLen += len;
		
		const char* buffertmp = m_recvBuffer;
		uint32_t buffertmplen = m_recvBufferDataLen;

		while (buffertmplen > 0)
		{
			if (m_header != NULL && m_bodylen > m_body.length())
			{
				uint32_t needlen = min(m_bodylen - m_body.length(), buffertmplen);
				m_body += std::string(buffertmp, needlen);
				buffertmp += needlen;
				buffertmplen -= needlen;
			}
			else if (m_header == NULL && m_haveFindHeaderStart)
			{
				uint32_t usedlen = 0;
				m_header = HTTPParse::parse(buffertmp, buffertmplen, usedlen);

				if (usedlen > buffertmplen)
				{
					assert(0);
				}

				buffertmp += usedlen;
				buffertmplen -= usedlen;
				if (m_header != NULL)
				{
					m_bodylen = m_header->header(Content_Length).readInt();
				}
			}
			else if (m_extdataLen != 0 && m_extdataLen > m_extdata.length())
			{
				uint32_t needlen = min(m_extdataLen - m_extdata.length(), buffertmplen);
				m_extdata += std::string(buffertmp, needlen);
				buffertmp += needlen;
				buffertmplen -= needlen;
			}
			else if (*buffertmp == RTPOVERTCPMAGIC)
			{
				if (buffertmplen < sizeof(INTERLEAVEDFRAME)) break;

				INTERLEAVEDFRAME* frame = (INTERLEAVEDFRAME*)buffertmp;
				uint32_t datalen = ntohs(frame->rtp_len);
				if (datalen <= 0 || (m_tcpinterleaved && frame->channel != m_tcpinterleaved->dataChannel && frame->channel != m_tcpinterleaved->contrlChannel))
				{
					buffertmp++;
					buffertmplen--;
					continue;
				}
				m_extdataLen = datalen;
				m_extdatach = frame->channel;

				if (m_extdataLen <= 0)
				{
					assert(0);
				}
				buffertmp += sizeof(INTERLEAVEDFRAME);
				buffertmplen -= sizeof(INTERLEAVEDFRAME);
			}
			else if(!m_haveFindHeaderStart)
			{
				//放到头解析里去解析的数据一定是可显示的文本数据，如果不是，那么跳过
				if (*buffertmp < 0x20 || *buffertmp >= 0x7f)
				{
					assert(0);
					buffertmp++;
					buffertmplen--;
					continue;
				}
				if (buffertmplen < strlen(RTSPCMDFLAG)) break;

				if (strncasecmp(buffertmp, RTSPCMDFLAG,strlen(RTSPCMDFLAG)) != 0)
				{
					assert(0);
					buffertmp++;
					buffertmplen--;
					continue;
				}

				m_haveFindHeaderStart = true;
			}
			else
			{
				assert(0);
				int a = 0;
			}

			if (m_header != NULL && m_bodylen == m_body.length())
			{
				shared_ptr<RTSPCommandInfo> cmd = make_shared<RTSPCommandInfo>();
				cmd->method = m_header->method;
				cmd->url = m_header->url.href();
				cmd->verinfo.protocol = m_header->verinfo.protocol;
				cmd->verinfo.version = m_header->verinfo.version;
				cmd->statuscode = m_header->statuscode;
				cmd->statusmsg = m_header->statusmsg;
				cmd->headers = std::move(m_header->headers);
				
				cmd->body = std::move(m_body);
				cmd->cseq = cmd->header("CSeq").readInt();

				m_cmdcallback(cmd);
				m_header = NULL;
				m_body = "";
				m_bodylen = 0;
				m_haveFindHeaderStart = false;
			}
			else if (m_extdataLen != 0 && m_extdataLen == m_extdata.length())
			{
				m_extdatacallback(m_extdatach, m_extdata.c_str(), m_extdata.length());
				
				m_extdataLen = 0;
				m_extdata = "";
			}
		}
		if (buffertmplen > 0 && m_recvBuffer != buffertmp)
		{
			memmove(m_recvBuffer, buffertmp, buffertmplen);
		}
		m_recvBufferDataLen = buffertmplen;


		shared_ptr<Socket> socktmp = sock.lock();
		if (socktmp != NULL)
		{
			socktmp->async_recv(m_recvBuffer + m_recvBufferDataLen, MAXPARSERTSPBUFFERLEN - m_recvBufferDataLen, Socket::ReceivedCallback(&RTSPProtocol::onSocketRecvCallback, this));
		}
	}
public:
	shared_ptr<Socket>			m_sock;
private:
	CommandCallback				m_cmdcallback;
	ExternDataCallback			m_extdatacallback;
	DisconnectCallback			m_disconnect;

	char*						m_recvBuffer;
	uint32_t					m_recvBufferDataLen;

	Mutex						m_mutex;
	std::list<shared_ptr<SendItem> > m_sendList;

	uint64_t					m_prevalivetime;
	bool						m_haveFindHeaderStart;
	shared_ptr<HTTPParse::Header> m_header;
	uint32_t					m_bodylen;
	std::string					m_body;

	uint32_t					m_extdataLen;
	uint32_t					m_extdatach;
	std::string					m_extdata;

	shared_ptr<TCPInterleaved>  m_tcpinterleaved;
};


