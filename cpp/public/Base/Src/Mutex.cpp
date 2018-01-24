//
//  Copyright (c)1998-2012, Chongqing Public Technology
//  All Rights Reserved.
//
//	Description:
//	$Id: Mutex.cpp 154 2013-09-02 01:59:53Z lixiaoqiang $
//

#include <assert.h>
#include "Base/Mutex.h"


////////////////////////////////////////////////////////////////////////////////

#ifdef WIN32

#include <Windows.h>

namespace Public{
namespace Base {

struct Mutex::MutexInternal
{
    ::CRITICAL_SECTION mtx;
};

Mutex::Mutex()
{
	internal = new MutexInternal;
	if(!::InitializeCriticalSectionAndSpinCount(&internal->mtx,1000))
	{
		::InitializeCriticalSection(&internal->mtx);
	}
}

Mutex::~Mutex()
{
	::DeleteCriticalSection(&internal->mtx);
	delete internal;
}

bool Mutex::enter()
{
	::EnterCriticalSection(&internal->mtx);
	return true;
}

bool Mutex::leave()
{
	::LeaveCriticalSection(&internal->mtx);
	return true;
}

} // namespace Base
} // namespace Public


////////////////////////////////////////////////////////////////////////////////

#elif defined(__linux__) || defined(_POSIX_THREADS)

#include <stdlib.h>
#include <pthread.h>
/// pthread_mutexattr_settype bad
extern "C"	int pthread_mutexattr_settype (pthread_mutexattr_t *__attr, int __kind);

namespace Public{
namespace Base {

struct Mutex::MutexInternal
{
	pthread_mutex_t	mtx;
};

Mutex::Mutex()
{
	internal = new MutexInternal;

	int ret = pthread_mutex_init(&internal->mtx, NULL);
	BASE_Assert(ret == 0);
	(void)ret;	// 消除变量未使用编译警告
}

Mutex::~Mutex()
{
	int ret = pthread_mutex_destroy(&internal->mtx);
//	assert(ret == 0);
	(void)ret;	// 消除变量未使用编译警告

	delete internal;
}

bool Mutex::enter()
{
	return (pthread_mutex_lock(&internal->mtx) == 0);
}

bool Mutex::leave()
{
	return (pthread_mutex_unlock(&internal->mtx) == 0);
}

} // namespace Base
} // namespace Public

#else
	#error "Unknown Mutex Platform"
#endif



