//
//  Copyright (c)1998-2012,  Public Technology
//  All Rights Reserved.
//
//	Description:
//	$Id: Timer.cpp 56 2013-03-08 01:47:53Z  $
//



#include <string.h>
#include <assert.h>
#include <algorithm>
#ifdef WIN32
	#include <Windows.h>
	#include <Mmsystem.h>
	#include <stdio.h>
#ifndef snprintf
#define snprintf _snprintf
#endif
	UINT     wTimerRes;
#elif defined(__linux__)
	#include <stdio.h>
	#include <errno.h>
	#include <time.h>
	#include <signal.h>
	#include <sys/time.h>
#else
	#error "Unknown Platform"
#endif

#include "Base/Time.h"
#include "Base/Timer.h"
#include "Base/PrintLog.h"
#include "Base/Thread.h"
#include "Base/Semaphore.h"
#include "Base/Guard.h"
#include "Base/BaseTemplate.h"
#include "TimerManager.h"

namespace Public{
namespace Base{

static Mutex				  timerMutex;
static weak_ptr<TimerManager> manager;


Timer::Timer(const std::string& pName)
{
	internal = new TimerInternal();

	setName(pName);

	{
		Guard locker(timerMutex);
		shared_ptr<TimerManager> timermanager = manager.lock();
		if (timermanager == NULL)
		{
			timermanager = shared_ptr<TimerManager>(new TimerManager());
			manager = timermanager;
		}

		internal->manager = timermanager;
	}
}

Timer::~Timer()
{
	stopAndWait();

	delete internal;
}

bool Timer::start(const Proc& fun, uint32_t delay, uint32_t period, unsigned long param /* = 0 */, uint32_t timeout /* = 0 */)
{
	internal->fun = fun;
	internal->callTime = internal->manager->curTime + (uint64_t)delay;
	internal->period = period;
	internal->timeout = timeout;
	internal->param = param;
	internal->started = true;

	internal->manager->addTimer(internal);

	return true;
}

bool Timer::stop(bool bCallNow /* = false */)
{
	Guard locker(internal->mutex);
	internal->started = false;

	return true;
}

bool Timer::stopAndWait()
{
	internal->stopandwait();
	
	return true;
}

std::string Timer::getName()
{
	return internal->name;
}

void Timer::setName(const std::string& pszName)
{
	internal->name = pszName;
}

bool Timer::isStarted()
{
	return internal->started;
}

bool Timer::isCalled()
{
	return internal->called;
}

bool Timer::isRunning()
{
	return internal->started;
}

bool Timer::TimerInternal::run()
{
	uint64_t curTime = manager->curTime;

	{
		Guard locker(mutex);
		
		if(called || curTime <= callTime || !started)
		{
			return false;
		}
		runningTime = 0;
		checkTime = curTime;
		called = true;
	}

	if (!manager->runTimer(this))
	{
		Guard locker(mutex);
		called = false;
	}

	return true;
}

void Timer::TimerInternal::reset()
{
	Guard locker(mutex);
	callTime = manager->curTime + period;
	runningTime = manager->curTime - checkTime;
	called = false;
	calledId = 0;
	if(quitsem != NULL)
	{
		quitsem->post();
	}
}


void Timer::TimerInternal::stopandwait()
{
	SAFE_DELETE(quitsem);
	{
		Guard locker(mutex);
		started = false;
		if(called && calledId != Thread::getCurrentThreadID())
		{
			quitsem = new Semaphore();
		}		
	}

	if(quitsem != NULL)
	{
		quitsem->pend();
	}
	manager->removeTimer(this);
	called = false;
	SAFE_DELETE(quitsem);
}

} // namespace Base
} // namespace Public

