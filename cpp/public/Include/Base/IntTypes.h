//
//  Copyright (c)1998-2014, Public Technology
//  All Rights Reserved.
//
//	Description:
//	$Id: IntTypes.h 3 2013-01-21 06:57:38Z jiangwei $
//


#ifndef __BASE_INT_TYPES_H__
#define __BASE_INT_TYPES_H__

#ifndef WIN32
#   include <inttypes.h>
#else

#if _MSC_VER > 1500 //VGS2008
#   include <stdint.h>
#else
#ifndef int8_t
typedef signed __int8				int8_t;
#endif

#ifndef int16_t
typedef signed __int16             int16_t;
#endif

#ifndef int32_t
typedef signed __int32             int32_t;
#endif

#ifndef int64_t
typedef signed __int64			int64_t;
#endif

#ifndef uint8_t
typedef unsigned __int8     uint8_t;
#endif

#ifndef uint16_t
typedef unsigned __int16    uint16_t;
#endif

#ifndef uint32_t
typedef unsigned __int32	uint32_t;
#endif

#ifndef uint64_t
typedef unsigned __int64	uint64_t;
#endif

#endif

#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <vector>
#include <string>
#include <map>
#include <set>
#include <list>

using namespace std;

namespace Public{
namespace Base{

#include "Base/Defs.h"

//加锁释放锁接口
class BASE_API ILockerObjcPtr
{
public:
	ILockerObjcPtr(){}
	virtual ~ILockerObjcPtr(){}

	virtual bool enter() {return false;}
	virtual bool leave() {return false;}
};

//内存释放分配接口
class BASE_API IMallcFreeMemObjcPtr
{
public:
	IMallcFreeMemObjcPtr(){}
	virtual ~IMallcFreeMemObjcPtr(){}

	virtual void* Malloc(uint32_t size,uint32_t& realsize) {return NULL;}
	virtual void Free(void* addr) {}
};

//信号量接口
class BASE_API ISemaphoreObjcPtr
{
public:
	ISemaphoreObjcPtr(){}
	virtual ~ISemaphoreObjcPtr(){}

	virtual int pend() {return -1;}
	virtual int pend(uint32_t timeout) {return -1;}
	virtual int post() {return -1;}
};

#ifdef WIN32
#define BASE_Assert(x){\
	if(!x){\
		int errorno = GetLastError();\
		(void)errorno;\
		assert(x);\
	}\
}
#else
#include<errno.h>
#define BASE_Assert(x) {\
	if(!x){\
	int errorno = errno;\
	(void)errorno;\
	assert(x);\
	}\
}
#endif

}
}





#endif// __BASE_INT_TYPES_H__

