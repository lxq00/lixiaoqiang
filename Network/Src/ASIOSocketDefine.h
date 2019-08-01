#ifndef __ASIOSOCKET_OBJCET_DEFINE_H__
#define __ASIOSOCKET_OBJCET_DEFINE_H__
#include "ASIOServerPool.h"
#include "Network/Socket.h"
#include "Network/TcpClient.h"
#include "UserThreadInfo.h"

#ifdef WIN32
typedef int  socklen_t;
#endif

namespace Public{
namespace Network{

///发送基类指针、普通的消息发送结构体
struct SendCallbackObject
{
public:
	SendCallbackObject(const weak_ptr<Socket>&_sock, const shared_ptr<UserThreadInfo>& _userthread, const Socket::SendedCallback& _callback, const char* buffer,size_t len)
		:sock(_sock),userthread(_userthread), callback(_callback), bufferaddr(buffer),bufferlen(len){}
	~SendCallbackObject() {}

	void _sendCallbackPtr(const boost::system::error_code& er, std::size_t length)
	{
		shared_ptr<UserThreadInfo> userthreadobj = userthread.lock();
		if (userthreadobj == NULL || !userthreadobj->callbackThreadUsedStart())
		{
			return;
		}

		shared_ptr<Socket>sockptr = sock.lock();
		if (sockptr)
		{
			callback(sockptr, bufferaddr, (er || length > bufferlen) ? 0 : length);
		}		

		userthreadobj->callbackThreadUsedEnd();
	}
private:
	weak_ptr<Socket>			sock;
	weak_ptr<UserThreadInfo>	userthread;
	Socket::SendedCallback		callback;
public:
	const char*					bufferaddr;
	size_t						bufferlen;
	NetAddr						toaddr;
};

struct RecvCallbackObject
{
public:
	RecvCallbackObject(const weak_ptr<Socket>& _sock, const shared_ptr< UserThreadInfo>& _userthread, const Socket::ReceivedCallback& _recvcallback,char* buffer = NULL, size_t len = 0)
		:sock(_sock), userthread(_userthread), recvcallback(_recvcallback), authenfree(false), bufferaddr(buffer), bufferlen(len)
	{
		if (bufferaddr == NULL)
		{
			authenfree = true;
			bufferaddr = new char[bufferlen];
		}
	}
	RecvCallbackObject(const weak_ptr<Socket>& _sock, const shared_ptr<UserThreadInfo>& _userthread, const Socket::RecvFromCallback& _recvcallback,
		char* buffer = NULL, size_t len = 0)
		:sock(_sock), userthread(_userthread), recvfromcallback(_recvcallback), authenfree(false), bufferaddr(buffer), bufferlen(len)
	{
		if (bufferaddr == NULL)
		{
			authenfree = true;
			bufferaddr = new char[bufferlen];
		}
	}

	~RecvCallbackObject()
	{
		if (authenfree) SAFE_DELETEARRAY(bufferaddr);
	}

	void _recvCallbackPtr(const boost::system::error_code& er, std::size_t length)
	{
		shared_ptr<UserThreadInfo> userthreadobj = userthread.lock();
		if (userthreadobj == NULL || !userthreadobj->callbackThreadUsedStart())
		{
			return;
		}

		shared_ptr<Socket>sockptr = sock.lock();
		if (sockptr)
		{
			if (er)
			{
				sockptr->socketError(er.message());
			}
			else if (recvcallback)
			{
				recvcallback(sockptr, bufferaddr, (er && length > bufferlen) ? 0 : length);
			}
			else
			{
				SockAddrIPv4 ipv4 = { 0 };
				ipv4.sin_family = AF_INET;
				ipv4.sin_addr.s_addr = recvpoint.address().to_v4().to_ulong();
				ipv4.sin_port = htons(recvpoint.port());
				
				recvfromcallback(sockptr, bufferaddr, (er && length > bufferlen) ? 0 : length, NetAddr(ipv4));
			}
		}
		
		userthreadobj->callbackThreadUsedEnd();
	}
private:
	weak_ptr<Socket>			sock;
	weak_ptr<UserThreadInfo>	userthread;

	Socket::ReceivedCallback	recvcallback;
	Socket::RecvFromCallback	recvfromcallback;

	bool						authenfree;
public:
	char*						bufferaddr;
	size_t						bufferlen;
	boost::asio::ip::udp::endpoint	recvpoint;
};

struct AcceptCallbackObject
{
public:
	AcceptCallbackObject(const weak_ptr<Socket>& _sock, const shared_ptr<UserThreadInfo>& _userthread,const Socket::AcceptedCallback& _callback)
		:sock(_sock),userthread(_userthread),acceptcallback(_callback)
	{

	}
	~AcceptCallbackObject() {}

	void _acceptCallbackPtr(const boost::system::error_code& er)
	{
		shared_ptr<UserThreadInfo> userthreadobj = userthread.lock();
		if (userthreadobj == NULL || !userthreadobj->callbackThreadUsedStart())
		{
			return;
		}

		shared_ptr<Socket>sockptr = sock.lock();
		if (sockptr)
		{
			if (er)
			{
				logdebug("%s %d _acceptCallbackPtr std::exception %s\r\n", __FUNCTION__, __LINE__, er.message().c_str());

				acceptcallback(sockptr, shared_ptr<Socket>());
			}
			else
			{
				newsock->socketReady();

				acceptcallback(sock, newsock);
			}
		}

		userthreadobj->callbackThreadUsedEnd();
	}
private:
	weak_ptr<Socket>			sock;
	weak_ptr<UserThreadInfo>	userthread;

	Socket::AcceptedCallback	acceptcallback;
public:
	shared_ptr<TCPClient>		newsock;
};

struct ConnectCallbackObject
{
public:
	ConnectCallbackObject(const weak_ptr<Socket>& _sock,const shared_ptr<UserThreadInfo>& _userthread,const Socket::ConnectedCallback& _callback)
		:sock(_sock),userthread(_userthread),callback(_callback)
	{}
	~ConnectCallbackObject() {}

	void _connectCallbackPtr(const boost::system::error_code& er)
	{
		shared_ptr<UserThreadInfo> userthreadobj = userthread.lock();
		if (userthreadobj == NULL || !userthreadobj->callbackThreadUsedStart())
		{
			return;
		}

		shared_ptr<Socket>sockptr = sock.lock();
		if (sockptr)
		{
			if(!er)
				sockptr->socketReady();

			callback(sockptr, !er, er ? er.message() : "success");
		}		

		userthreadobj->callbackThreadUsedEnd();
	}
private:
	weak_ptr<Socket>			sock;
	weak_ptr<UserThreadInfo>	userthread;

	Socket::ConnectedCallback	callback;
};

}
}


#endif //__ASIOSOCKET_OBJCET_H__
