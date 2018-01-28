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
	EventType_Read,
	EventType_Write,
	EventType_Error,
}EventType;



class IAsyncManager
{
public:
	IAsyncManager(){}
	virtual ~IAsyncManager() {}

	virtual shared_ptr<IAsyncEvent> addConnectEvent(const shared_ptr<Socket>& sock, const NetAddr& othreaddr,const Socket::ConnectedCallback& callback) = 0;
	virtual shared_ptr<IAsyncEvent> addAcceptEvent(const shared_ptr<Socket>& sock,const Socket::AcceptedCallback& callback) = 0;
	virtual shared_ptr<IAsyncEvent> addRecvFromEvent(const shared_ptr<Socket>& sock, char* buffer, int bufferlen,const Socket::RecvFromCallback& callback) = 0;
	virtual shared_ptr<IAsyncEvent> addRecvEvent(const shared_ptr<Socket>& sock, char* buffer, int bufferlen, const Socket::ReceivedCallback& callback) = 0;
	virtual shared_ptr<IAsyncEvent> addSendToEvent(const shared_ptr<Socket>& sock, const char* buffer, int bufferltn, const NetAddr& otheraddr, const Socket::SendedCallback& callback) = 0;
	virtual shared_ptr<IAsyncEvent> addSendEvent(const shared_ptr<Socket>& sock, const char* buffer, int bufferlen, const Socket::SendedCallback& callback) = 0;
	virtual shared_ptr<IAsyncEvent>	addEmptyEvent(const shared_ptr<Socket>& sock, const Socket::DisconnectedCallback& callback) = 0;
};




#endif //__IASYNCEVENT_H__
