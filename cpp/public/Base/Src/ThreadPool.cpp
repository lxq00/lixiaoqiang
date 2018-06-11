//
//  Copyright (c)1998-2012,  Public Technology
//  All Rights Reserved.
//
//	Description:
//	$Id: ThreadPool.cpp 3 2013Äê8ÔÂ21ÈÕ10:18:30  $
#include <string.h>
#include "Base/ThreadPool.h"
#include "ThreadPoolInternal.h"
#include "Base/Semaphore.h"
#include "Base/BaseTemplate.h"
#include "Base/Shared_ptr.h"
#include "Base/Guard.h"
#include "Base/Time.h"
#include "Base/PrintLog.h"
namespace Public{
namespace Base{

ThreadDispatch::ThreadDispatch(ThreadPool::ThreadPoolInternal* pool): Thread("[ThreadDispatch]", priorTop, policyRealtime)
{
	threadpool = pool;
	Dispatchthreadstatus = false;

	createThread();
}
ThreadDispatch::~ThreadDispatch()
{
	cancelThread();
}
void ThreadDispatch::cancelThread()
{
	Thread::cancelThread();
	Dispatchcond.post();
	Thread::destroyThread();
}
void 	ThreadDispatch::threadProc()
{
	while(looping())
	{
		Dispatchcond.pend();
		if(!looping())
		{
			break;
		}
		Dispatchthreadstatus = true;
		Dispatchfunc(Dispatchparam);
		Dispatchthreadstatus = false;

		threadpool->refreeThraed(this);
	}
}

void ThreadDispatch::SetDispatchFunc(const ThreadPool::Proc& func,void* param)
{
	if(!Dispatchthreadstatus)
	{
		Dispatchfunc = func;
		Dispatchparam = param;
		Dispatchcond.post();
	}
}

#define MAXPRINTTHREADPOOLTIME		5*60*1000
#define CHECKTHREADIDELETIME		2*1000

ThreadPool::ThreadPoolInternal::ThreadPoolInternal(uint32_t maxSize,uint64_t threadLivetime)
:liveTime(threadLivetime),maxDiapathcerSize(maxSize)
{
	prevTime = 0, printtime = 0;
}

ThreadPool::ThreadPoolInternal::~ThreadPoolInternal(){}

void ThreadPool::ThreadPoolInternal::start()
{
	pooltimer = make_shared<Timer>("ThreadPoolInternal");
	pooltimer->start(Timer::Proc(&ThreadPoolInternal::poolTimerProc, this), 0, 1000);
}
void ThreadPool::ThreadPoolInternal::stop()
{
	pooltimer = NULL;

	{
		Guard locker(mutex);
		threadPoolList.clear();
		threadIdelList.clear();
	}
	
}
void ThreadPool::ThreadPoolInternal::poolTimerProc(unsigned long)
{
	if(Time::getCurrentMilliSecond() < prevTime || Time::getCurrentMilliSecond() - prevTime >= CHECKTHREADIDELETIME)
	{
		checkThreadIsLiveOver();

		prevTime = Time::getCurrentMilliSecond();
	}

	if (Time::getCurrentMilliSecond() < printtime || Time::getCurrentMilliSecond() - printtime >= MAXPRINTTHREADPOOLTIME)
	{
		Guard locker(mutex);

		logtrace("Base ThreadPoolInternal:	 threadPoolList:%d threadIdelList:%d maxDiapathcerSize:%d", threadPoolList.size(), threadIdelList.size(), maxDiapathcerSize);

		printtime = Time::getCurrentMilliSecond();
	}
}

void ThreadPool::ThreadPoolInternal::refreeThraed(ThreadDispatch* thread)
{
	Guard locker(mutex);
	std::map<ThreadDispatch*,shared_ptr<ThreadItemInfo> >::iterator iter = threadPoolList.find(thread);
	if(iter == threadPoolList.end())
	{
		return;
	}

	iter->second->prevUsedTime = Time::getCurrentTime().makeTime();
	threadIdelList[thread] = iter->second;
}
bool ThreadPool::ThreadPoolInternal::doDispatcher(const ThreadPool::Proc& func,void* param)
{
	Guard locker(mutex);
	if(threadIdelList.size() > 0)
	{
		std::map<ThreadDispatch*,shared_ptr<ThreadItemInfo> >::iterator iter = threadIdelList.begin();

		iter->second->dispacher->SetDispatchFunc(func,param);

		threadIdelList.erase(iter);
	}
	else if(threadPoolList.size() < maxDiapathcerSize)
	{
		shared_ptr<ThreadItemInfo> info = shared_ptr<ThreadItemInfo>(new ThreadItemInfo);
		if(info == (void*)NULL)
		{
			return false;
		}
		info->dispacher = make_shared<ThreadDispatch>(this);
		if(info->dispacher == (void*)NULL)
		{
			return false;
		}

		threadPoolList[info->dispacher.get()] = info;
		info->dispacher->SetDispatchFunc(func,param);
	}
	else
	{
		return false;
	}

	return true;
}

void ThreadPool::ThreadPoolInternal::checkThreadIsLiveOver()
{
	uint64_t nowtime = Time::getCurrentTime().makeTime();
	std::list<shared_ptr<ThreadItemInfo> > needFreeThreadList;

	{
		Guard locker(mutex);

		std::map<ThreadDispatch*,shared_ptr<ThreadItemInfo> >::iterator iter;
		std::map<ThreadDispatch*,shared_ptr<ThreadItemInfo> >::iterator iter1;
		for(iter = threadIdelList.begin();iter != threadIdelList.end(); iter = iter1)
		{
			iter1 = iter;
			iter1 ++;

			if(liveTime == 0 || nowtime - iter->second->prevUsedTime >= liveTime || iter->second->prevUsedTime > nowtime)
			{
				needFreeThreadList.push_back(iter->second);
				threadPoolList.erase(iter->second->dispacher.get());
				threadIdelList.erase(iter);
			}
		}
	}
}
ThreadPool::ThreadPool(uint32_t maxDiapathcerSize,uint32_t liveTime)
{
	internal = new ThreadPoolInternal(maxDiapathcerSize,liveTime);
	internal->start();
}

ThreadPool::~ThreadPool()
{
	internal->stop();
	SAFE_DELETE(internal);
}

bool ThreadPool::dispatch(const ThreadPool::Proc& func,void* param)
{
	return internal->doDispatcher(func,param);
}


};//Base
};//Public


