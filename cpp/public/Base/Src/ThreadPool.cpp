//
//  Copyright (c)1998-2012, Chongqing Xunmei Technology
//  All Rights Reserved.
//
//	Description:
//	$Id: ThreadPool.cpp 3 2013Äê8ÔÂ21ÈÕ10:18:30 lixiaoqiang $
#include <string.h>
#include "Base/ThreadPool.h"
#include "ThreadPoolInternal.h"
#include "Base/Semaphore.h"
#include "Base/BaseTemplate.h"
#include "Base/Singleton.h"
#include "Base/Shared_ptr.h"
namespace Xunmei{
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

void ThreadDispatch::SetDispatchFunc(const ThreadPoolHandler& func,void* param)
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
:Thread("ThreadPoolInternal"),liveTime(threadLivetime),maxDiapathcerSize(maxSize){}

ThreadPool::ThreadPoolInternal::~ThreadPoolInternal(){}

void ThreadPool::ThreadPoolInternal::start()
{
	createThread();
}
void ThreadPool::ThreadPoolInternal::stop()
{
	cancelThread();

	{
		Guard locker(mutex);
		threadPoolList.clear();
		threadIdelList.clear();
	}
	

	destroyThread();
}
void ThreadPool::ThreadPoolInternal::threadProc()
{
	Semaphore waitsem;
	uint64_t prevTime = 0,printtime = 0;
	while(looping())
	{
		waitsem.pend(500);
		if(Time::getCurrentMilliSecond() < prevTime || Time::getCurrentMilliSecond() - prevTime >= CHECKTHREADIDELETIME)
		{
			checkThreadIsLiveOver();

			prevTime = Time::getCurrentMilliSecond();
		}

		if(Time::getCurrentMilliSecond() < printtime || Time::getCurrentMilliSecond() - printtime >= MAXPRINTTHREADPOOLTIME)
		{
			Guard locker(mutex);

			logtrace("Base ThreadPoolInternal:	 threadPoolList:%d threadIdelList:%d maxDiapathcerSize:%d",threadPoolList.size(),threadIdelList.size(),maxDiapathcerSize);

			printtime = Time::getCurrentMilliSecond();
		}
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
bool ThreadPool::ThreadPoolInternal::doDispatcher(const ThreadPoolHandler& func,void* param)
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
ThreadPoolInternalManager::ThreadPoolInternalManager(uint32_t maxSize,uint64_t threadLivetime):ThreadPool::ThreadPoolInternal(maxSize,threadLivetime){}
ThreadPoolInternalManager::~ThreadPoolInternalManager(){}

void ThreadPoolInternalManager::start()
{
	createThread();
}
void ThreadPoolInternalManager::stop()
{
	ThreadPool::ThreadPoolInternal::stop();

	{
		Guard locker(mutex);
		threadpoolitemlist.clear();
	}
}
void ThreadPoolInternalManager::threadProc()
{
	uint64_t prevTime = 0,printtime = 0;

	while(looping())
	{
		{
			if(Time::getCurrentMilliSecond() < prevTime || Time::getCurrentMilliSecond() - prevTime >= CHECKTHREADIDELETIME)
			{
				checkThreadIsLiveOver();

				prevTime = Time::getCurrentMilliSecond();
			}

			if(Time::getCurrentMilliSecond() < printtime || Time::getCurrentMilliSecond() - printtime >= MAXPRINTTHREADPOOLTIME)
			{
				Guard locker(mutex);

				logtrace("Base ThreadPoolInternalManager:	 threadpoolitemlist:%d threadPoolList:%d threadIdelList:%d maxDiapathcerSize:%d",threadpoolitemlist.size(),threadPoolList.size(),threadIdelList.size(),maxDiapathcerSize);

				printtime = Time::getCurrentMilliSecond();
			}
		}	

		ThreadPoolItem item;
		bool isHaveItem = false;
		if(threadpoollistcond.pend(100) >= 0)
		{
			Guard locker(mutex);

			if(threadpoolitemlist.size() <= 0)
			{
				continue;
			}			
			item = threadpoolitemlist.front();
			isHaveItem = true;
		}

		if(isHaveItem)
		{
			if(ThreadPool::ThreadPoolInternal::doDispatcher(item.func,item.param))
			{
				Guard locker(mutex);
				threadpoolitemlist.pop_front();
			}
		}
	}
}
bool ThreadPoolInternalManager::doDispatcher(const ThreadPoolHandler& func,void* param)
{
	Guard locker(mutex);

	if(threadpoolitemlist.size() > maxDiapathcerSize)
	{
		return false;
	}

	ThreadPoolItem item(func,param);

	threadpoolitemlist.push_back(item);
	threadpoollistcond.post();

	return true;
}

ThreadPool::ThreadPool(Type type,uint32_t maxDiapathcerSize,uint32_t liveTime)
{
	if(type == Type_Manager)
	{
		internal = new ThreadPoolInternalManager(maxDiapathcerSize,liveTime);
	}
	else
	{
		internal = new ThreadPoolInternal(maxDiapathcerSize,liveTime);
	}
	internal->start();
}

ThreadPool::~ThreadPool()
{
	internal->stop();
	SAFE_DELETE(internal);
}

bool ThreadPool::Dispatch(const ThreadPoolHandler& func,void* param)
{
	return internal->doDispatcher(func,param);
}



void ThreadPool::uninit()
{
	if(internal != NULL)
	{
		SAFE_DELETE(internal);
	}
}

ThreadPool* ThreadPool::instance()
{
	return CreateSingleton<ThreadPool>(SINGLETON_Levl_Base,1);
}

};//Base
};//Xunmei


