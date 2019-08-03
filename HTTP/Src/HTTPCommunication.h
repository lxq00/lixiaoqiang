#pragma once
#include "HTTP/HTTPPublic.h"
#include "HTTP/HTTPParse.h"
namespace Public {
namespace HTTP {

class HTTPCommunicationHandler
{
public:
	HTTPCommunicationHandler() {}
	virtual ~HTTPCommunicationHandler() {}

	virtual void onRecvHeaderOk(HTTPCommunication* commu) = 0;
	virtual void onRecvContentOk(HTTPCommunication* commu) = 0;
	virtual void onDisconnect(HTTPCommunication* commu) = 0;
};

#define MAXHTTPCACHELEN		1024*56

#define HTTPTIMEOUT			60*1000

class HTTPCommunication :public HTTPParse
{
public:
	HTTPCommunication(bool _isserver,const shared_ptr<Socket> & _sock,const shared_ptr<HTTPCommunicationHandler>& _handler,const std::string& _useragent)
		:HTTPParse(_isserver),socket(_sock),handler(_handler),recvContentLen(-1),recvContentTotalLen(0),
		sendContentLen(-1), sendTotalLen(0),sendHeaderLen(0), prevalivetime(Time::getCurrentMilliSecond()),useragent(_useragent)
	{
		socket->setDisconnectCallback(Socket::DisconnectedCallback(&HTTPCommunication::onSocketDisconnectCallback, this));

		char* recvBufferaddr = recvBuffer.alloc(MAXHTTPCACHELEN);
		socket->async_recv(recvBufferaddr, MAXHTTPCACHELEN, Socket::ReceivedCallback(&HTTPCommunication::onSocketRecvCallback, this));
	}
	~HTTPCommunication() {}

	void close()
	{
		if (socket) socket->disconnect();
		socket = NULL;
	}

	void setSendHeaderContentAndPostSend(const shared_ptr<HTTPHeader>& header, const shared_ptr<IContent>& content)
	{
		if (!sendHeader) 
		{
			sendHeader = header;

			Value contentlenval = sendHeader->header(Content_Length);
			if (!contentlenval.empty()) sendContentLen = contentlenval.readUint64();
		}
		if (!sendContent) sendContent = content;

		checkSendResultAndPostSend(false);
	}
	void onPoolTimerProc()
	{
		checkSendResultAndPostSend(false);
	}
	bool sessionIsFinish()
	{
		Guard locker(mutex);

		//接收到头数据，且content数据满，有发送头数据，且发送数据已满content和header长度 
		return (recvHeader && recvContentLen != -1 && recvContentLen == recvContentTotalLen) && 
			(sendHeaderLen > 0 && sendContentLen != -1 && sendContentLen == sendTotalLen - sendHeaderLen);
	}
	bool isTimeout()
	{
		uint64_t nowtime = Time::getCurrentMilliSecond();
		uint64_t prevtime = prevalivetime;

		if (nowtime < prevtime) return false;

		return nowtime - prevtime >= HTTPTIMEOUT;
	}
private:
	void checkSendResultAndPostSend(bool sendcheck,int len = 0)
	{
		Guard locker(mutex);

		if (len > 0)
		{
			sendTotalLen += len;
			if (len > sendBuffer.length())
			{
				assert(0);
			}
			if (len == sendBuffer.length()) sendBuffer.resize(0);
			else
			{
				char* sendbufferaddr = (char*)sendBuffer.c_str();
				memmove(sendbufferaddr, sendbufferaddr + len, sendBuffer.length() - len);
				sendBuffer.resize(sendBuffer.length() - len);
			}
		}
		
		//发送缓冲区有数据，且当前属于检测状态，不发送
		if (sendBuffer.length() > 0 && !sendcheck)
		{
			return;
		}

		if (sendHeaderLen == 0)
		{
			if (!sendHeader) return;
			
			sendBuffer = HTTPBuild::build(!isServer, *sendHeader.get());
			sendHeaderLen = sendBuffer.length();
		}
		else if (sendBuffer.length() == 0 && sendContent)
		{
			sendContent->read(sendBuffer);
		}
		if (sendBuffer.length() == 0) return;

		shared_ptr<Socket> socktmp = socket;
		if (socktmp) socktmp->async_send(sendBuffer.c_str(), sendBuffer.length(), Socket::SendedCallback(&HTTPCommunication::onSocketSendCallback, this));
	}
	void onSocketRecvCallback(const weak_ptr<Socket>& sock, const char* buffer, int len)
	{
		prevalivetime = Time::getCurrentMilliSecond();
		char* recvbufferaddr = (char*)recvBuffer.c_str();
		if (len > 0)
		{
			recvBuffer.resize(recvBuffer.length() + len);
			const char* buffertmp = recvbufferaddr;
			uint32_t buffertmplen = recvBuffer.length();

			while (buffertmplen > 0)
			{
				if (!recvHeader)
				{
					uint32_t usedlen = 0;
					recvHeader = parse(buffer, len, usedlen);

					buffertmp += usedlen;
					buffertmplen -= usedlen;

					if (recvHeader)
					{
						Value contentlenval = recvHeader->header(Content_Length);
						if (!contentlenval.empty()) recvContentLen = contentlenval.readUint64();

						shared_ptr<HTTPCommunicationHandler> handlerobj = handler.lock();
						if (handlerobj)
						{
							handlerobj->onRecvHeaderOk(this);
						}
					}
				}
				else if(recvContentLen == -1 || recvContentLen > recvContentTotalLen)
				{
					uint32_t needsize = buffertmplen;
					if (recvContentLen != -1) needsize = min(recvContentLen - recvContentTotalLen, buffertmplen);

					recvContent->append(buffertmp, needsize);

					recvContentTotalLen += needsize;
					buffertmplen -= needsize;
					buffertmp += needsize;
				}
				if (recvContentLen != -1 && recvContentLen == recvContentTotalLen)
				{
					shared_ptr<HTTPCommunicationHandler> handlerobj = handler.lock();
					if (handlerobj)
					{
						handlerobj->onRecvContentOk(this);
					}
					break;
				}
			}
			
			if (buffertmplen > 0 && recvbufferaddr != buffertmp)
			{
				memmove(recvbufferaddr, buffertmp, buffertmplen);
			}
			recvBuffer.resize(buffertmplen);
		}
		
		shared_ptr<Socket> socktmp = socket;
		if(socktmp) socktmp->async_recv(recvbufferaddr + recvBuffer.length(), MAXHTTPCACHELEN - recvBuffer.length(), Socket::ReceivedCallback(&HTTPCommunication::onSocketRecvCallback, this));
	}
	void onSocketSendCallback(const weak_ptr<Socket>& sock, const char* buffer, int len)
	{
		prevalivetime = Time::getCurrentMilliSecond();
		if (len < 0) return;

		checkSendResultAndPostSend(true, len);
	}
	void onSocketDisconnectCallback(const weak_ptr<Socket>& sock, const std::string&)
	{
		shared_ptr<HTTPCommunicationHandler> handlerobj = handler.lock();
		if (handlerobj) handlerobj->onDisconnect(this);
	}
private:
	uint64_t					prevalivetime;

	Mutex						mutex;
	weak_ptr<HTTPCommunicationHandler> handler;


	bool						isServer;
public:	
	shared_ptr<Socket>			socket;

	shared_ptr<HTTPHeader>		recvHeader;
	shared_ptr<IContent>		recvContent;
private:
	uint64_t					recvContentLen;
	uint64_t					recvContentTotalLen;

	String						recvBuffer;
public:
	shared_ptr<HTTPHeader>		sendHeader;
	shared_ptr<IContent>		sendContent;
private:
	uint64_t					sendContentLen;

	uint64_t					sendHeaderLen;
	uint64_t					sendTotalLen;
	
	String						sendBuffer;

	std::string					useragent;
};

inline void buildErrorResponse(const shared_ptr<HTTPCommunication>& commu, uint32_t errcode, const std::string& errmsg)
{

}

}
}

