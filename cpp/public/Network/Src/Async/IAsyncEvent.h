#ifndef __IASYNCEVENT_H__
#define __IASYNCEVENT_H__
#include "Base/Base.h"
#include "Network/Network.h"
using namespace Public::Base;
using namespace Public::Network;

//支持模式定义
typedef enum
{
	SuportType_IOCP = 1,
	SuportType_EPOOL = 2,
	SuportType_POOL = 4,
	SuportType_SELECT = 8
}AsyncSuportType;

inline int SuportType()
{
	int suportType = SuportType_SELECT;
#ifdef WIN32
	suportType |= SuportType_IOCP;
	suportType |= SuportType_POOL;
#else
	suportType |= SuportType_EPOOL;
	suportType |= SuportType_POOL;
#endif

	return suportType;
}


inline shared_ptr<Socket> buildSocketBySock(int sock,const NetAddr& otheraaddr);

typedef enum
{
	EventType_Read = 1,
	EventType_Write = 2,
	EventType_Error = 4,
}EventType;



class IAsyncEvent
{
public:
	IAsyncEvent() :doSuccess(false), runTimes(0) {}
	virtual ~IAsyncEvent() {}

	virtual bool doPrevEvent(const shared_ptr<Socket>& sock) { return true; }
	virtual bool doCanEvent(const shared_ptr<Socket>& sock,int flag = 0) = 0;
	virtual bool doResultEvent(const shared_ptr<Socket>& sock, int flag = 0, const std::string& context = "") = 0;

	bool doSuccess;
	uint32_t runTimes;
};

struct ConnectEvent :public IAsyncEvent
{
	NetAddr						otheraddr;
	Socket::ConnectedCallback	callback;

	virtual bool doResultEvent(const shared_ptr<Socket>& sock, int flag = 0, const std::string& context = "")
	{
		shared_ptr<Socket>		s = sock;
		if (s != NULL)
		{
			callback(s);
		}
		doSuccess = true;

		return true;
	}
};


struct AcceptEvent :public IAsyncEvent
{
	NetAddr						otheraaddr;
	Socket::AcceptedCallback	callback;

	AcceptEvent() {}
	virtual ~AcceptEvent() {}
	virtual bool doResultEvent(const shared_ptr<Socket>& sock, int flag, const std::string& context = "")
	{
		shared_ptr<Socket> ns = buildSocketBySock(flag, otheraaddr);
		shared_ptr<Socket>	s = sock;
		if (s != NULL && ns != NULL)
		{
			callback(s, ns);
		}

		//需要一直进行
		return false;
	}
};

struct SendEvent :public IAsyncEvent
{
	const char*					buffer;
	int							bufferlen;
	shared_ptr<NetAddr>			otheraddr;
	Socket::SendedCallback		callback;

	virtual bool doResultEvent(const shared_ptr<Socket>& sock, int flag, const std::string& context = "")
	{
		shared_ptr<Socket>	s = sock;
		if (s != NULL)
		{
			callback(s, buffer, flag);
		}
		doSuccess = true;
		return true;
	}
};

struct RecvEvent :public IAsyncEvent
{
	shared_ptr<NetAddr>			otheraddr;
	char*						buffer;
	int							bufferlen;
	Socket::ReceivedCallback	recvCallback;
	Socket::RecvFromCallback	recvFromCallback;

	virtual bool doResultEvent(const shared_ptr<Socket>& sock, int flag, const std::string& context = "")
	{
		shared_ptr<Socket>	s = sock;
		if (s != NULL)
		{
			if (otheraddr == NULL)
			{
				recvCallback(s, buffer, flag);
			}
			else
			{
				recvFromCallback(s, buffer, flag, *otheraddr.get());
			}
		}
		doSuccess = true;
		return true;
	}
};

struct DisconnectEvent :public IAsyncEvent
{
	Socket::DisconnectedCallback callback;
	virtual bool doCanEvent(const shared_ptr<Socket>& sock,int flag)
	{
		int sockfd = sock->getHandle();

		fd_set readfd;
		FD_ZERO(&readfd);
		FD_SET(sockfd, &readfd);
		timeval timeout;
		timeout.tv_sec = 0;
		timeout.tv_usec = 0;
		int ret = select(sockfd + 1, &readfd, NULL, NULL, &timeout);
		if (ret > 0)
		{
			unsigned long buffersize = 0;
#ifdef WIN32
			int ioret = ::ioctlsocket(sockfd, FIONREAD, &buffersize);
#else
			int ioret = ::ioctl(handle, FIONREAD, &buffersize);
#endif
			if (ioret >= 0 && buffersize <= 0)
			{
				return doResultEvent(sock, 0, "Socket Disconnect");
			}
		}

		return false;
	}
	virtual bool doResultEvent(const shared_ptr<Socket>& sock, int flag, const std::string& context)
	{
		shared_ptr<Socket>	s = sock;
		if (s != NULL)
		{
			callback(s, context);
		}
		doSuccess = true;
		return true;
	}
};




#endif //__IASYNCEVENT_H__
