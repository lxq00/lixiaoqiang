#ifndef __THREADPOLLINTERNAL_H__
#define __THREADPOLLINTERNAL_H__
#include "Base/ThreadPool.h"
#include "Base/Shared_ptr.h"
#include "Base/Semaphore.h"
#include "Base/Timer.h"
#include <list>

using namespace std;

namespace Public{
namespace Base{


class ThreadDispatch:public Thread
{
public:
	ThreadDispatch(ThreadPool::ThreadPoolInternal* pool);
	~ThreadDispatch();
	void threadProc();
	void cancelThread();
	void SetDispatchFunc(const ThreadPool::Proc& func,void* param);
public:
	Semaphore						Dispatchcond;
	ThreadPool::Proc				Dispatchfunc;
	void*							Dispatchparam;


	ThreadPool::ThreadPoolInternal*	threadpool;

	bool 							Dispatchthreadstatus;
};

struct ThreadPool::ThreadPoolInternal
{
public:
	struct ThreadItemInfo
	{
		shared_ptr<ThreadDispatch>			dispacher;
		uint64_t							prevUsedTime; //单位秒
	};
public:
	ThreadPoolInternal(uint32_t maxSize,uint64_t threadLivetime);
	virtual ~ThreadPoolInternal();

	virtual void start();
	virtual void stop();
	virtual void poolTimerProc(unsigned long);
	void refreeThraed(ThreadDispatch* thread);
	virtual bool doDispatcher(const ThreadPool::Proc& func,void* param);
protected:
	void checkThreadIsLiveOver();
protected:
	Mutex													mutex;
	std::map<ThreadDispatch*,shared_ptr<ThreadItemInfo> >	threadPoolList;
	std::map<ThreadDispatch*,shared_ptr<ThreadItemInfo> >	threadIdelList;
	uint64_t												liveTime; //单位秒
	uint32_t												maxDiapathcerSize;
	shared_ptr<Timer>										pooltimer;

	uint64_t prevTime, printtime;
};


};//Base
};//Public


#endif //__THREADPOLLINTERNAL_H__
