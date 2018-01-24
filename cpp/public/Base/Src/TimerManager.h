#ifndef __TIMERMANAGER_H__
#define __TIMERMANAGER_H__
#include "Base/IntTypes.h"
#include "Base/Thread.h"
#include "Base/Time.h"
#include "Base/Timer.h"
#include "Base/Guard.h"
#include "Base/Semaphore.h"
namespace Public {
namespace Base {

class TimerManager;
struct Timer::TimerInternal
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
	shared_ptr<TimerManager> manager;
public:
	bool run();
	void reset();
	void stopandwait();
	TimerInternal()
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
	~TimerInternal()
	{
		stopandwait();
	}
};
class TimerThread;
class ITimerThreadManager
{
public:
	ITimerThreadManager() {}
	virtual ~ITimerThreadManager() {}
	virtual bool resetTimer(Timer::TimerInternal * pTimer, uint64_t usedTime) = 0;
	virtual void putTimerThread(TimerThread *thread) = 0;
};
/// 使用使用线程池模式
class TimerThread : public Thread
{
public:
	TimerThread(ITimerThreadManager* _manager) :Thread("[Pooled]", priorDefault),manager(_manager) ,runtimer(NULL), prevUsedTime(Time::getCurrentMilliSecond())
	{
	}
	~TimerThread()
	{
		Thread::cancelThread();
		semaphore.post();
		destroyThread();
	}
	bool runTimer(Timer::TimerInternal* pTimer)
	{
		runtimer = pTimer;
		semaphore.post();

		return true;
	}
private:
	void threadProc()
	{
		while (looping())
		{
			if (semaphore.pend(100) < 0 || runtimer == NULL)
			{
				continue;
			}
			uint64_t startTime = Time::getCurrentMilliSecond();
			runtimer->calledId = Thread::getCurrentThreadID();
			if (!runtimer->fun.empty())
			{
				runtimer->fun(runtimer->param);
			}
			uint64_t stopTime = Time::getCurrentMilliSecond();
			manager->resetTimer(runtimer, stopTime >= startTime ? stopTime - startTime : 0);

			runtimer = NULL;
			manager->putTimerThread(this);

			prevUsedTime = Time::getCurrentMilliSecond();
		}
	}
private:
	ITimerThreadManager *	manager;
	Timer::TimerInternal *	runtimer;
	Semaphore 				semaphore;
public:
	uint64_t 				prevUsedTime;
};


class TimerManager : public Thread,public ITimerThreadManager
{
	struct TimerRunInfo
	{
		uint32_t	runTimes;
		uint64_t	usedTime;
	};
public:
	TimerManager(): Thread("TimerManager", Thread::priorTop), prevPrintTime(0)
	{
		curTime = Time::getCurrentMilliSecond();

		createThread();
	}
	/// 析构函数
	~TimerManager()
	{
		cancelThread();
		{
			std::set<TimerThread*> freeTimerThreadList;
			{
				Guard locker(timerMutex);
				for (std::map<Timer::TimerInternal*, TimerRunInfo>::iterator iter = timerList.begin(); iter != timerList.end(); iter++)
				{
					iter->first->reset();
				}
				freeTimerThreadList = timerThreadPool;

				timerList.clear();
				timerThreadPool.clear();
			}
			std::set<TimerThread*>::iterator titer;
			for (titer = freeTimerThreadList.begin(); titer != freeTimerThreadList.end(); titer++)
			{
				delete *titer;
			}
		}
		destroyThread();
	}
	bool addTimer(Timer::TimerInternal * pTimer)
	{
		Guard locker(timerMutex);

		std::map<Timer::TimerInternal*, TimerRunInfo>::iterator iter = timerList.find(pTimer);
		if (iter == timerList.end())
		{
			TimerRunInfo info = { 0,0 };
			timerList[pTimer] = info;
		}
		pTimer->called = false;

		return true;
	}
	bool removeTimer(Timer::TimerInternal * pTimer)
	{
		Guard locker(timerMutex);
		timerList.erase(pTimer);

		return true;
	}
	bool runTimer(Timer::TimerInternal* pTimer)
	{
		TimerThread* thread = getTimerThread();
		if (thread != NULL)
		{
			thread->runTimer(pTimer);
		}

		return thread != NULL;
	}
private:
	bool resetTimer(Timer::TimerInternal * pTimer, uint64_t usedTime)
	{
		Guard locker(timerMutex);

		std::map<Timer::TimerInternal*, TimerRunInfo>::iterator iter = timerList.find(pTimer);
		if (iter != timerList.end())
		{
			iter->first->reset();
			if (iter->first->period == 0)
			{
				timerList.erase(iter);
				return false;
			}

			iter->second.runTimes++;
			iter->second.usedTime += usedTime;
		}

		return true;
	}
	void threadProc()
	{
		int checkRunTimes = 0;
		do
		{
			setTimeout(10000); // 设置超时时间为10秒钟，超时看门狗会重启
			Thread::sleep(10);

			if (checkRunTimes++ >= 1000)
			{
				checkTimerThread();
				checkRunTimes = 0;
			}

			Guard locker(timerMutex);

			uint64_t OldTime = curTime;
			curTime = Time::getCurrentMilliSecond();

			// 计时没有改变，可能是因为计时器精度不高
			if (curTime == OldTime)
			{
				continue;
			}

			if (curTime < OldTime)
			{
				//	assert(0); // overflowd
			}

			for (std::map<Timer::TimerInternal*, TimerRunInfo>::iterator iter = timerList.begin(); iter != timerList.end(); iter++)
			{
				iter->first->run();
			}
		} while (looping());
	}

	TimerThread * getTimerThread()
	{
		if (timerIdleThreadList.size() > 0)
		{
			TimerThread* thread = timerIdleThreadList.front();
			timerIdleThreadList.pop_front();

			return thread;
		}

		TimerThread * p = new TimerThread(this);
		if (!p->createThread())
		{
			delete p;
			return NULL;
		}

		timerThreadPool.insert(p);

		return p;
	}
	void putTimerThread(TimerThread * thread)
	{
		Guard locker(timerMutex);
		timerIdleThreadList.push_front(thread);
	}
	void checkTimerThread()
	{
#define MAXTIMERTHREADTIMEOUT	60000

		uint64_t nowtime = Time::getCurrentMilliSecond();

		std::list<TimerThread*>			needFreeThread;

		{
			Guard locker(timerMutex);

			std::list<TimerThread*>::iterator iter;
			std::list<TimerThread*>::iterator iter1;
			for (iter = timerIdleThreadList.begin(); iter != timerIdleThreadList.end(); iter = iter1)
			{
				iter1 = iter;
				iter1++;
				if (nowtime - (*iter)->prevUsedTime >= MAXTIMERTHREADTIMEOUT || (*iter)->prevUsedTime > nowtime)
				{
					needFreeThread.push_back(*iter);
					timerThreadPool.erase(*iter);
					(*iter)->cancelThread();
					timerIdleThreadList.erase(iter);
				}
			}
		}
		std::list<TimerThread*>::iterator iter;
		for (iter = needFreeThread.begin(); iter != needFreeThread.end(); iter++)
		{
			(*iter)->destroyThread();
			delete *iter;
		}
	}
private:
	std::set<TimerThread*> 							timerThreadPool;
	std::list<TimerThread*> 						timerIdleThreadList;
	std::map<Timer::TimerInternal*, TimerRunInfo> 	timerList;
	Mutex 											timerMutex;

	uint64_t										prevPrintTime;

public:
	uint64_t 										curTime;
};

} // namespace Base
} // namespace Public


#endif //__TIMERMANAGER_H__
