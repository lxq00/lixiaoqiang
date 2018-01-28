#ifndef __IASYNCOBJECT_H__
#define __IASYNCOBJECT_H__
#include "Base/Base.h"
#include "Network/Network.h"
using namespace Public::Base;
using namespace Public::Network;
#include "IAsyncEvent.h"

#define EVENTRUNTIMES	5

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
	virtual ~AsyncObject() {}
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
	virtual void cleanSocketEvent(const shared_ptr<Socket>& sock, EventType type,const shared_ptr<DoingAsyncInfo>& doasyncinfo,const shared_ptr<IAsyncEvent>& event) {}
	virtual void addSocketEvent(const shared_ptr<Socket>& sock, EventType type, const shared_ptr<DoingAsyncInfo>& doasyncinfo, const shared_ptr<IAsyncEvent>& event) {}
	virtual void addSocket(const shared_ptr<Socket>& sock) {}
	virtual void cleanSocketAllEvent(const shared_ptr<Socket>& sock) {}
	virtual void addSocketAllEvent(const shared_ptr<Socket>& sock, int event) {}
	void cleanSuccessEvent(shared_ptr<AsyncInfo> asyfncinfo,shared_ptr<DoingAsyncInfo> doasyncinfo)
	{
		shared_ptr<Socket> sock = asyfncinfo->sock.lock();
		if (sock == NULL) return;
		if (doasyncinfo->acceptEvent != NULL && doasyncinfo->acceptEvent->doSuccess)
		{
			cleanSocketEvent(sock, EventType_Read, asyfncinfo, doasyncinfo->acceptEvent);
			doasyncinfo->acceptEvent = NULL;
		}
		if (doasyncinfo->connectEvent != NULL && doasyncinfo->connectEvent->doSuccess)
		{
			cleanSocketEvent(sock, EventType_Write, asyfncinfo, doasyncinfo->connectEvent);
			doasyncinfo->connectEvent = NULL;
		}
		if (doasyncinfo->recvEvent != NULL && doasyncinfo->recvEvent->doSuccess)
		{
			cleanSocketEvent(sock, EventType_Read, asyfncinfo, doasyncinfo->recvEvent);
			doasyncinfo->recvEvent = NULL;
		}
		if (doasyncinfo->sendEvent != NULL && doasyncinfo->sendEvent->doSuccess)
		{
			cleanSocketEvent(sock, EventType_Write, asyfncinfo, doasyncinfo->sendEvent);
			doasyncinfo->sendEvent = NULL;
		}
		if (doasyncinfo->disconnedEvent != NULL && doasyncinfo->disconnedEvent->doSuccess)
		{
			cleanSocketEvent(sock, EventType_Error, asyfncinfo, doasyncinfo->disconnedEvent);
			doasyncinfo->disconnedEvent = NULL;
		}
	}
	bool _rebuildDoingEvent(shared_ptr<Socket>& sock,shared_ptr<AsyncInfo>& asyncinfo, shared_ptr<DoingAsyncInfo>& doasyncinfo)
	{
		cleanSocketAllEvent(sock);
		cleanSuccessEvent(asyncinfo, doasyncinfo);
		if (asyncinfo->sendList.size() > 0 && doasyncinfo->sendEvent == NULL)
		{
			doasyncinfo->sendEvent = asyncinfo->sendList.front();
			asyncinfo->sendList.pop_front();
			addSocketEvent(sock, EventType_Write, asyncinfo, doasyncinfo->sendEvent);
		}
		if (asyncinfo->recvList.size() > 0 && doasyncinfo->recvEvent == NULL)
		{
			doasyncinfo->recvEvent = asyncinfo->recvList.front();
			asyncinfo->recvList.pop_front();
			addSocketEvent(sock, EventType_Read, asyncinfo, doasyncinfo->recvEvent);
		}
		if (asyncinfo->connectList.size() > 0 && doasyncinfo->connectEvent == NULL)
		{
			doasyncinfo->connectEvent = asyncinfo->connectList.front();
			asyncinfo->connectList.pop_front();
			addSocketEvent(sock, EventType_Write, asyncinfo, doasyncinfo->connectEvent);
		}
		if (asyncinfo->acceptList.size() > 0 && doasyncinfo->acceptEvent == NULL)
		{
			doasyncinfo->acceptEvent = asyncinfo->acceptList.front();
			asyncinfo->acceptList.pop_front();
			addSocketEvent(sock, EventType_Read, asyncinfo, doasyncinfo->acceptEvent);
		}
		if (asyncinfo->disconnectList.size() > 0 && doasyncinfo->disconnedEvent == NULL)
		{
			doasyncinfo->disconnedEvent = asyncinfo->disconnectList.front();
			asyncinfo->disconnectList.pop_front();
			addSocketEvent(sock, EventType_Error, asyncinfo, doasyncinfo->disconnedEvent);
		}

		int allevent = 0;
		if (doasyncinfo->sendEvent != NULL)
		{
			allevent |= EventType_Write;
		}
		if (doasyncinfo->recvEvent != NULL)
		{
			allevent |= EventType_Read;
		}
		if (doasyncinfo->connectEvent != NULL && doasyncinfo->connectEvent->runTimes++ % EVENTRUNTIMES == 0)
		{
			allevent |= EventType_Write;
		}
		if (doasyncinfo->acceptEvent != NULL)
		{
			allevent |= EventType_Read;
		}
		if (doasyncinfo->disconnedEvent != NULL)
		{
			allevent |= EventType_Error;
		}
		addSocketAllEvent(sock, allevent);
	}
	bool buildDoingEvent(const Socket* sockptr)
	{
		Guard locker(mutex);

		std::map<const Socket*, shared_ptr<AsyncInfo> >::iterator iter = socketList.find(sockptr);
		if (iter == socketList.end())
		{
			return true;
		}
		shared_ptr<Socket> sock = iter->second->sock.lock();
		if (sock == NULL) return;

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
		_rebuildDoingEvent(sock,iter->second, doasyncinfo);
	}
	bool buildDoingEvent()
	{
		Guard locker(mutex);

		std::map<const Socket*, shared_ptr<DoingAsyncInfo> >		doingtmp = doingList;
		for (std::map<const Socket*, shared_ptr<AsyncInfo> >::iterator iter = socketList.begin(); iter != socketList.end(); iter++)
		{
			shared_ptr<Socket> sock = iter->second->sock.lock();
			if (sock == NULL) continue;

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
			_rebuildDoingEvent(sock, iter->second, doasyncinfo);
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

			addSocket(sock);
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

			addSocket(sock);
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

			addSocket(sock);
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

			addSocket(sock);
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

			addSocket(sock);
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
