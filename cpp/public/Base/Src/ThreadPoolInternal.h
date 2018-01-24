#ifndef __THREADPOLLINTERNAL_H__
#define __THREADPOLLINTERNAL_H__
#include "Base/ThreadPool.h"
#include "Base/Shared_ptr.h"
#include "Base/Semaphore.h"
#include <list>

using namespace std;

namespace Xunmei{
namespace Base{


class ThreadDispatch:public Thread
{
public:
	ThreadDispatch(ThreadPool::ThreadPoolInternal* pool);
	~ThreadDispatch();
	void threadProc();
	void cancelThread();
	void SetDispatchFunc(const ThreadPoolHandler& func,void* param);
public:
	Semaphore						Dispatchcond;
	ThreadPoolHandler				Dispatchfunc;
	void*							Dispatchparam;


	ThreadPool::ThreadPoolInternal*	threadpool;

	bool 							Dispatchthreadstatus;
};

struct ThreadPool::ThreadPoolInternal:public Thread ///��ͨ�̳߳�ģʽ
{
public:
	struct ThreadItemInfo
	{
		shared_ptr<ThreadDispatch>			dispacher;
		uint64_t							prevUsedTime; //��λ��
	};
public:
	ThreadPoolInternal(uint32_t maxSize,uint64_t threadLivetime);
	virtual ~ThreadPoolInternal();

	virtual void start();
	virtual void stop();
	virtual void threadProc();
	void refreeThraed(ThreadDispatch* thread);
	virtual bool doDispatcher(const ThreadPoolHandler& func,void* param);
protected:
	void checkThreadIsLiveOver();
protected:
	Mutex													mutex;
	std::map<ThreadDispatch*,shared_ptr<ThreadItemInfo> >	threadPoolList;
	std::map<ThreadDispatch*,shared_ptr<ThreadItemInfo> >	threadIdelList;
	uint64_t												liveTime; //��λ��
	uint32_t												maxDiapathcerSize;
};

class ThreadPoolInternalManager:public ThreadPool::ThreadPoolInternal ///����ģʽ��
{
	class ThreadPoolItem
	{
	public:
		ThreadPoolItem(){}
		ThreadPoolItem(ThreadPoolHandler _func,void * _param)
		{
			func = _func;
			param = _param;
		}
		~ThreadPoolItem(){}

		ThreadPoolHandler			func;
		void*						param;
	};
public:
	ThreadPoolInternalManager(uint32_t maxSize,uint64_t threadLivetime);
	virtual ~ThreadPoolInternalManager();

	virtual void start();
	virtual void stop();
	virtual void threadProc();
	virtual bool doDispatcher(const ThreadPoolHandler& func,void* param);
private:
	std::list<ThreadPoolItem> 	threadpoolitemlist;
	Semaphore					threadpoollistcond;
};

};//Base
};//Xunmei


#endif //__THREADPOLLINTERNAL_H__
