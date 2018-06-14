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
	virtual Timer::TimerInternal * getNeedRunTimer() = 0;	
	virtual bool putTimerTimeoutAndDestory(TimerThread *thread) = 0;
};

#define TIMERTHREADTIMEOUT		60000
#define MINTHREADPOOLSIZE		2
#define MAXTHREADPOOLSIZE		10

/// 使用使用线程池模式
class TimerThread : public Thread
{
public:
	TimerThread(ITimerThreadManager* _manager) :Thread("[Pooled]", priorTop + 5),manager(_manager) ,prevUsedTime(Time::getCurrentMilliSecond())
	{
	}
	~TimerThread()
	{
		Thread::cancelThread();
		destroyThread();
	}
private:
	void threadProc()
	{
		while (looping())
		{
			Timer::TimerInternal * runtimer = manager->getNeedRunTimer();
			if (runtimer != NULL)
			{
				uint64_t startTime = Time::getCurrentMilliSecond();
				runtimer->calledId = Thread::getCurrentThreadID();
				runtimer->fun(runtimer->param);
				uint64_t stopTime = Time::getCurrentMilliSecond();
				manager->resetTimer(runtimer, stopTime >= startTime ? stopTime - startTime : 0);

				prevUsedTime = Time::getCurrentMilliSecond();
			}

			uint64_t nowtime = Time::getCurrentMilliSecond();
			if (nowtime < prevUsedTime || nowtime > prevUsedTime + TIMERTHREADTIMEOUT)
			{
				if (manager->putTimerTimeoutAndDestory(this))
				{
					break;
				}

				prevUsedTime = nowtime;
			}
		}
	}
private:
	ITimerThreadManager *	manager;
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
		destroyThread();

		timerThreadPool.clear();
		destoryTimerThreadList.clear();
		timerList.clear();
	}
	bool addTimer(Timer::TimerInternal * pTimer)
	{
		Guard locker(timerMutex);

		std::map<Timer::TimerInternal*, TimerRunInfo>::iterator iter = timerList.find(pTimer);
		if (iter == timerList.end())
		{
			TimerRunInfo info = { 0,0 };
			timerList[pTimer] = info;

			pTimer->called = false;
		}

		return true;
	}
	bool removeTimer(Timer::TimerInternal * pTimer)
	{
		while (1)
		{
			{
				Guard locker(timerMutex);
				std::set<Timer::TimerInternal*>::iterator iter = waitAndDoingTimer.find(pTimer);
				if (iter == waitAndDoingTimer.end())
				{
					timerList.erase(pTimer);
					break;
				}
			}

			Thread::sleep(10);
		}

		return true;
	}
	bool runTimer(Timer::TimerInternal* pTimer)
	{
		Guard locker(timerMutex);

		std::map<Timer::TimerInternal*, TimerRunInfo>::iterator iter = timerList.find(pTimer);
		if (iter == timerList.end())
		{
			return false;
		}
		std::set<Timer::TimerInternal*>::iterator witer = waitAndDoingTimer.find(pTimer);
		if (witer == waitAndDoingTimer.end())
		{
			needRunTimer.push_back(pTimer);
			waitAndDoingTimer.insert(pTimer);

			timerSem.post();
		}

		return true;
	}
private:
	virtual Timer::TimerInternal * getNeedRunTimer()
	{
		timerSem.pend(100);
		
		Guard locker(timerMutex);
		if (needRunTimer.size() <= 0) return NULL;

		Timer::TimerInternal * timer = needRunTimer.front();
		needRunTimer.pop_front();

		return timer;
	}



	virtual bool putTimerTimeoutAndDestory(TimerThread *thread)
	{
		Guard locker(timerMutex);
		if (timerThreadPool.size() <= MINTHREADPOOLSIZE) return false;

		std::map<TimerThread*, shared_ptr<TimerThread> >::iterator iter = timerThreadPool.find(thread);
		if (iter != timerThreadPool.end())
		{
			destoryTimerThreadList.insert(iter->second);
			timerThreadPool.erase(iter);
		}

		return true;
	}

	bool resetTimer(Timer::TimerInternal * pTimer, uint64_t usedTime)
	{
		Guard locker(timerMutex);

		waitAndDoingTimer.erase(pTimer);
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

			checkTimerThread();

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
	
	void checkTimerThread()
	{
		{
			std::set<shared_ptr<TimerThread> > freeList;
			{
				Guard locker(timerMutex);
				freeList = destoryTimerThreadList;
				destoryTimerThreadList.clear();
			}
		}


		{
			Guard locker(timerMutex);
			int newthreadsize = 0;
			if (timerThreadPool.size() < MINTHREADPOOLSIZE)
			{
				//必須保證2個
				newthreadsize = MINTHREADPOOLSIZE - timerThreadPool.size();
			}
			if (timerThreadPool.size() < MAXTHREADPOOLSIZE)
			{
				//thread 2 -> needRunTimer.size() <=5
				//thread 4 -> needRunTimer.size() 5 ~ 10
				//thread 6 -> needRunTimer.size() 10 ~ 30
				//thread 8 -> needRunTimer.size() 30 ~ 80
				//thread 10 -> needRunTimer.size() > 80

				int neednewthreadsize = 0;
				if (needRunTimer.size() > 5 && needRunTimer.size() <= 10 && timerThreadPool.size() < 4)
				{
					neednewthreadsize = 4 - timerThreadPool.size();
				}
				else if (needRunTimer.size() > 10 && needRunTimer.size() <= 30 && timerThreadPool.size() < 6)
				{
					neednewthreadsize = 6 - timerThreadPool.size();
				}
				else if (needRunTimer.size() > 30 && needRunTimer.size() <= 80 && timerThreadPool.size() < 8)
				{
					neednewthreadsize = 8 - timerThreadPool.size();
				}
				else if(needRunTimer.size() > 80)
				{
					neednewthreadsize = MAXTHREADPOOLSIZE - timerThreadPool.size();
				}

				newthreadsize = max(newthreadsize, neednewthreadsize);
			}
			if (newthreadsize + timerThreadPool.size() > MAXTHREADPOOLSIZE)
			{
				newthreadsize = MAXTHREADPOOLSIZE - timerThreadPool.size();
			}

			for (int i = 0; i < newthreadsize; i++)
			{
				TimerThread * p = new TimerThread(this);
				shared_ptr<TimerThread> timerthread(p);
				if (timerthread->createThread())
				{
					timerThreadPool[p] = timerthread;
				}
			}
		}
	}
private:
	std::map<TimerThread*,shared_ptr<TimerThread> > 			timerThreadPool;
	std::set<shared_ptr<TimerThread> > 							destoryTimerThreadList;


	std::map<Timer::TimerInternal*, TimerRunInfo> 				timerList;
	std::list<Timer::TimerInternal*>							needRunTimer;
	std::set<Timer::TimerInternal*>								waitAndDoingTimer;
	Mutex 														timerMutex;
	Semaphore													timerSem;

	uint64_t													prevPrintTime;

public:
	uint64_t 										curTime;
};

} // namespace Base
} // namespace Public


#endif //__TIMERMANAGER_H__
