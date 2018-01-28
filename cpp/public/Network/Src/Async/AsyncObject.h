#ifndef __IASYNCOBJECT_H__
#define __IASYNCOBJECT_H__
#include "Base/Base.h"
#include "Network/Network.h"
using namespace Public::Base;
using namespace Public::Network;
#include "IAsyncEvent.h"

class IAsyncEvent
{
public:
	IAsyncEvent():doSuccess(false){}
	virtual ~IAsyncEvent() {}

	virtual bool doCanEvent(const shared_ptr<Socket>& sock) = 0;
	virtual bool doResultEvent(const shared_ptr<Socket>& sock, int flag = 0, const std::string& context = "") = 0;

	bool doSuccess;
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

	virtual bool doResultEvent(const shared_ptr<Socket>& sock,int flag, const std::string& context = "")
	{
		shared_ptr<Socket> ns = buildSocketBySock(flag, otheraaddr);
		shared_ptr<Socket>	s = sock;
		if (s != NULL && ns != NULL)
		{
			callback(s,ns);
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

	virtual bool doResultEvent(const shared_ptr<Socket>& sock,int flag, const std::string& context = "")
	{
		shared_ptr<Socket>	s = sock;
		if (s != NULL)
		{
			callback(s, buffer,flag);
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

	virtual bool doResultEvent(const shared_ptr<Socket>& sock,int flag, const std::string& context = "")
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

struct DisconnectEvent:public IAsyncEvent
{
	Socket::DisconnectedCallback callback;
	virtual bool doCanEvent(const shared_ptr<Socket>& sock)
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

class AsyncObject
{
public:
	struct AsyncInfo
	{
		weak_ptr<Socket>						sock;
		std::list<shared_ptr<ConnectEvent> >	connectList;
		std::list<shared_ptr<AcceptEvent> >		acceptList;
		std::list<shared_ptr<SendEvent> >		sendList;
		std::list<shared_ptr<RecvEvent> >		recvList;
		std::list<shared_ptr<DisconnectEvent> >	disconnectList;
	};
protected:
	struct DoingAsyncInfo
	{
		weak_ptr<AsyncInfo>			asyncInfo;
		shared_ptr<IAsyncEvent>		connectEvent;
		shared_ptr<IAsyncEvent>		acceptEvent;
		shared_ptr<IAsyncEvent>		recvEvent;
		shared_ptr<IAsyncEvent>		sendEvent;
		shared_ptr<IAsyncEvent>		disconnedEvent;
	};
public:
	AsyncObject() {}
	virtual ~AsyncObject(){}
	virtual bool deleteSocket(const Socket* sock)
	{
		Guard locker(mutex);
		socketList.erase(sock);
		doingList.erase(sock);

		return true;
	}
	virtual shared_ptr<AsyncInfo> addConnectEvent(const shared_ptr<Socket>& sock, const NetAddr& othreaddr, const Socket::ConnectedCallback& callback) = 0;
	virtual shared_ptr<AsyncInfo> addAcceptEvent(const shared_ptr<Socket>& sock, const Socket::AcceptedCallback& callback) = 0; 
	virtual shared_ptr<AsyncInfo> addRecvFromEvent(const shared_ptr<Socket>& sock, char* buffer, int bufferlen, const Socket::RecvFromCallback& callback) = 0;
	virtual shared_ptr<AsyncInfo> addRecvEvent(const shared_ptr<Socket>& sock, char* buffer, int bufferlen, const Socket::ReceivedCallback& callback) = 0;
	virtual shared_ptr<AsyncInfo> addSendToEvent(const shared_ptr<Socket>& sock, const char* buffer, int bufferlen, const NetAddr& otheraddr, const Socket::SendedCallback& callback) = 0;
	virtual shared_ptr<AsyncInfo> addSendEvent(const shared_ptr<Socket>& sock, const char* buffer, int bufferlen, const Socket::SendedCallback& callback) = 0;
	virtual shared_ptr<AsyncInfo> addDisconnectEvent(const shared_ptr<Socket>& sock, const Socket::DisconnectedCallback& callback) = 0;
protected:
	void cleanSuccessEvent(shared_ptr<DoingAsyncInfo> doasyncinfo)
	{
		if (doasyncinfo->acceptEvent != NULL && doasyncinfo->acceptEvent->doSuccess)
		{
			doasyncinfo->acceptEvent = NULL;
		}
		if (doasyncinfo->connectEvent != NULL && doasyncinfo->connectEvent->doSuccess)
		{
			doasyncinfo->connectEvent = NULL;
		}
		if (doasyncinfo->recvEvent != NULL && doasyncinfo->recvEvent->doSuccess)
		{
			doasyncinfo->recvEvent = NULL;
		}
		if (doasyncinfo->sendEvent != NULL && doasyncinfo->sendEvent->doSuccess)
		{
			doasyncinfo->sendEvent = NULL;
		}
		if (doasyncinfo->disconnedEvent != NULL && doasyncinfo->disconnedEvent->doSuccess)
		{
			doasyncinfo->disconnedEvent = NULL;
		}
	}
	bool buildDoingEvent(const Socket* sock)
	{
		std::map<const Socket*, shared_ptr<AsyncInfo> >::iterator iter = socketList.find(sock);
		if (iter == socketList.end())
		{
			return true;
		}
		shared_ptr<DoingAsyncInfo> doasyncinfo;
		std::map<const Socket*, shared_ptr<DoingAsyncInfo> >::iterator diter = doingList.find(iter->first);
		if (diter != doingList.end())
		{
			doasyncinfo = diter->second;
		}
		else
		{
			doasyncinfo = new DoingAsyncInfo;
			doasyncinfo->asyncInfo = iter->second;
			doingList[iter->first] = doasyncinfo;
		}
		cleanSuccessEvent(doasyncinfo);
		if (iter->second->sendList.size() > 0 && doasyncinfo->sendEvent == NULL)
		{
			doasyncinfo->sendEvent = iter->second->sendList.front();
			iter->second->sendList.pop_front();
		}
		if (iter->second->recvList.size() > 0 && doasyncinfo->recvEvent == NULL)
		{
			doasyncinfo->recvEvent = iter->second->recvList.front();
			iter->second->recvList.pop_front();
		}
		if (iter->second->connectList.size() > 0 && doasyncinfo->connectEvent == NULL)
		{
			doasyncinfo->connectEvent = iter->second->connectList.front();
			iter->second->connectList.pop_front();
		}
		if (iter->second->disconnectList.size() > 0 && doasyncinfo->disconnedEvent == NULL)
		{
			doasyncinfo->disconnedEvent = iter->second->disconnectList.front();
			iter->second->disconnectList.pop_front();
		}
	}
	bool buildDoingEvent()
	{
		Guard locker(mutex);

		std::map<const Socket*, shared_ptr<DoingAsyncInfo> >		doingtmp = doingList;
		for (std::map<const Socket*, shared_ptr<AsyncInfo> >::iterator iter = socketList.begin(); iter != socketList.end(); iter++)
		{
			shared_ptr<DoingAsyncInfo> doasyncinfo;
			std::map<const Socket*, shared_ptr<DoingAsyncInfo> >::iterator diter = doingList.find(iter->first);
			if (diter != doingList.end())
			{
				doasyncinfo = diter->second;
				doingtmp.erase(iter->first);
			}
			else
			{
				doasyncinfo = new DoingAsyncInfo;
				doasyncinfo->asyncInfo = iter->second;
				doingList[iter->first] = doasyncinfo;
			}
			cleanSuccessEvent(doasyncinfo);
			if (iter->second->sendList.size() > 0 && doasyncinfo->sendEvent == NULL)
			{
				doasyncinfo->sendEvent = iter->second->sendList.front();
				iter->second->sendList.pop_front();
			}
			if (iter->second->recvList.size() > 0 && doasyncinfo->recvEvent == NULL)
			{
				doasyncinfo->recvEvent = iter->second->recvList.front();
				iter->second->recvList.pop_front();
			}
			if (iter->second->connectList.size() > 0 && doasyncinfo->connectEvent == NULL)
			{
				doasyncinfo->connectEvent = iter->second->connectList.front();
				iter->second->connectList.pop_front();
			}
			if (iter->second->disconnectList.size() > 0 && doasyncinfo->disconnedEvent == NULL)
			{
				doasyncinfo->disconnedEvent = iter->second->disconnectList.front();
				iter->second->disconnectList.pop_front();
			}
		}
		for (std::map<const Socket*, shared_ptr<DoingAsyncInfo> >::iterator iter = doingtmp.begin(); iter != doingtmp.end(); iter++)
		{
			doingList.erase(iter->first);
		}
	}
	virtual shared_ptr<AsyncInfo> addConnectEvent(const shared_ptr<Socket>& sock, const shared_ptr<ConnectEvent>& event)
	{
		Guard locker(mutex);

		shared_ptr<AsyncInfo> asyncinfo;

		std::map<const Socket*, shared_ptr<AsyncInfo> >::iterator iter = socketList.find(sock.get());
		if (iter != socketList.end())
		{
			asyncinfo = iter->second;
		}
		else
		{
			asyncinfo = new AsyncInfo;
			asyncinfo->sock = sock;
			socketList[sock.get()] = asyncinfo;
		}

		asyncinfo->connectList.clear();
		asyncinfo->connectList.push_back(event);

		return asyncinfo;
	}
	virtual shared_ptr<AsyncInfo> addAcceptEvent(const shared_ptr<Socket>& sock, const shared_ptr<AcceptEvent>& event)
	{
		Guard locker(mutex);

		shared_ptr<AsyncInfo> asyncinfo;

		std::map<const Socket*, shared_ptr<AsyncInfo> >::iterator iter = socketList.find(sock.get());
		if (iter != socketList.end())
		{
			asyncinfo = iter->second;
		}
		else if (event != NULL)
		{
			asyncinfo = new AsyncInfo;
			asyncinfo->sock = sock;
			socketList[sock.get()] = asyncinfo;
		}
		if (asyncinfo != NULL)
		{
			asyncinfo->acceptList.clear();
			if (event != NULL)
			{
				asyncinfo->acceptList.push_back(event);
			}
		}

		return asyncinfo;
	}
	virtual shared_ptr<AsyncInfo> addRecvEvent(const shared_ptr<Socket>& sock, const shared_ptr<RecvEvent>& event)
	{
		Guard locker(mutex);

		shared_ptr<AsyncInfo> asyncinfo;

		std::map<const Socket*, shared_ptr<AsyncInfo> >::iterator iter = socketList.find(sock.get());
		if (iter != socketList.end())
		{
			asyncinfo = iter->second;
		}
		else
		{
			asyncinfo = new AsyncInfo;
			asyncinfo->sock = sock;
			socketList[sock.get()] = asyncinfo;
		}

		asyncinfo->recvList.push_back(event);


		return asyncinfo;
	}
	virtual shared_ptr<AsyncInfo> addSendEvent(const shared_ptr<Socket>& sock,const shared_ptr<SendEvent>& event)
	{
		Guard locker(mutex);

		shared_ptr<AsyncInfo> asyncinfo;

		std::map<const Socket*, shared_ptr<AsyncInfo> >::iterator iter = socketList.find(sock.get());
		if (iter != socketList.end())
		{
			asyncinfo = iter->second;
		}
		else
		{
			asyncinfo = new AsyncInfo;
			asyncinfo->sock = sock;
			socketList[sock.get()] = asyncinfo;
		}

		asyncinfo->sendList.push_back(event);

		return asyncinfo;
	}	
	virtual shared_ptr<AsyncInfo> addDisconnectEvent(const shared_ptr<Socket>& sock, const shared_ptr<DisconnectEvent>& event)
	{
		Guard locker(mutex);

		shared_ptr<AsyncInfo> asyncinfo;

		std::map<const Socket*, shared_ptr<AsyncInfo> >::iterator iter = socketList.find(sock.get());
		if (iter != socketList.end())
		{
			asyncinfo = iter->second;
		}
		else if (event != NULL)
		{
			asyncinfo = new AsyncInfo;
			asyncinfo->sock = sock;
			socketList[sock.get()] = asyncinfo;
		}
		if (asyncinfo != NULL)
		{
			asyncinfo->disconnectList.clear();
			if (event != NULL) 
			{
				asyncinfo->disconnectList.push_back(event);
			}
		}

		return asyncinfo;
	}
protected:	
	Mutex										mutex;
	std::map<const Socket*, shared_ptr<AsyncInfo> >	socketList;
protected:	
	std::map<const Socket*, shared_ptr<DoingAsyncInfo> >	doingList;
};


#endif //__IASYNCOBJECT_H__
