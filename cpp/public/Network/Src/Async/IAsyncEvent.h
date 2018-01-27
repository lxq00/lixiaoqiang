#ifndef __IASYNCEVENT_H__
#define __IASYNCEVENT_H__
#include "Base/Base.h"
using namespace Public::Base;

class IAsyncEvent
{
public:
	IAsyncEvent() {}
	virtual ~IAsyncEvent() {}

	virtual bool doEvent() = 0
};

class IAsyncCanEvent :IAsyncEvent
{
public:
	IAsyncCanEvent() {}
	virtual ~IAsyncCanEvent() {}

	virtual bool doCanEvent() = 0;
};




#endif //__IASYNCEVENT_H__
