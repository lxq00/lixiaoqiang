#include "Base/TempCache.h"
#include "Base/Thread.h"
#include "Base/Mutex.h"
#include "Base/Guard.h"
#include "Base/IntTypes.h"
#include "Base/Timer.h"
#include "Base/Singleton.h"
#include "Base/PrintLog.h"
#include <set>
using namespace std;
namespace Xunmei{
namespace Base{

#define MAXPRINTTMPCACHEINFOTIME		5*60*1000

class ICacheIteratorManager
{
public:
	ICacheIteratorManager():printTempTime(0)
	{
		deleteTimer = new Timer("ICacheIteratorManager");
		deleteTimer->start(Timer::Proc(&ICacheIteratorManager::deleteTimerProc,this),0,1000);
	}
	~ICacheIteratorManager()
	{
		delete deleteTimer;
	}
	void addIterator(ICacheIterator* iter)
	{
		Guard locker(IteratorMutex);
		IteratorList.insert(iter);
	}
	void delIterator(ICacheIterator* iter)
	{
		Guard locker(IteratorMutex);
		IteratorList.erase(iter);
	}
	static ICacheIteratorManager* instance()
	{
		return CreateSingleton<ICacheIteratorManager>(SINGLETON_Levl_Base,3);
	}
private:
	void deleteTimerProc(unsigned long param)
	{
		uint64_t currTime = Time::getCurrentMilliSecond();
		if(currTime < printTempTime || currTime - printTempTime >= MAXPRINTTMPCACHEINFOTIME)
		{
			logtrace("Base ICacheIteratorManager:-------------print ICacheIteratorManager start ------------");
			{
				uint32_t index = 0;
				Guard locker(IteratorMutex);
				for(std::set<ICacheIterator*>::iterator iter = IteratorList.begin();iter != IteratorList.end();iter ++,index++)
				{
					ICacheIterator::CacheSizeInfo info;
					(*iter)->getCacheSizeInfo(info);

					logtrace("Base ICacheIteratorManager:	%d/%d key:%s val:%s cachecount:%d cachememsize:%llu",index+1,IteratorList.size(),
						info.keyname.c_str(),info.valname.c_str(),info.valCount,info.totalMem);
				}
			}
			logtrace("Base ICacheIteratorManager:-------------print ICacheIteratorManager end ------------");
			printTempTime = currTime;
		}


		Guard locker(IteratorMutex);

		uint64_t nowtime = Time::getCurrentMilliSecond();
		for(std::set<ICacheIterator*>::iterator iter = IteratorList.begin();iter != IteratorList.end();iter ++)
		{
			(*iter)->runCheck(nowtime);
		}
	}
private:
	Mutex						IteratorMutex;
	std::set<ICacheIterator*>	IteratorList;
	Timer*						deleteTimer;
	uint64_t					printTempTime;
};

ICacheIterator::ICacheIterator()
{
}
ICacheIterator::~ICacheIterator()
{
}

void ICacheIterator::addToManager()
{
	ICacheIteratorManager::instance()->addIterator(this);
}

void ICacheIterator::delToManager()
{
	ICacheIteratorManager::instance()->delIterator(this);
}

Mutex* ICacheIterator::getLocker()
{
	static Mutex			CacheMutex;

	return &CacheMutex;
}

} // namespace Base
} // namespace Xunmei
