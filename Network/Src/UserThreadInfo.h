#ifndef __ASIOSOCKET_OBJCET_USERTHREADINFO_DEFINE_H__
#define __ASIOSOCKET_OBJCET_USERTHREADINFO_DEFINE_H__
#include "ASIOServerPool.h"
#include "Network/Socket.h"
namespace Public {
namespace Network {

class UserThreadInfo
{
public:
	UserThreadInfo():mustQuit(false),usedCallbackThreadId(0),usedCallbackThreadNum(0){}
	~UserThreadInfo(){}
	void quit() 
	{
		Guard locker(mutex);
		mustQuit = true;
	}
	void waitAllOtherCallbackThreadUsedEnd()////等待所有回调线程使用结束
	{
		uint32_t currThreadId = Thread::getCurrentThreadID();
		while(1)
		{
			{
				Guard locker(mutex);
				if(usedCallbackThreadNum == 0 || (usedCallbackThreadNum == 1 && usedCallbackThreadId == currThreadId))
				{
					///如果没有线程使用，或者是自己线程关闭自己，可以关闭
					break;
				}
			}
			Thread::sleep(10);
		}
	}
	bool callbackThreadUsedStart()///记录当前回调线程使用信息
	{
		Guard locker(mutex);
		if (mustQuit) return false;

		uint32_t currThreadId = Thread::getCurrentThreadID();
		usedCallbackThreadId += currThreadId;
		++usedCallbackThreadNum;

		return true;
	}
	bool callbackThreadUsedEnd() ///当前回调线程使用结束
	{
		Guard locker(mutex);
		
		uint32_t currThreadId = Thread::getCurrentThreadID();
		usedCallbackThreadId -= currThreadId;
		--usedCallbackThreadNum;

		return true;
	}
protected:
	Mutex											mutex;
	bool											mustQuit;				///是不是已经被关闭
	uint64_t										usedCallbackThreadId;	//正在使用回调的线程ID和
	uint32_t										usedCallbackThreadNum;	//正在使用回调的线程数
};

}
}


#endif //__ASIOSOCKET_OBJCET_H__
