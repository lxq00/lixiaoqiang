//
//  Copyright (c)1998-2012, Chongqing Xunmei Technology
//  All Rights Reserved.
//
//	Description:
//	$Id: RecursiveMutex.cpp 11 2013-01-22 08:42:03Z jiangwei $
//

#include <assert.h>
#include "Base/RecursiveMutex.h"


////////////////////////////////////////////////////////////////////////////////

#ifdef WIN32

#include <Windows.h>


namespace {

#ifdef _MSC_VER

#  if _MSC_VER >= 1300
	extern "C" void _ReadWriteBarrier(void);
	#pragma intrinsic(_ReadWriteBarrier)


	inline long interlocked_read_acquire(long volatile* x)
	{
		long const res = *x;
		_ReadWriteBarrier();
		return res;
	}

#  else // _MSC_VER < 1300

	inline long interlocked_read_acquire(long volatile* x)
	{
		return (long)InterlockedCompareExchange((PVOID*)x, 0, 0);
	}
	
#  endif

#else

	inline long interlocked_read_acquire(long volatile* x)
	{
		return InterlockedCompareExchange(x, 0, 0);
	}

#endif

} // namespace noname

namespace Xunmei{
namespace Base {

struct RecursiveMutex::RecursiveMutexInternal
{
    CRITICAL_SECTION mtx;
	long recursion_count;
	long locking_tid;
};

RecursiveMutex::RecursiveMutex()
{
	internal = new RecursiveMutexInternal;
	memset(internal, 0, sizeof(RecursiveMutexInternal));
	InitializeCriticalSection(&internal->mtx);
}

RecursiveMutex::~RecursiveMutex()
{
	DeleteCriticalSection(&internal->mtx);
	delete internal;
}

bool RecursiveMutex::enter()
{
	long const current_tid = GetCurrentThreadId();
	if (interlocked_read_acquire(&internal->locking_tid) == current_tid)
	{
		++internal->recursion_count;
	}
	else
	{
		EnterCriticalSection(&internal->mtx);
		InterlockedExchange(&internal->locking_tid, current_tid);
		internal->recursion_count = 1;
	}

	return true;
}

bool RecursiveMutex::leave()
{
	if (!--internal->recursion_count)
	{
		InterlockedExchange(&internal->locking_tid, 0);
		LeaveCriticalSection(&internal->mtx);
	}

	return true;
}

} // namespace Base
} // namespace Xunmei

////////////////////////////////////////////////////////////////////////////////

#elif defined(__linux__) || defined(_POSIX_THREADS)

#include <stdlib.h>
#include <pthread.h>
/// pthread_mutexattr_settype bad
extern "C"	int pthread_mutexattr_settype (pthread_mutexattr_t *__attr, int __kind);

namespace Xunmei{
namespace Base {

struct RecursiveMutex::RecursiveMutexInternal
{
	pthread_mutex_t	mtx;
};

RecursiveMutex::RecursiveMutex()
{
	internal = new RecursiveMutexInternal;

	pthread_mutexattr_t attr;
	int ret = pthread_mutexattr_init(&attr);
	BASE_Assert(ret == 0);

	ret = pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE_NP);
	BASE_Assert(ret == 0);

	ret = pthread_mutex_init(&internal->mtx, &attr);
	BASE_Assert(ret == 0);
	(void)ret;	// 消除变量未使用编译警告
	
}

RecursiveMutex::~RecursiveMutex()
{
	int ret = pthread_mutex_destroy(&internal->mtx);
	BASE_Assert(ret == 0);
	(void)ret;	// 消除变量未使用编译警告

	delete internal;
}

bool RecursiveMutex::enter()
{
	return (pthread_mutex_lock(&internal->mtx) == 0);
}

bool RecursiveMutex::leave()
{
	return (pthread_mutex_unlock(&internal->mtx) == 0);
}

} // namespace Base
} // namespace Xunmei

////////////////////////////////////////////////////////////////////////////////

#else
	#error "Unknown Platform"
#endif



