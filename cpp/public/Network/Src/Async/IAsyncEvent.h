#ifndef __IASYNCEVENT_H__
#define __IASYNCEVENT_H__
#include "Base/Base.h"
using namespace Public::Base;

class IAsyncEvent
{
public:
	IAsyncEvent() {}
	virtual ~IAsyncEvent() {}

	virtual bool doResultEvent(int len) = 0
};

class IAsyncCanEvent :public IAsyncEvent
{
public:
	IAsyncCanEvent() {}
	virtual ~IAsyncCanEvent() {}

	virtual bool doCanEvent() = 0;
};

class IEventManager
{
public:
	typedef enum
	{
		EventType_Read,
		EventType_Write,
		EventType_Error,
	}EventType;
public:
	IEventManager() {}
	virtual ~IEventManager() {}

	virtual shared_ptr<IAsyncEvent> find
};



#endif //__IASYNCEVENT_H__
