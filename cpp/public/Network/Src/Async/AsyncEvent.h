#ifndef __AsyncEvent_H__
#define __AsyncEvent_H__
#include "Base/Base.h"
#include "Network/Network.h"
using namespace Public::Base;
using namespace Public::Network;
#ifndef WIN32
#include <sys/ioctl.h>
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>
#endif

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
#else
	suportType |= SuportType_EPOOL;
	suportType |= SuportType_POOL;
#endif

	return suportType;
}

class AsyncManager;
class AsyncObject;
inline Public::Base::shared_ptr<Socket> buildSocketBySock(const Public::Base::weak_ptr<AsyncManager>& manager,int sock,const NetAddr& otheraaddr);
inline bool insertSocketResult(const Public::Base::weak_ptr<AsyncObject>& obj, int sock);
inline void eraseSocketResult(const Public::Base::weak_ptr<AsyncObject>& obj, int sock);


class AsynResultWaitObject
{
public:
	AsynResultWaitObject(const Public::Base::weak_ptr<AsyncObject>& _obj, const Public::Base::shared_ptr<Socket>& sock):obj(_obj),sockfd(-1)
	{
		if (sock != NULL)
		{
			sockfd = sock->getHandle();
		}
	}
	bool lock()
	{
		return insertSocketResult(obj, sockfd);
	}
	~AsynResultWaitObject()
	{
		eraseSocketResult(obj,sockfd);
	}
private:
	Public::Base::weak_ptr<AsyncObject> obj;
	int sockfd;
};

typedef enum
{
	EventType_Read = 1,
	EventType_Write = 2,
	EventType_Error = 4,
}EventType;



class AsyncEvent
{
public:
	AsyncEvent() :doSuccess(false), runTimes(0) {}
	virtual ~AsyncEvent() {}

	virtual bool doPrevEvent(const Public::Base::shared_ptr<Socket>& sock) { return true; }
	virtual bool doCanEvent(const Public::Base::shared_ptr<Socket>& sock,int flag = 0) = 0;
	virtual bool doResultEvent(const Public::Base::shared_ptr<Socket>& sock, int flag = 0, const std::string& context = "") = 0;
	virtual void* getFlag() { return NULL; }

	bool doSuccess;
	uint32_t runTimes;
	Public::Base::weak_ptr<AsyncManager> manager;
	Public::Base::weak_ptr<AsyncObject>	 asyncobj;
};

struct ConnectEvent :public AsyncEvent
{
	NetAddr						otheraddr;
	Socket::ConnectedCallback	callback;

	virtual bool doResultEvent(const Public::Base::shared_ptr<Socket>& sock, int flag = 0, const std::string& context = "")
	{
		Public::Base::shared_ptr<Socket>		s = sock;
		if (s != NULL)
		{
			AsynResultWaitObject wait(asyncobj, s);
			if(wait.lock()) callback(s);
		}
		doSuccess = true;

		return true;
	}
};


struct AcceptEvent :public AsyncEvent
{
	NetAddr						otheraaddr;
	Socket::AcceptedCallback	callback;

	AcceptEvent() {}
	virtual ~AcceptEvent() {}
	virtual bool doResultEvent(const Public::Base::shared_ptr<Socket>& sock, int flag, const std::string& context = "")
	{
		Public::Base::shared_ptr<Socket> ns = buildSocketBySock(manager,flag, otheraaddr);
		Public::Base::shared_ptr<Socket>	s = sock;
		if (s != NULL && ns != NULL)
		{
			AsynResultWaitObject wait(asyncobj, s);
			if (wait.lock()) callback(s, ns);
		}

		//需要一直进行
		doSuccess = false;

		return true;
	}
};

struct SendEvent :public AsyncEvent
{
	const char*					buffer;
	int							bufferlen;
	Public::Base::shared_ptr<NetAddr>			otheraddr;
	Socket::SendedCallback		callback;

	virtual bool doResultEvent(const Public::Base::shared_ptr<Socket>& sock, int flag, const std::string& context = "")
	{
		Public::Base::shared_ptr<Socket>	s = sock;
		if (s != NULL)
		{
			AsynResultWaitObject wait(asyncobj, s);
			if (wait.lock()) callback(s, buffer, flag);
		}
		doSuccess = true;
		return true;
	}
};

struct RecvEvent :public AsyncEvent
{
	Public::Base::shared_ptr<NetAddr>			otheraddr;
	char*						buffer;
	int							bufferlen;
	Socket::ReceivedCallback	recvCallback;
	Socket::RecvFromCallback	recvFromCallback;

	virtual bool doResultEvent(const Public::Base::shared_ptr<Socket>& sock, int flag, const std::string& context = "")
	{
		shared_ptr<Socket>	s = sock;
		if (s != NULL)
		{
			if (otheraddr == NULL)
			{
				AsynResultWaitObject wait(asyncobj, s);
				if (wait.lock()) recvCallback(s, buffer, flag);
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

struct DisconnectEvent :public AsyncEvent
{
	Socket::DisconnectedCallback callback;
	virtual bool doCanEvent(const Public::Base::shared_ptr<Socket>& sock,int flag)
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
			int ioret = ::ioctl(sockfd, FIONREAD, &buffersize);
#endif
			if (ioret >= 0 && buffersize <= 0)
			{
				return doResultEvent(sock, 0, "Socket Disconnect");
			}
		}

		return false;
	}
	virtual bool doResultEvent(const Public::Base::shared_ptr<Socket>& sock, int flag, const std::string& context)
	{
		Public::Base::shared_ptr<Socket>	s = sock;
		if (s != NULL)
		{
			AsynResultWaitObject wait(asyncobj, s);
			if (wait.lock()) callback(s, context.c_str());
		}
		doSuccess = true;
		return true;
	}
};




#endif //__AsyncEvent_H__
