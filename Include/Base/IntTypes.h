//
//  Copyright (c)1998-2014, Public Technology
//  All Rights Reserved.
//
//	Description:
//	$Id: IntTypes.h 3 2013-01-21 06:57:38Z  $
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
#include <deque>
#include <sstream>

using namespace std;

namespace Public{
namespace Base{

#include "Base/Defs.h"

//加锁释放锁接口
class BASE_API IMutexInterface
{
public:
	IMutexInterface(){}
	virtual ~IMutexInterface(){}

	virtual bool enter() {return false;}
	virtual bool leave() = 0;
};

//内存释放分配接口
class BASE_API IMempoolInterface
{
public:
	IMempoolInterface(){}
	virtual ~IMempoolInterface(){}

	virtual void* Malloc(uint32_t size, uint32_t& realsize) = 0;
	virtual bool Free(void* addr) = 0;
	virtual uint32_t maxBufferSize() { return 0; }
	virtual uint32_t usedBufferSize() { return 0; }
};

//信号量接口
class BASE_API ISemaphoreInterface
{
public:
	ISemaphoreInterface(){}
	virtual ~ISemaphoreInterface(){}

	virtual int pend() = 0;
	virtual int pend(uint32_t timeout) = 0;
	virtual int post() = 0;
};

#ifdef WIN32
#define BASE_Assert(x){\
	if(!(x)){\
		int errorno = GetLastError();\
		(void)errorno;\
		assert(x);\
	}\
}
#else
#include<errno.h>
#define BASE_Assert(x) {\
	if(!(x)){\
	int errorno = errno;\
	(void)errorno;\
	assert(x);\
	}\
}
#endif

template<typename T>
const T& move(const T& tmp)
{
#ifdef STDSUPORTMOVE
	return std::move(tmp);
#else
	return tmp;
#endif 
}

}
}





#endif// __BASE_INT_TYPES_H__

