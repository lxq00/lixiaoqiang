#pragma once
#include "respCmd.h"
#include "HTTP/HTTPParse.h"
using namespace Public::HTTP;

#define MAXPARSERTSPBUFFERLEN	1024*16
#define RTPOVERTCPMAGIC		'$'
#define RTSPCMDFLAG			"RTSP/1.0"

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
public:
	typedef Function2<void, const shared_ptr<HTTPParse::Header>&, const std::string&> CommandCallback;
	typedef Function2<void, const char*, uint32_t> ExternDataCallback;
	typedef Function0<void> DisconnectCallback;
protected:
	RTSPProtocol(const shared_ptr<Socket>& sock, const CommandCallback& cmdcallback, const ExternDataCallback& datacallback, const DisconnectCallback& disconnectCallback,bool server)
		:HTTPParse(!server), m_isserver(server)
	{
		m_sock = sock;
		m_cmdcallback = cmdcallback;
		m_extdatacallback = datacallback;
		m_disconnect = disconnectCallback;
		m_prevalivetime = Time::getCurrentMilliSecond();

		m_recvBuffer = new char[MAXPARSERTSPBUFFERLEN + 100];
		m_recvBufferDataLen = 0;


		m_sock->setDisconnectCallback(Socket::DisconnectedCallback(&RTSPProtocol::onSocketDisconnectCallback, this));
		m_sock->async_recv(Socket::ReceivedCallback(&RTSPProtocol::onSocketRecvCallback, this));
	}
	~RTSPProtocol()
	{
		m_sock->disconnect();
		m_sock = NULL;
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

	void sendCmd(const shared_ptr<CMDProtocol>& cmd)
	{
		std::string cmdstr = cmd->build();
		if (cmdstr.length() == 0) return;

		Guard locker(m_mutex);
		_addAndCheckSendData(cmdstr);
	}
	void sendMedia(int channel,bool isvideo,const char* rtpheader, uint32_t rtpheaderlen, const char* dataaddr, uint32_t datalen)
	{
		Guard locker(m_mutex);

		{
			INTERLEAVEDFRAME frame;
			frame.magic = RTPOVERTCPMAGIC;
			frame.channel = channel;
			frame.rtp_len = htons(rtpheaderlen + datalen);

			_addAndCheckSendData(std::string((char*)&frame, sizeof(frame)));
		}

		_addAndCheckSendData(std::string(rtpheader, rtpheaderlen));
		_addAndCheckSendData(std::string(dataaddr, datalen));
	}
private:
	void _addAndCheckSendData(const std::string& data)
	{
		shared_ptr<SendItem> item = make_shared<SendItem>();
		item->data = std::move(data);

		m_sendList.push_back(item);
		if (m_sendList.size() == 1 && m_sock != NULL)
		{
			shared_ptr<SendItem> item = make_shared<SendItem>();
			const char* sendbuffer = item->data.c_str();
			uint32_t sendbufferlen = item->data.length();

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

			m_sock->async_send(sendbuffer, sendbufferlen, Socket::SendedCallback(&RTSPProtocol::onSocketSendCallback, this));
		}
	}
	void onSocketDisconnectCallback(const weak_ptr<Socket>&, const std::string&)
	{
		m_disconnect();
	}
	void onSocketRecvCallback(const weak_ptr<Socket>& sock, const char* buffer, int len)
	{
		if (len <= 0) return;

		if (m_recvBufferDataLen + len > MAXPARSERTSPBUFFERLEN) return;

		m_recvBufferDataLen += len;

	}
private:
	bool						m_isserver;
	Mutex						m_mutex;

	CommandCallback				m_cmdcallback;
	ExternDataCallback			m_extdatacallback;
	DisconnectCallback			m_disconnect;

	shared_ptr<Socket>			m_sock;

	char*						m_recvBuffer;
	uint32_t					m_recvBufferDataLen;

	std::list<shared_ptr<SendItem> > m_sendList;

	uint64_t					m_prevalivetime;
};


