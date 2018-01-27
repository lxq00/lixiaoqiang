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

typedef enum
{
	EventType_Read,
	EventType_Write,
	EventType_Error,
}EventType;

class IAsyncEvent
{
public:
	IAsyncEvent() {}
	virtual ~IAsyncEvent() {}

	virtual bool doResultEvent(int len) = 0;
	virtual int sock() = 0;
	virtual EventType type() = 0;
};

class IAsyncCanEvent :public IAsyncEvent
{
public:
	IAsyncCanEvent() {}
	virtual ~IAsyncCanEvent() {}

	virtual bool doCanEvent() = 0;
};
class IAsyncManager;
class IEventManager
{
public:
	IEventManager() {}
	virtual ~IEventManager() {}

	virtual shared_ptr<IAsyncEvent> findEvent(IAsyncEvent* eventptr) = 0;
	virtual shared_ptr<IAsyncCanEvent> findCanEvent(int sock, EventType type) = 0;
	virtual bool cleanEvent(const shared_ptr<IAsyncEvent>& event) = 0;
	virtual bool getAsyncEventList(IAsyncManager* async, std::list<int>& readlist, std::list<int>& writelist, std::list<int>& errorlist) = 0;
};

class IAsyncManager
{
public:
	IAsyncManager(const weak_ptr<IEventManager>& _manager):manager(_manager) {}
	virtual ~IAsyncManager() {}

	virtual shared_ptr<IAsyncEvent> addConnectEvent(int sock, const NetAddr& othreaddr,const Socket::ConnectedCallback& callback) = 0;
	virtual shared_ptr<IAsyncEvent> addAcceptEvent(int sock,const Socket::AcceptedCallback& callback) = 0;
	virtual shared_ptr<IAsyncEvent> addRecvFromEvent(int sock, char* buffer, int bufferlen,const Socket::RecvFromCallback& callback) = 0;
	virtual shared_ptr<IAsyncEvent> addRecvEvent(int sock, char* buffer, int bufferlen, const Socket::ReceivedCallback& callback) = 0;
	virtual shared_ptr<IAsyncEvent> addSendToEvent(int sock, const char* buffer, int bufferltn, const NetAddr& otheraddr, const Socket::SendedCallback& callback) = 0;
	virtual shared_ptr<IAsyncEvent> addSendEvent(int sock, const char* buffer, int bufferlen, const Socket::SendedCallback& callback) = 0;
	virtual shared_ptr<IAsyncEvent>	addEmptyEvent(int sock, const Socket::DisconnectedCallback& callback) = 0;
protected:
	weak_ptr<IEventManager> manager;
};

class IAsyncCanObject
{
public:
	IAsyncCanObject(const weak_ptr<IEventManager>& _manager):manager(_manager) {}
	virtual ~IAsyncCanObject() {}

	virtual bool addErrorEvent(int sock) = 0;
	virtual bool addReadEvent(int sock) = 0;
	virtual bool addWriteEvent(int sock) = 0;
protected:
	weak_ptr<IEventManager> manager;
};



#endif //__IASYNCEVENT_H__
