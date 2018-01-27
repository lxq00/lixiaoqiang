//
//  Copyright (c)1998-2012,  Public Technology
//  All Rights Reserved.
//
//	Description:
//	$Id: ReadWriteMutex.cpp 11 2013-01-22 08:42:03Z  $
//

#include <assert.h>
#include "Base/ReadWriteMutex.h"
#include "Base/PrintLog.h"

#ifdef WIN32
	#include <Windows.h>
	#include <stdio.h>
#elif defined(__linux__)
	#include <stdlib.h>
	#include <pthread.h>
	 #include <stdio.h>
#endif

#ifdef __linux

#define RWMUXTEX_MAXENTER	0xffff

#endif  /* __linux*/


namespace Public{
namespace Base{


struct ReadWriteMutex::ReadWriteMutexInternal
{
#ifdef WIN32
	LONG dwreader;
	int iswritemutex;
	CRITICAL_SECTION enterlock;
#elif defined(__linux__)
	pthread_rwlock_t * lock;
#endif
};


ReadWriteMutex::ReadWriteMutex()
{
	internal = new ReadWriteMutexInternal;
	assert(internal);

#ifdef __linux
	internal->lock = (pthread_rwlock_t *)malloc(sizeof(pthread_rwlock_t));
	BASE_Assert(internal->lock);
	int ret = pthread_rwlock_init(internal->lock,NULL);
	BASE_Assert(ret == 0);
	(void)ret;	// 消除变量未使用编译警告

#elif defined(WIN32)
	InitializeCriticalSection(&internal->enterlock);
	internal->dwreader = 0;
	internal->iswritemutex = 0;
#endif
}

ReadWriteMutex::~ReadWriteMutex()
{
#ifdef __linux
	int ret = pthread_rwlock_destroy(internal->lock);
	BASE_Assert(ret == 0);
	free(internal->lock);
	(void)ret;	// 消除变量未使用编译警告

#elif defined(WIN32)
	if (internal->dwreader > 0)
		logerror("has lock Read count(%d), maybe err\n",internal->dwreader);
	if (internal->iswritemutex == 1) {
		logerror("has lock write, maybe err\n");
		LeaveCriticalSection(&internal->enterlock);
	}
	DeleteCriticalSection(&internal->enterlock);

#endif

	delete internal;

}

 bool ReadWriteMutex::enterread()
{
#ifdef __linux
	int ret = pthread_rwlock_rdlock(internal->lock);
	if (ret != 0)
		return false;
	return true;
#elif defined(WIN32)
	EnterCriticalSection(&internal->enterlock);
	InterlockedIncrement(&internal->dwreader);
	LeaveCriticalSection(&internal->enterlock);
	return true;
#endif
	return false;

}

bool ReadWriteMutex::enterwrite()
{
#ifdef __linux
	int ret = pthread_rwlock_wrlock(internal->lock);
	if (ret != 0)
		return false;
	return true;

#elif defined(WIN32)
	EnterCriticalSection(&internal->enterlock);
	while(internal->dwreader > 0)
	{
		Sleep(0);
	}
	internal->iswritemutex = 1;
	return true;
#endif
	return false;
}
bool ReadWriteMutex::leave()
{
#ifdef __linux
	int ret = pthread_rwlock_unlock(internal->lock);
	if (ret != 0)
		return false;
	return true;

#elif defined(WIN32)
	if (internal->dwreader > 0)
		InterlockedDecrement(&internal->dwreader);
	else if (internal->iswritemutex == 1) {
		internal->iswritemutex = 0;
		LeaveCriticalSection(&internal->enterlock);
	} else {
		logerror("No lock, maybe err\n");
		return false;
	}
	return true;
#endif

	return false;
}

} // namespace Base
} // namespace Public



