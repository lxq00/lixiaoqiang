#ifndef __IASYNCOBJECT_H__
#define __IASYNCOBJECT_H__
#include "Base/Base.h"
#include "Network/Network.h"
using namespace Public::Base;
using namespace Public::Network;
#include "AsyncEvent.h"

#define EVENTRUNTIMES	2

class AsyncObject:public Public::Base::enable_shared_from_this<AsyncObject>
{
public:
	struct AsyncInfo
	{
		Public::Base::weak_ptr<Socket>						sock;
		std::list<Public::Base::shared_ptr<ConnectEvent> >	connectList;
		std::list<Public::Base::shared_ptr<AcceptEvent> >	acceptList;
		std::list<Public::Base::shared_ptr<SendEvent> >		sendList;
		std::list<Public::Base::shared_ptr<RecvEvent> >		recvList;
		std::list<Public::Base::shared_ptr<DisconnectEvent> >	disconnectList;
	};
protected:
	struct DoingAsyncInfo
	{
		Public::Base::shared_ptr<AsyncInfo>			asyncInfo;
		Public::Base::shared_ptr<AsyncEvent>		connectEvent;
		Public::Base::shared_ptr<AsyncEvent>		acceptEvent;
		Public::Base::shared_ptr<AsyncEvent>		recvEvent;
		Public::Base::shared_ptr<AsyncEvent>		sendEvent;
		Public::Base::shared_ptr<AsyncEvent>		disconnedEvent;
		int											events;
		DoingAsyncInfo():events(0) {}
	};
public:
	AsyncObject(const Public::Base::shared_ptr<AsyncManager>& _manager, AsyncSuportType _type):manager(_manager),type(_type) {}
	virtual ~AsyncObject() {}
	virtual AsyncSuportType asyncType() { return type; }
	bool lockSocketUsing(const Public::Base::shared_ptr<Socket>& sock)
	{
		Guard locker(mutex);
		std::map<Socket*, Public::Base::shared_ptr<AsyncInfo> >::iterator siter = socketList.find(sock.get());
		if (siter == socketList.end())
		{
			return false;
		}
		std::map<Socket*, std::set<uint32_t> >::iterator iter = sockUsingList.find(sock.get());
		if (iter == sockUsingList.end())
		{
			sockUsingList[sock.get()] = std::set<uint32_t>();
			iter = sockUsingList.find(sock.get());
		}
		iter->second.insert(Thread::getCurrentThreadID());

		return true;
	}
	void unlockSocketUsing(const Public::Base::shared_ptr<Socket>& sock)
	{
		Guard locker(mutex);

		std::map<Socket*, std::set<uint32_t> >::iterator iter = sockUsingList.find(sock.get());
		if (iter != sockUsingList.end())
		{
			iter->second.erase(Thread::getCurrentThreadID());
		}
	}
	virtual bool deleteSocket(Socket* sockptr,int sockfd)
	{
		{
			Guard locker(mutex);
			socketList.erase(sockptr);
			doingList.erase(sockptr);
		}
		while (true)
		{
			{
				Guard locker(mutex);

				std::map<Socket*, std::set<uint32_t> >::iterator iter = sockUsingList.find(sockptr);
				if (iter == sockUsingList.end())
				{
					break;
				}
				if (iter->second.size() <= 0 || 
					(iter->second.size() == (uint32_t)1 && *iter->second.begin() == (uint32_t)Thread::getCurrentThreadID()))
				{
					sockUsingList.erase(iter);
				}
			}
			Thread::sleep(10);
		}
		return true;
	}
	virtual bool addSocket(const Public::Base::shared_ptr<Socket>& sock)
	{
		Guard locker(mutex);
		Public::Base::shared_ptr<AsyncInfo> asyncinfo(new AsyncInfo);
		asyncinfo->sock = sock;
		socketList[sock.get()] = asyncinfo;

		return true;
	}
	virtual uint32_t getSocketCount()
	{
		Guard locker(mutex);
		return socketList.size();
	}
	virtual bool addConnect(const Public::Base::shared_ptr<Socket>& sock, const NetAddr& othreaddr, const Socket::ConnectedCallback& callback) = 0;
	virtual bool addAccept(const Public::Base::shared_ptr<Socket>& sock, const Socket::AcceptedCallback& callback) = 0;
	virtual bool addRecvfrom(const Public::Base::shared_ptr<Socket>& sock, char* buffer, int bufferlen, const Socket::RecvFromCallback& callback) = 0;
	virtual bool addRecv(const Public::Base::shared_ptr<Socket>& sock, char* buffer, int bufferlen, const Socket::ReceivedCallback& callback) = 0;
	virtual bool addSendto(const Public::Base::shared_ptr<Socket>& sock, const char* buffer, int bufferlen, const NetAddr& otheraddr, const Socket::SendedCallback& callback) = 0;
	virtual bool addSend(const Public::Base::shared_ptr<Socket>& sock, const char* buffer, int bufferlen, const Socket::SendedCallback& callback) = 0;
	virtual bool addDisconnect(const Public::Base::shared_ptr<Socket>& sock, const Socket::DisconnectedCallback& callback) = 0;
protected:
	virtual void _cleanSocketEvent(const Public::Base::shared_ptr<Socket>& sock, EventType type,const Public::Base::shared_ptr<DoingAsyncInfo>& doasyncinfo,const Public::Base::shared_ptr<AsyncEvent>& event) {}
	virtual void _addSocketEvent(const Public::Base::shared_ptr<Socket>& sock, EventType type, const Public::Base::shared_ptr<DoingAsyncInfo>& doasyncinfo, const Public::Base::shared_ptr<AsyncEvent>& event) {}
	void _cleanSuccessEvent(const Public::Base::shared_ptr<Socket>& sock, const Public::Base::shared_ptr<AsyncInfo>& asyfncinfo, const Public::Base::shared_ptr<DoingAsyncInfo>& doasyncinfo)
	{
		if (doasyncinfo->acceptEvent != NULL && doasyncinfo->acceptEvent->doSuccess)
		{
			_cleanSocketEvent(sock, EventType_Read, doasyncinfo, doasyncinfo->acceptEvent);
			doasyncinfo->acceptEvent = NULL;
		}
		if (doasyncinfo->connectEvent != NULL && doasyncinfo->connectEvent->doSuccess)
		{
			_cleanSocketEvent(sock, EventType_Write, doasyncinfo, doasyncinfo->connectEvent);
			doasyncinfo->connectEvent = NULL;
		}
		if (doasyncinfo->recvEvent != NULL && doasyncinfo->recvEvent->doSuccess)
		{
			_cleanSocketEvent(sock, EventType_Read, doasyncinfo, doasyncinfo->recvEvent);
			doasyncinfo->recvEvent = NULL;
		}
		if (doasyncinfo->sendEvent != NULL && doasyncinfo->sendEvent->doSuccess)
		{
			_cleanSocketEvent(sock, EventType_Write, doasyncinfo, doasyncinfo->sendEvent);
			doasyncinfo->sendEvent = NULL;
		}
		if (doasyncinfo->disconnedEvent != NULL && doasyncinfo->disconnedEvent->doSuccess)
		{
			_cleanSocketEvent(sock, EventType_Error, doasyncinfo, doasyncinfo->disconnedEvent);
			doasyncinfo->disconnedEvent = NULL;
		}
	}
	bool _rebuildDoingEvent(const Public::Base::shared_ptr<Socket>& sock,const Public::Base::shared_ptr<AsyncInfo>& asyncinfo,const Public::Base::shared_ptr<DoingAsyncInfo>& doasyncinfo)
	{
		_cleanSuccessEvent(sock, asyncinfo, doasyncinfo);
		if (asyncinfo->sendList.size() > 0 && doasyncinfo->sendEvent == NULL)
		{
			doasyncinfo->sendEvent = asyncinfo->sendList.front();
			asyncinfo->sendList.pop_front();
			_addSocketEvent(sock, EventType_Write, doasyncinfo, doasyncinfo->sendEvent);
		}
		if (asyncinfo->recvList.size() > 0 && doasyncinfo->recvEvent == NULL)
		{
			doasyncinfo->recvEvent = asyncinfo->recvList.front();
			asyncinfo->recvList.pop_front();
			_addSocketEvent(sock, EventType_Read, doasyncinfo, doasyncinfo->recvEvent);
		}
		if (asyncinfo->connectList.size() > 0 && doasyncinfo->connectEvent == NULL)
		{
			doasyncinfo->connectEvent = asyncinfo->connectList.front();
			asyncinfo->connectList.pop_front();
			_addSocketEvent(sock, EventType_Write, doasyncinfo, doasyncinfo->connectEvent);
		}
		if (asyncinfo->acceptList.size() > 0 && doasyncinfo->acceptEvent == NULL)
		{
			doasyncinfo->acceptEvent = asyncinfo->acceptList.front();
			asyncinfo->acceptList.pop_front();
			_addSocketEvent(sock, EventType_Read, doasyncinfo, doasyncinfo->acceptEvent);
		}
		if (asyncinfo->disconnectList.size() > 0 && doasyncinfo->disconnedEvent == NULL)
		{
			doasyncinfo->disconnedEvent = asyncinfo->disconnectList.front();
			asyncinfo->disconnectList.pop_front();
			_addSocketEvent(sock, EventType_Error, doasyncinfo, doasyncinfo->disconnedEvent);
		}

		return true;
	}
	bool buildDoingEvent(const Public::Base::shared_ptr<Socket>& sock)
	{
		Guard locker(mutex);

		std::map<Socket*, Public::Base::shared_ptr<AsyncInfo> >::iterator iter = socketList.find(sock.get());
		if (iter == socketList.end())
		{
			return true;
		}
		Public::Base::shared_ptr<DoingAsyncInfo> doasyncinfo;
		std::map<Socket*, Public::Base::shared_ptr<DoingAsyncInfo> >::iterator diter = doingList.find(iter->first);
		if (diter != doingList.end())
		{
			doasyncinfo = diter->second;
		}
		else
		{
			doasyncinfo = new DoingAsyncInfo();
			doasyncinfo->asyncInfo = iter->second;
			doingList[iter->first] = doasyncinfo;
		}
		_rebuildDoingEvent(sock,iter->second, doasyncinfo);

		return true;
	}
	bool buildDoingEvent()
	{
		Guard locker(mutex);

		std::map<Socket*, Public::Base::shared_ptr<DoingAsyncInfo> >		doingtmp = doingList;
		for (std::map<Socket*, Public::Base::shared_ptr<AsyncInfo> >::iterator iter = socketList.begin(); iter != socketList.end(); iter++)
		{
			Public::Base::shared_ptr<Socket> sock = iter->second->sock.lock();
			if (sock == NULL) continue;

			Public::Base::shared_ptr<DoingAsyncInfo> doasyncinfo;
			std::map<Socket*, Public::Base::shared_ptr<DoingAsyncInfo> >::iterator diter = doingList.find(iter->first);
			if (diter != doingList.end())
			{
				doasyncinfo = diter->second;
				doingtmp.erase(iter->first);
			}
			else
			{
				doasyncinfo = new DoingAsyncInfo();
				doasyncinfo->asyncInfo = iter->second;
				doingList[iter->first] = doasyncinfo;
			}
			_rebuildDoingEvent(sock, iter->second, doasyncinfo);
		}
		for (std::map<Socket*, Public::Base::shared_ptr<DoingAsyncInfo> >::iterator iter = doingtmp.begin(); iter != doingtmp.end(); iter++)
		{
			doingList.erase(iter->first);
		}

		return true;
	}
	virtual bool addConnectEvent(const Public::Base::shared_ptr<Socket>& sock, const Public::Base::shared_ptr<ConnectEvent>& event)
	{
		Guard locker(mutex);

		std::map<Socket*, Public::Base::shared_ptr<AsyncInfo> >::iterator iter = socketList.find(sock.get());
		if (iter == socketList.end())
		{
			return false;
		}
		Public::Base::shared_ptr<AsyncInfo> asyncinfo = iter->second;
		event->asyncobj = shared_from_this();
		asyncinfo->connectList.clear();
		asyncinfo->connectList.push_back(event);

		return true;
	}
	virtual bool addAcceptEvent(const Public::Base::shared_ptr<Socket>& sock, const Public::Base::shared_ptr<AcceptEvent>& event)
	{
		Guard locker(mutex);

		std::map<Socket*, Public::Base::shared_ptr<AsyncInfo> >::iterator iter = socketList.find(sock.get());
		if (iter == socketList.end())
		{
			return false;
		}
		Public::Base::shared_ptr<AsyncInfo> asyncinfo = iter->second;
		event->asyncobj = shared_from_this();
		asyncinfo->acceptList.clear();
		if (event != NULL)
		{
			asyncinfo->acceptList.push_back(event);
		}
		
		return true;
	}
	virtual bool addRecvEvent(const Public::Base::shared_ptr<Socket>& sock, const Public::Base::shared_ptr<RecvEvent>& event)
	{
		Guard locker(mutex);

		std::map<Socket*, Public::Base::shared_ptr<AsyncInfo> >::iterator iter = socketList.find(sock.get());
		if (iter == socketList.end())
		{
			return false;
		}
		Public::Base::shared_ptr<AsyncInfo> asyncinfo = iter->second;
		event->asyncobj = shared_from_this();
		asyncinfo->recvList.push_back(event);


		return true;
	}
	virtual bool addSendEvent(const Public::Base::shared_ptr<Socket>& sock,const Public::Base::shared_ptr<SendEvent>& event)
	{
		Guard locker(mutex);

		std::map<Socket*, Public::Base::shared_ptr<AsyncInfo> >::iterator iter = socketList.find(sock.get());
		if (iter == socketList.end())
		{
			return false;
		}
		Public::Base::shared_ptr<AsyncInfo> asyncinfo = iter->second;
		event->asyncobj = shared_from_this();
		asyncinfo->sendList.push_back(event);

		return true;
	}	
	virtual bool addDisconnectEvent(const Public::Base::shared_ptr<Socket>& sock, const Public::Base::shared_ptr<DisconnectEvent>& event)
	{
		Guard locker(mutex);

		std::map<Socket*, Public::Base::shared_ptr<AsyncInfo> >::iterator iter = socketList.find(sock.get());
		if (iter == socketList.end())
		{
			return false;
		}
		Public::Base::shared_ptr<AsyncInfo> asyncinfo = iter->second;
		asyncinfo->disconnectList.clear();
		event->asyncobj = shared_from_this();
		if (event != NULL)
		{
			asyncinfo->disconnectList.push_back(event);
		}
		
		return true;
	}
protected:	
	Mutex														mutex;
	std::map<Socket*, Public::Base::shared_ptr<AsyncInfo> >		socketList;
	std::map<Socket*, std::set<uint32_t> >						sockUsingList;
protected:	
	std::map<Socket*, Public::Base::shared_ptr<DoingAsyncInfo> >	doingList;
public:
	Public::Base::weak_ptr<AsyncManager>						manager;
	AsyncSuportType												type;
};

inline bool lockSocketUsing(const Public::Base::weak_ptr<AsyncObject>& obj, const Public::Base::shared_ptr<Socket>& sock)
{
	Public::Base::shared_ptr<AsyncObject> asyncobj = obj.lock();
	if (asyncobj != NULL && sock != -1)
	{
		return asyncobj->lockSocketUsing(sock);
	}
	return false;
}
inline void unlockSocketUsing(const Public::Base::weak_ptr<AsyncObject>& obj, const Public::Base::shared_ptr<Socket>& sock)
{
	Public::Base::shared_ptr<AsyncObject> asyncobj = obj.lock();
	if (asyncobj != NULL)
	{
		asyncobj->unlockSocketUsing(sock);
	}
}

#endif //__IASYNCOBJECT_H__
