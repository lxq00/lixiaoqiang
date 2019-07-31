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

		callback(sock, bufferaddr, (er || length >= bufferlen) ? 0 : length);

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
	typedef Function1<void, const boost::system::error_code& > ErrorCallback;
public:
	RecvCallbackObject(const weak_ptr<Socket>& _sock, const shared_ptr< UserThreadInfo>& _userthread, const Socket::ReceivedCallback& _recvcallback,
		const ErrorCallback& _errcalblack,char* buffer = NULL, size_t len = 0)
		:sock(_sock), userthread(_userthread), recvcallback(_recvcallback), authenfree(false), bufferaddr(buffer), bufferlen(len),errcallback(_errcalblack)
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

		if (er)
		{
			errcallback(er);
		}

		if (recvcallback)
		{
			recvcallback(sock, bufferaddr, (er && length > bufferlen) ? 0 : length);
		}
		else
		{
			NetAddr recvaddr(recvpoint.address().to_string(), recvpoint.port());

			recvfromcallback(sock, bufferaddr, (er && length > bufferlen) ? 0 : length, recvaddr);
		}
		userthreadobj->callbackThreadUsedEnd();
	}
private:
	weak_ptr<Socket>			sock;
	weak_ptr<UserThreadInfo>	userthread;

	Socket::ReceivedCallback	recvcallback;
	Socket::RecvFromCallback	recvfromcallback;

	bool						authenfree;

	ErrorCallback				errcallback;
public:
	char*						bufferaddr;
	size_t						bufferlen;
	boost::asio::ip::udp::endpoint	recvpoint;
};

struct AcceptCallbackObject
{
public:
	typedef Function2<void, const boost::system::error_code&, const Socket::AcceptedCallback&> ErrorCallback;
public:
	AcceptCallbackObject(const weak_ptr<Socket>& _sock, const shared_ptr<UserThreadInfo>& _userthread,
		const Socket::AcceptedCallback& _callback, const ErrorCallback& _errcallback)
		:sock(_sock),userthread(_userthread),acceptcallback(_callback),errcallback(_errcallback)
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

		if (er)
		{
			errcallback(er, acceptcallback);
		}
		else
		{
			newsock->socketReady();

			acceptcallback(sock, newsock);
		}

		userthreadobj->callbackThreadUsedEnd();
	}
private:
	weak_ptr<Socket>			sock;
	weak_ptr<UserThreadInfo>	userthread;

	Socket::AcceptedCallback	acceptcallback;

	ErrorCallback				errcallback;
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

		{
			shared_ptr<Socket> sockobj = sock.lock();
			if (sockobj) sockobj->socketReady();
		}
		

		if (er)
		{
			callback(sock, er, er ? er.message() : "success");
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
