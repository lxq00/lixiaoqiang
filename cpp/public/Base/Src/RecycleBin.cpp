#include "Base/RecycleBin.h"
#include "Base/Guard.h"
#include "Base/Timer.h"
#include "Base/Singleton.h"
namespace Xunmei{
namespace Base{

struct RecourceProc::RecourceProcInternal
{
	RecourceProcInternal()
	{
		syncTimer = new Timer("RecourceProcInternal");
		syncTimer->start(Timer::Proc(&RecourceProcInternal::poolRecourceProc,this),0,1000);
	}
	~RecourceProcInternal()
	{
		SAFE_DELETE(syncTimer);
	}

	void poolRecourceProc(unsigned long param)
	{
		std::list<IResource*> freeList;

		{
			Guard locker(mutex);

			uint64_t nowtime = Time::getCurrentMilliSecond();
			std::map<IResource*,RecourceInfo>::iterator iter1;
			std::map<IResource*,RecourceInfo>::iterator iter2;
			for(iter1 = recourceList.begin();iter1 != recourceList.end();iter1 = iter2)
			{
				iter2 = iter1;
				iter2 ++;

				if(nowtime - iter1->second.startTime >= iter1->second.timeout)
				{
					freeList.push_back(iter1->first);
					recourceList.erase(iter1);
				}
			}
		}

		std::list<IResource*>::iterator iter;
		for(iter = freeList.begin();iter != freeList.end();iter ++)
		{
			SAFE_DELETE(*iter);
		}
	}


	struct RecourceInfo
	{
		uint64_t		startTime;
		uint64_t		timeout;
	};

	Timer*								syncTimer;
	Mutex								mutex;
	std::map<IResource*,RecourceInfo>	recourceList;
};
RecourceProc::RecourceProc()
{
	internal = new RecourceProcInternal();
}
RecourceProc::~RecourceProc()
{
	SAFE_DELETE(internal);
}
void RecourceProc::put(IResource* res,uint64_t timeout)
{
	Guard locker(internal->mutex);
	std::map<IResource*,RecourceProcInternal::RecourceInfo>::iterator iter = internal->recourceList.find(res);
	if(iter != internal->recourceList.end())
	{
		return;
	}


	RecourceProcInternal::RecourceInfo info = {Time::getCurrentMilliSecond(),timeout};
	internal->recourceList[res] = info;
}
RecourceProc* RecourceProc::instance()
{
	return CreateSingleton<RecourceProc>(SINGLETON_Levl_Base,4);
}

}
}