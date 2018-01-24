//
//  Copyright (c)1998-2012, Chongqing Xunmei Technology
//  All Rights Reserved.
//
//	Description:
//	$Id: Timer.cpp 56 2013-03-08 01:47:53Z jiangwei $
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
#include "Base/Singleton.h"
#include <set>
#include <list>

using namespace std;

namespace Xunmei{
namespace Base{

class TimerThread;

struct TimerInternal
{
	uint64_t 			runningTime;			///< 定时器执行累计时间，微秒为单位
	uint64_t 			callTime;
	uint64_t 			checkTime;
	unsigned long 		period;
	unsigned long 		timeout;
	Timer::Proc 		fun;
	unsigned long 		param;
	std::string  		name;

	int					calledId;
	bool 				called;
	bool				started;
	Mutex 				mutex;
	Semaphore*			quitsem;

	bool run();
	void reset();
	void stopandwait();
	TimerInternal();
	~TimerInternal();
};

/// \class TimerThread
/// \brief 定时器线程类，仅共CTimer
///
/// 使用使用线程池模式
class TimerThread : public Thread
{
	friend class Timer;
	friend struct TimerInternal;
	friend struct TimerManagerInternal;

private:
	TimerThread();
	~TimerThread();
	void threadProc();
	void cancelThread();
private:
	TimerInternal*	runtimer;
	Semaphore 		semaphore;
	TimerThread* 	nextPooled;
	uint64_t 		prevUsedTime;
};

/// \class TimerManager
/// \brief 定时器管理类，使用高精度系统定时其来驱动应用定时器工作
///
/// 高精度定时器每次被触发时，检查所有应用定时器的状态，决定是否调用其回调函数。
/// 这个定时器的周期也决定了应用定时器的精度。
/// - Win32使用多媒体定时器，周期为1ms
/// - pSOS使用tmdlTimer组件，周期为1ms
/// - ucLinux2.4 使用系统信号，周期为10ms
/// \see Timer

struct TimerRunInfo
{
	uint32_t	runTimes;
	uint64_t	usedTime;
};

struct TimerManagerInternal : public Thread
{
	friend struct TimerInternal;
	friend class TimerThread;
	friend class Timer;

	/// 构造函数
	TimerManagerInternal();

public:
	/// 析构函数
	~TimerManagerInternal();

	/// 打印所有定时器
	void dumpTimers();

	void uninit();
private:
	bool addTimer(TimerInternal * pTimer);
	bool removeTimer(TimerInternal * pTimer);
	bool resetTimer(TimerInternal * pTimer,uint64_t usedTime);
	void threadProc();

	TimerThread * getTimerThread();
	void putTimerThread(TimerThread * thread);
	void checkTimerThread();	
	void printTimerInfo();
private:
	std::set<TimerThread*> 					timerThreadPool;
	std::list<TimerThread*> 				timerIdleThreadList;
	std::map<TimerInternal*,TimerRunInfo> 	timerList;
	uint64_t 								curTime;
	Mutex 									timerMutex;

	uint64_t								prevPrintTime;
};

Timer::Timer(const char * pName)
{
	gTimerManager;

	internal = new TimerInternal();

	setName(pName);
}

Timer::~Timer()
{
	stopAndWait();

	delete internal;
}

bool Timer::start(const Proc& fun, uint32_t delay, uint32_t period, unsigned long param /* = 0 */, uint32_t timeout /* = 0 */)
{
	internal->fun = fun;
	internal->callTime = gTimerManager.internal->curTime + (uint64_t)delay;
	internal->period = period;
	internal->timeout = timeout;
	internal->param = param;
	internal->started = true;

	gTimerManager.internal->addTimer(internal);

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

const char * Timer::getName()
{
	return internal->name.c_str();
}

void Timer::setName(const char * pszName)
{
	internal->name = (pszName == NULL) ? ":":pszName;
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

////////////////////////////////////////////////////////////////////////////////////
// TimerThread
////////////////////////////////////////////////////////////////////////////////////
TimerThread::TimerThread()
	: Thread("[Pooled]", priorDefault),runtimer(NULL),prevUsedTime(Time::getCurrentMilliSecond())
{
}

TimerThread::~TimerThread()
{
	destroyThread();
}
void TimerThread::cancelThread()
{
	Thread::cancelThread();
	semaphore.post();
}
void TimerThread::threadProc()
{
	while(looping())
	{
		if(semaphore.pend(100) < 0 || runtimer == NULL)
		{
			continue;
		}
		uint64_t startTime = Time::getCurrentMilliSecond();
		runtimer->calledId = Thread::getCurrentThreadID();
		if(!runtimer->fun.empty())
		{
			runtimer->fun(runtimer->param);
		}
		uint64_t stopTime = Time::getCurrentMilliSecond();
		gTimerManager.internal->resetTimer(runtimer,stopTime >= startTime ? stopTime - startTime : 0);
		
		runtimer = NULL;
		gTimerManager.internal->putTimerThread(this);

		prevUsedTime = Time::getCurrentMilliSecond();
	}
}

////////////////////////////////////////////////////////////////////////////////
// TimerManager
////////////////////////////////////////////////////////////////////////////////
TimerManager* TimerManager::instance()
{
	return CreateSingleton<TimerManager>(SINGLETON_Levl_Base,2);
}

TimerManager::TimerManager()
{
	internal = new TimerManagerInternal();
}

TimerManager::~TimerManager()
{
	internal->uninit();
	SAFE_DELETE(internal);
}

void TimerManager::dumpTimers()
{
	internal->dumpTimers();
}

void TimerManager::uninit()
{
	internal->uninit();
}

////////////////////////////////////////////////////////////////////////////////
TimerManagerInternal::TimerManagerInternal()
: Thread("TimerManager", Thread::priorTop),prevPrintTime(0)
{
	curTime = Time::getCurrentMilliSecond();

	createThread();
}

TimerManagerInternal::~TimerManagerInternal()
{
}

bool TimerManagerInternal::addTimer(TimerInternal * pTimer)
{
	Guard locker(timerMutex);

	std::map<TimerInternal*,TimerRunInfo>::iterator iter = timerList.find(pTimer);
	if(iter == timerList.end())
	{
		TimerRunInfo info = {0,0};
		timerList[pTimer] = info;
	}
	pTimer->called = false;

	return true;
}

bool TimerManagerInternal::removeTimer(TimerInternal * pTimer)
{
	Guard locker(timerMutex);
	timerList.erase(pTimer);	

	return true;
}

bool TimerManagerInternal::resetTimer(TimerInternal * pTimer,uint64_t usedTime)
{
	Guard locker(timerMutex);

	std::map<TimerInternal*,TimerRunInfo>::iterator iter = timerList.find(pTimer);
	if(iter != timerList.end())
	{
		iter->first->reset();
		if(iter->first->period == 0)
		{
			timerList.erase(iter);
			return false;
		}

		iter->second.runTimes ++;
		iter->second.usedTime += usedTime;
	}

	return true;
}

void TimerManagerInternal::printTimerInfo()
{
#define PRINTTIMERTIMES		5*60*1000
	uint64_t currTime = Time::getCurrentMilliSecond();
	if(currTime >= prevPrintTime && currTime - prevPrintTime < PRINTTIMERTIMES)
	{
		return;
	}

	prevPrintTime = currTime;

	logtrace("Base Timer: ----- print Timer Run Info Start--------");
	logtrace("Base Timer: timerThread:%d iledThread:%d",timerThreadPool.size(),timerIdleThreadList.size());
	uint32_t threadnum = 0;
	for(std::map<TimerInternal*,TimerRunInfo>::iterator iter = timerList.begin();iter != timerList.end();iter ++,threadnum++)
	{
		logtrace("Base Timer:	%d/%d name:%s period:%d poolCount:%d usedTime:%llums pooltime:%dms",threadnum+1,timerList.size(),
			iter->first->name.c_str(),iter->first->period,iter->second.runTimes,iter->second.usedTime,PRINTTIMERTIMES);
		iter->second.usedTime = iter->second.runTimes = 0;
	}
	logtrace("Base Timer: ----- print Timer Run Info End--------");

}


TimerThread * TimerManagerInternal::getTimerThread()
{
	if(timerIdleThreadList.size() > 0)
	{
		TimerThread* thread = timerIdleThreadList.front();
		timerIdleThreadList.pop_front();

		return thread;
	}

	TimerThread * p = new TimerThread();
	if(!p->createThread())
	{
		delete p;
		return NULL;
	}

	timerThreadPool.insert(p);

	return p;
}

void TimerManagerInternal::putTimerThread(TimerThread *thread)
{
	Guard locker(timerMutex);
	timerIdleThreadList.push_front(thread);
}

void TimerManagerInternal::checkTimerThread()
{
#define MAXTIMERTHREADTIMEOUT	60000

	uint64_t nowtime = Time::getCurrentMilliSecond();

	std::list<TimerThread*>			needFreeThread;
	
	{
		Guard locker(timerMutex);

		std::list<TimerThread*>::iterator iter;
		std::list<TimerThread*>::iterator iter1;
		for(iter = timerIdleThreadList.begin();iter != timerIdleThreadList.end();iter = iter1)
		{
			iter1 = iter;
			iter1 ++;
			if(nowtime - (*iter)->prevUsedTime >= MAXTIMERTHREADTIMEOUT || (*iter)->prevUsedTime > nowtime)
			{
				needFreeThread.push_back(*iter);
				timerThreadPool.erase(*iter);
				(*iter)->cancelThread();
				timerIdleThreadList.erase(iter);
			}
		}
	}	
	std::list<TimerThread*>::iterator iter;
	for(iter = needFreeThread.begin(); iter != needFreeThread.end();iter ++)
	{
		(*iter)->destroyThread();
		delete *iter;
	}
}
void TimerManagerInternal::dumpTimers()
{
	Guard locker(timerMutex);

	infof("Timers: ( %llu Milli-Seconds Elapsed )\n", curTime);
	infof("            Name            NextTime RunningTime Period   Used/Timeout State\n");
	infof("____________________________________________________________________________\n");

	for(std::map<TimerInternal*,TimerRunInfo>::iterator iter = timerList.begin();iter != timerList.end();iter ++)
	{
		TimerInternal* p = iter->first;
		if(p == NULL)
		{
			continue;
		}
		infof("%24s %11llu %11llu %6d %6d/%6d %s\n",
			p->name.c_str(),
			p->callTime,
			p->runningTime/ 1000,
			p->period,
			p->called ? (int)(curTime - p->checkTime) : 0,
			p->timeout,
			p->called ? "Running" : "Idle");
	}
	infof("\n");
}

void TimerManagerInternal::threadProc()
{
	int checkRunTimes = 0;
	do
	{
		setTimeout(10000); // 设置超时时间为10秒钟，超时看门狗会重启
		Thread::sleep(10);

		if(checkRunTimes ++ >= 1000)
		{
			printTimerInfo();
			checkTimerThread();
			checkRunTimes = 0;
		}		

		Guard locker(timerMutex);

		uint64_t OldTime = curTime;
		curTime = Time::getCurrentMilliSecond();

		// 计时没有改变，可能是因为计时器精度不高
		if(curTime == OldTime)
		{
			continue;
		}

		if(curTime < OldTime)
		{
		//	assert(0); // overflowd
		}

		for(std::map<TimerInternal*,TimerRunInfo>::iterator iter = timerList.begin();iter != timerList.end();iter ++)
		{
			iter->first->run();
		}
	} while(looping());
}

void TimerManagerInternal::uninit()
{
	destroyThread();

	std::set<TimerThread*> freeTimerThreadList;
	{
		Guard locker(timerMutex);
		for(std::map<TimerInternal*,TimerRunInfo>::iterator iter = timerList.begin();iter != timerList.end();iter ++)
		{
			iter->first->reset();
		}
		freeTimerThreadList = timerThreadPool;
		
		timerList.clear();
		timerThreadPool.clear();
	}
	std::set<TimerThread*>::iterator titer;
	for(titer = freeTimerThreadList.begin();titer != freeTimerThreadList.end();titer ++)
	{
		delete *titer;
	}
}

bool TimerInternal::run()
{
	uint64_t curTime = gTimerManager.internal->curTime;

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
	
	TimerThread* thread = gTimerManager.internal->getTimerThread();
	if(thread != NULL)
	{
		thread->runtimer = this;
		thread->semaphore.post();
	}
	else
	{
		Guard locker(mutex);
		called = false;
	}

	return true;
}

void TimerInternal::reset()
{
	Guard locker(mutex);
	callTime = gTimerManager.internal->curTime + period;
	runningTime = gTimerManager.internal->curTime - checkTime;
	called = false;
	calledId = 0;
	if(quitsem != NULL)
	{
		quitsem->post();
	}
}
TimerInternal::TimerInternal()
{
	runningTime = 0;
	callTime = 0;
	checkTime = 0;
	period = 0;
	timeout = 0;
	param = 0;
	called = false;
	started = false;
	calledId = 0;
	quitsem = NULL;
}
TimerInternal::~TimerInternal()
{
	stopandwait();
}

void TimerInternal::stopandwait()
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
	gTimerManager.internal->removeTimer(this);
	called = false;
	SAFE_DELETE(quitsem);
}

} // namespace Base
} // namespace Xunmei

