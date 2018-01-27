//
//  Copyright (c)1998-2012, Chongqing Public Technology
//  All Rights Reserved.
//
//	Description:
//	$Id: Thread.cpp 226 2013-10-29 09:10:03Z lixiaoqiang $
//

#include <assert.h>
#include <list>

#ifdef WIN32
#include <Windows.h>
#include <stdio.h>
#include <process.h>
#elif defined(__linux__)
#include <stdio.h>
#include <algorithm>
#include <unistd.h>
#include <sys/time.h>
#include <sys/resource.h>
#define TSK_DEF_STACK_SIZE		64*1024
#include <sys/syscall.h>
#include <signal.h>
#include <stdarg.h>
#else
#error "Unknown Platform"
#endif
#include "Thread.h"
#include "Base/ThreadEx.h"
#include "Base/Timer.h"
namespace Public {
namespace Base {

#ifdef WIN32
typedef void* thread_handle_t;
#elif defined(__linux__)
typedef pthread_t thread_handle_t;
#endif

static ThreadErrorManager threadErrorManager;
static Mutex ThreadManagerInfoMutex;
class ThreadManagerInfo:public Thread
{

#define PRINTTHREADINFOMAXLONGTIME		5*60*1000

	struct ThreadInfo
	{
		std::string		threadName;
		uint32_t		threadId;
		uint32_t		poolCount;
	};
public:
	ThreadManagerInfo():Thread(NULL), prevTime(0)
	{
		createThread();
	}
	~ThreadManagerInfo()
	{
		destroyThread();
	}
	static ThreadManagerInfo* instalce()
	{
		//return CreateSingleton<ThreadManagerInfo>(SINGLETON_Levl_Base,200);
		Guard locker(ThreadManagerInfoMutex);
		static ThreadManagerInfo info;
		return &info;
	}
	void addThread(const Thread* addr,const std::string& name,uint32_t threadId)
	{
		ThreadInfo info;
		info.poolCount = 0;
		info.threadId = threadId;
		info.threadName = name;

		Guard locker(mutex);
		threadInfo[addr] = info;
	}
	void delThread(const Thread* addr)
	{
		Guard locker(mutex);
		threadInfo.erase(addr);
	}
	void threadPool(const Thread* addr)
	{
		Guard locker(mutex);
		std::map<const Thread*,ThreadInfo>::iterator iter = threadInfo.find(addr);
		if(iter == threadInfo.end())
		{
			return;
		}
		iter->second.poolCount ++;
	}
private:
	void threadProc()
	{
		while(looping())
		{
			Thread::sleep(100);
			if (Time::getCurrentMilliSecond() >= prevTime && Time::getCurrentMilliSecond() - prevTime < PRINTTHREADINFOMAXLONGTIME)
			{
				continue;
			}
			prevTime = Time::getCurrentMilliSecond();
			std::map<const Thread*,ThreadInfo> threadlist;
			{
				Guard locker(mutex);
				for(std::map<const Thread*,ThreadInfo>::iterator iter = threadInfo.begin();iter != threadInfo.end();iter ++)
				{
					threadlist[iter->first] = iter->second;
					iter->second.poolCount = 0;
				}
			}
			logtrace("Base Thread: ----- print Thread Run Info Start--------");
			uint32_t threadnum = 0;
			for(std::map<const Thread*,ThreadInfo>::iterator iter = threadlist.begin();iter != threadlist.end();iter ++,threadnum++)
			{
				logtrace("Base Thread:	%d/%d	id:#%d	name:%s poolCount:%d pooltime:%dms",threadnum+1,threadlist.size(),iter->second.threadId,iter->second.threadName.c_str(),iter->second.poolCount,PRINTTHREADINFOMAXLONGTIME);
			}
			logtrace("Base Thread: ----- print Thread Run Info End--------");
		}
	}
private:
	Mutex							mutex;
	std::map<const Thread*,ThreadInfo>	threadInfo; 
	uint64_t							prevTime;
};

struct ThreadInternal
{
	thread_handle_t 		handle;

	int						running;
	int						policy;
	int						priority;
	int						stacksize;
	int						id;
	std::string				name;
	Semaphore				createSem;
	Semaphore				quitSem;			///< 该信号量用来防止同一个对象的线程同时被创建多次；
	uint64_t				expectedtime;		///< 预计执行结束时的时间，0表示不预计
	bool					loop;
	Mutex 					mutex;				///< 互斥量
	XM_ErrorInfo 			errinfo;

#ifdef WIN32
	static void threadEnryPointer(void* param)
#else
	static void * threadEnryPointer(void* param)
#endif // WIN32
	{
		Thread* threadptr = static_cast<Thread*>(param);
		if(threadptr == NULL || threadptr->internal == NULL)
		{
#ifdef WIN32
			return ;
#else
			return NULL;
#endif
		}	
#ifdef __linux__
		if(threadptr->internal->policy == Thread::policyNormal)
		{
			int priority = -19 + threadptr->internal->priority * 40 / (Thread::priorBottom + 1);

			setpriority(PRIO_PROCESS, 0, priority);
		}
#endif

		threadptr->internal->running = true;
		threadptr->internal->id = Public::Base::Thread::getCurrentThreadID();
		threadptr->internal->createSem.post();

		threadptr->threadProc();

		{
			Guard guard(threadptr->internal->mutex);
			threadErrorManager.cleanThreadErrorInfo(threadptr->internal->id);
			threadptr->internal->running = false;
			threadptr->internal->quitSem.post();
		}

#ifdef WIN32
		_endthread();
#elif defined(__linux__)
		pthread_exit(0);
		return 0;
#endif
	}

	///检测系统中线程是否存在 true 线程还存在
	bool checkThreadIsExist()
	{
		if(!running)
		{
			return false;
		}
#ifdef WIN32
		DWORD exitCode = 0;
		if(!GetExitCodeThread(handle,&exitCode) || exitCode == STILL_ACTIVE) //获取失败，或者存活都表示存在
		{
			return true;
		}

		return false;
#elif defined(__linux__)
		int killRet = pthread_kill(handle,0);//0信号不是杀死，为系统忽略的，专门检测在线不在线的
		if(killRet == ESRCH)
		{
			return false;
		}

		return true;
#endif
	}
};

#define DEFAULTTHREADNAME	"Nonamed"

Thread::Thread(const std::string& name, int priority /* = priorDefault */, int policy /* = policyNormal */, int stackSize /* = 0 */)
{
	internal = new ThreadInternal;

	internal->priority = priority;
	internal->policy = policy;
	internal->stacksize = stackSize;
	internal->errinfo.errorCode= 0;
	internal->name = name;
	internal->id = -1;
	internal->running = false;
	internal->loop = false;
	internal->expectedtime = 0;
	internal->handle = 0;
}

Thread::~Thread()
{
	destroyThread();

	{
		///锁一下，等待那些锁住释放的对象释放
		Guard locker(internal->mutex);
	}
	delete internal;
}

void Thread::setThreadName(const std::string& name)
{
	internal->name = name;
}

bool Thread::createThread()
{
	Guard guard(internal->mutex);

	if(internal->running)
	{
		return false;
	}
	// 获得创建权，设置loop标志
	internal->loop = true;

	// 需要再次初始化这些状态
	internal->id = -1;
	internal->expectedtime = 0;

#ifdef WIN32
	int prior = 0;
	int priorMin = THREAD_PRIORITY_IDLE;
	int priorMax = THREAD_PRIORITY_TIME_CRITICAL;

	if (internal->policy == policyRealtime)
	{
		prior = THREAD_PRIORITY_TIME_CRITICAL;
	}
	else
	{
		prior = priorMax -(internal->priority - Thread::priorTop)* (priorMax - priorMin)/ (Thread::priorBottom - Thread::priorTop);
		if(prior <= THREAD_PRIORITY_IDLE)
		{
			prior = THREAD_PRIORITY_IDLE;
		}
		else if(prior <= THREAD_PRIORITY_LOWEST)
		{
			prior = THREAD_PRIORITY_LOWEST;
		}
		else if(prior <= THREAD_PRIORITY_BELOW_NORMAL)
		{
			prior = THREAD_PRIORITY_BELOW_NORMAL;
		}
		else if(prior <= THREAD_PRIORITY_NORMAL)
		{
			prior = THREAD_PRIORITY_NORMAL;
		}
		else if(prior >= THREAD_PRIORITY_TIME_CRITICAL)
		{
			prior = THREAD_PRIORITY_TIME_CRITICAL;
		}
		else if(prior >= THREAD_PRIORITY_HIGHEST)
		{
			prior = THREAD_PRIORITY_HIGHEST;
		}
		else if(prior >= THREAD_PRIORITY_ABOVE_NORMAL)
		{
			prior = THREAD_PRIORITY_ABOVE_NORMAL;
		}
		else if(prior >= THREAD_PRIORITY_NORMAL)
		{
			prior = THREAD_PRIORITY_NORMAL;
		}
	}

	internal->handle = (HANDLE)_beginthread(ThreadInternal::threadEnryPointer, internal->stacksize,(void*)this);
	BASE_Assert((internal->handle != (HANDLE)0 && internal->handle != (HANDLE)-1));
	if(internal->handle == NULL || internal->handle == (HANDLE)-1)
	{
		internal->loop = false;	
		internal->handle = 0;
	}
	else
	{
		BOOL ret = SetThreadPriority(internal->handle, prior);
		BASE_Assert(ret);
	}

	bool succ = (internal->handle != NULL);

#elif defined(__linux__)
	if( internal->stacksize < TSK_DEF_STACK_SIZE )
	{
		internal->stacksize = TSK_DEF_STACK_SIZE;
	}

	pthread_attr_t		attr;
	int ret = pthread_attr_init(&attr);
	BASE_Assert(ret == 0);

	if (internal->policy == policyRealtime)
	{
		ret = pthread_attr_setschedpolicy(&attr, SCHED_FIFO);
		BASE_Assert(ret == 0);

		// 设置linux实时线程优先级
		struct sched_param param;
		int priorMin = sched_get_priority_min(SCHED_FIFO);
		int priorMax = sched_get_priority_max(SCHED_FIFO);

		param.sched_priority = priorMax -
			(internal->priority - Thread::priorTop)
				* (priorMax - priorMin)
				/ (Thread::priorBottom - Thread::priorTop);

		//infof("fifo sched priority = %d\n", param.sched_priority);
		ret = pthread_attr_setschedparam(&attr, &param);
		BASE_Assert(ret == 0);
	}
	else
	{
		if(internal->policy != policyNormal)
		{
		//	warnf("Thread::CreateThread policy isn't set properly, policy = %d", internal->policy);
		}

		ret = pthread_attr_setschedpolicy(&attr, SCHED_RR);
		BASE_Assert(ret == 0);
	}
	pthread_attr_setdetachstate(&attr,PTHREAD_CREATE_DETACHED);
//	pthread_attr_setstacksize(&attr,internal->stacksize);
	ret = pthread_create(&internal->handle, &attr,
		(void* (*)(void *))ThreadInternal::threadEnryPointer, (void *)this);
	BASE_Assert(ret == 0);

	pthread_detach(internal->handle);
	pthread_attr_destroy(&attr);
	//assert(ret == 0);

	if(ret)
	{
		internal->loop = false;
	}

	bool succ = (ret == 0);

#endif

	if (internal->createSem.pend(10000) < 0)///5秒线程都还没起来
	{
		//别人线程来杀自己
#ifdef WIN32
		TerminateThread(internal->handle, 0);
		CloseHandle(internal->handle);
		internal->handle = NULL;
#elif defined(__linux__)
		void* status;
		::pthread_cancel(internal->handle);
		::pthread_join(internal->handle,&status);
		internal->handle = 0;
#endif
		internal->running = false;
		return false;
	}

	if(internal->name != DEFAULTTHREADNAME)
	{
		ThreadManagerInfo::instalce()->addThread(this,internal->name,internal->id);
	}	

	return succ;
}

bool Thread::destroyThread()
{
	if (internal->name != DEFAULTTHREADNAME)
	{
		ThreadManagerInfo::instalce()->delThread(this);
	}
	

	{
		Guard locker(internal->mutex);
		if(!internal->running)
		{
			return false;
		}
		internal->loop = false;
	}

	while(internal->checkThreadIsExist())
	{
		internal->quitSem.pend(100); //一直等线程直到结束
	}
	internal->running = false;
	internal->handle = 0;

	return true;
}

bool Thread::terminateThread()
{
	if (internal->name != DEFAULTTHREADNAME)
	{
		ThreadManagerInfo::instalce()->delThread(this);
	}

	internal->loop = false;

	Guard guard(internal->mutex);
	if(!internal->running)
	{
		return true;
	}

	threadErrorManager.cleanThreadErrorInfo(internal->id);

	bool isTerminateMySelft = (internal->id == Thread::getCurrentThreadID());
	
	internal->id = 0;
	internal->running = false;
	if(isTerminateMySelft)
	{
		//自己线程杀自己
#ifdef WIN32
#elif defined(__linux__)
		::pthread_exit(0);
#endif
	}
	else
	{
		//别人线程来杀自己
#ifdef WIN32
		::TerminateThread(internal->handle, 0);
		CloseHandle(internal->handle);
#elif defined(__linux__)
		void* status = NULL;
		::pthread_cancel(internal->handle);
		::pthread_join(internal->handle,&status);
#endif
		internal->handle = 0;
	}
	
	return true;
}

/// 取消线程，设置线程退出标志，非阻塞方式，不等待线程结束
bool Thread::cancelThread()
{
	if (internal->name != DEFAULTTHREADNAME)
	{
		ThreadManagerInfo::instalce()->delThread(this);
	}

	Guard guard(internal->mutex);
	if(!internal->running)
	{
		return true;
	}

	internal->loop = false;
	

	return true;
}

bool Thread::isThreadOver()
{
	return !internal->running;
}

int Thread::getThreadID()
{
	return internal->id;
}

void Thread::setTimeout(int milliSeconds)
{
	Guard guard(internal->mutex);

	if(milliSeconds == 0) // 清空预期时间
	{
		internal->expectedtime = 0;
	}
	else
	{
		internal->expectedtime = Time::getCurrentMilliSecond() + (int)milliSeconds;
	}
}

bool Thread::isTimeout()
{
	Guard guard(internal->mutex);

	return (internal->expectedtime != 0 && internal->expectedtime < Time::getCurrentMilliSecond());
}

int Thread::getCurrentThreadID()
{
#ifdef WIN32
	return (int)GetCurrentThreadId();
#elif defined(__linux__)
	return (int)syscall(SYS_gettid);
#endif
}

bool Thread::looping() const
{
	if (internal->name != DEFAULTTHREADNAME)
	{
		ThreadManagerInfo::instalce()->threadPool(this);
	}

	return internal->loop;
}

void Thread::sleep(int ms)
{
#ifdef WIN32
	Sleep(ms);
#elif defined(__linux__)
	if (ms > 0)
	{
		usleep(ms * 1000);
	}
	else
	{
		sched_yield();
	}
#else
#error "Unknown Platform"
#endif
}


class ThreadProcThread :public Thread
{
public:
	ThreadProcThread(const std::string& name,const ThreadEx::Proc& _proc, void* _param,int priority, int policy, int stackSize)
	:Thread(name, priority, policy, stackSize),proc(_proc),param(_param)
	{

	}
	void threadProc()
	{
		if(!(proc == NULL))
		{
			proc(this,param);
		}		
	}
private:
	ThreadEx::Proc proc;
	void*		param;
};

Thread* ThreadEx::creatThreadEx(const std::string& name,const ThreadEx::Proc& proc, void* param, int priority, int policy, int stackSize)
{
	ThreadProcThread *thread = new ThreadProcThread(name, proc, param, priority, policy, stackSize);
	return thread;

}

ThreadErrorManager::ThreadErrorManager(){}

ThreadErrorManager::~ThreadErrorManager(){}

bool ThreadErrorManager::cleanThreadErrorInfo(uint32_t threadId)
{
	Guard locker(mutex);

	threadErrorMap.erase(threadId);

	return true;
}

bool ThreadErrorManager::setThreadErrorInfo(uint32_t threadId,const XM_ErrorInfo& info)
{
	Guard locker(mutex);

	threadErrorMap[threadId] = info;

	return true;
}

bool ThreadErrorManager::getThreadErrorInfo(uint32_t threadId,XM_ErrorInfo& info)
{
	Guard locker(mutex);

	std::map<uint32_t, XM_ErrorInfo>::iterator iter = threadErrorMap.find(threadId);
	if(iter == threadErrorMap.end())
	{
		return false;
	}

	info = iter->second;

	return true;
}

bool XM_SetLastErrorInfo(int errCode, const char *fmt, ...)
{
	char info[ErrorInfo_MaxLength] = {0};
	va_list arg;
	va_start(arg,fmt);
	vsprintf(info,fmt,arg);
	va_end(arg);

	return XM_SetLastError(errCode,info);
}
/// 按线程设置最后的错误码
/// \param errCode [in] 错误码
/// \param info [in] 错误码的描述信息
/// \retval true 成功
/// \retval false 失败	
bool XM_SetLastError(int errCode, const char *info)
{
	XM_ErrorInfo errinfo;

	errinfo.errorCode = errCode;
	errinfo.info = info;

	return XM_SetLastErrorEx(errinfo);
}


/// 按线程设置最后的错误码
/// \param errCode [in] 错误码
/// \param info [in] 错误码的描述信息
/// \retval true 成功
/// \retval false 失败	
bool XM_SetLastErrorEx(const XM_ErrorInfo &lastinfo)
{
	return threadErrorManager.setThreadErrorInfo(Thread::getCurrentThreadID(),lastinfo);
}

/// 获得最后的错误码
/// \param errinfo [out] 错误码
/// \retval true 成功
/// \retval false 失败	
bool XM_GetLastError(XM_ErrorInfo &errinfo)
{
	errinfo.errorCode = 0;
	errinfo.info.clear();
	errinfo.stacks.clear();

	return threadErrorManager.getThreadErrorInfo(Thread::getCurrentThreadID(),errinfo);
}

/// 清空最后的错误码
/// \retval true 成功
/// \retval false 失败
bool XM_ClearLastError()
{
	return threadErrorManager.cleanThreadErrorInfo(Thread::getCurrentThreadID());
}

/// 往错误信息中添加附加的调用栈信息
/// \param detail [in] 附加信息
/// \retval true 成功
/// \retval false 失败	
bool XM_AddLastErrorStack(const char *detail)
{
	if(detail == NULL)
	{
		return false;
	}

	XM_ErrorInfo info;
	if(!XM_GetLastError(info))
	{
		return false;
	}

	info.stacks = detail;

	return XM_SetLastErrorEx(info);
}


} // namespace Base
} // namespace Public
