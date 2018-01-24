#include "Base/Singleton.h"
#include "Base/Guard.h"
namespace Xunmei{
namespace Base{

class SingletonManager
{
	struct RunAppLevObject
	{
		std::map<std::string,SingleHolderInstance*>		singletonList;
	};
	struct AppLevObject
	{
		std::map<uint32_t,RunAppLevObject>		runList;
	};

	Mutex								mutex;
	std::map<uint32_t,AppLevObject>		appList;
public:
	SingletonManager(){}
	~SingletonManager()
	{
		//uninit();
	}
	static SingletonManager* instance()
	{
		static SingletonManager manager;
		return &manager;
	}
	void _addInstanceHolderInMap(uint32_t lev,uint32_t runlev,const char* classType,SingleHolderInstance* holder)
	{
		std::map<uint32_t,AppLevObject>::iterator iter = appList.find(lev);
		if(iter == appList.end())
		{
			appList[lev] = AppLevObject();
			iter = appList.find(lev);
		}
		std::map<uint32_t,RunAppLevObject>::iterator runiter = iter->second.runList.find(runlev);
		if(runiter == iter->second.runList.end())
		{
			iter->second.runList[runlev] = RunAppLevObject();
			runiter = iter->second.runList.find(runlev);
		}

		runiter->second.singletonList[classType] = holder;
	}

	SingleHolderInstance* _findInstanceHolder(uint32_t lev,uint32_t runlev,const char* classType)
	{
		std::map<uint32_t,AppLevObject>::iterator iter = appList.find(lev);
		if(iter == appList.end())
		{
			return NULL;
		}
		std::map<uint32_t,RunAppLevObject>::iterator runiter = iter->second.runList.find(runlev);
		if(runiter == iter->second.runList.end())
		{
			return NULL;
		}
		std::map<std::string,SingleHolderInstance*>::iterator singiter = runiter->second.singletonList.find(classType);
		if(singiter == runiter->second.singletonList.end())
		{
			return NULL;
		}

		return singiter->second;
	}

	void _lockMutex()
	{
		mutex.enter();
	}
	void _unlockMutex()
	{
		mutex.leave();
	}

	void uninit()
	{
		while (1)
		{
			SingleHolderInstance* instance = NULL;

			{
				Guard locker(mutex);
				if (appList.size() <= 0)
				{
					break;
				}

				std::list<uint32_t> applevlist;
				for (std::map<uint32_t, AppLevObject>::iterator iter = appList.begin(); iter != appList.end(); iter++)
				{
					applevlist.push_back(iter->first);
				}

				applevlist.sort();

				std::map<uint32_t, AppLevObject>::iterator iter = appList.find(applevlist.back());
				if (iter == appList.end())
				{
					continue;
				}

				if (iter->second.runList.size() <= 0)
				{
					appList.erase(iter);
					continue;
				}

				std::list<uint32_t> applist;
				for (std::map<uint32_t, RunAppLevObject>::iterator runiter = iter->second.runList.begin(); runiter != iter->second.runList.end(); runiter++)
				{
					applist.push_back(runiter->first);
				}

				applist.sort();

				std::map<uint32_t, RunAppLevObject>::iterator runiter = iter->second.runList.find(applist.back());
				if (runiter == iter->second.runList.end())
				{
					continue;
				}

				if (runiter->second.singletonList.size() <= 0)
				{
					iter->second.runList.erase(runiter);
					continue;
				}

				std::map<std::string, SingleHolderInstance*>::iterator singiter = runiter->second.singletonList.begin();
				instance = singiter->second;
				runiter->second.singletonList.erase(singiter);
			}

			SAFE_DELETE(instance);
		}
		{
			Guard locker(mutex);
			appList.clear();
		}
	}
};

SingleHolderInstance::SingleHolderInstance(){}
SingleHolderInstance::~SingleHolderInstance(){}
void SingleHolderInstance::addInstanceObjectHolder(uint32_t lev,uint32_t runlev,const char* classTypeFlag)
{
	SingletonManager::instance()->_addInstanceHolderInMap(lev,runlev,classTypeFlag,this);
}
void SingleHolderInstance::lockInstanceMap()
{
	SingletonManager::instance()->_lockMutex();
}
void SingleHolderInstance::unlockInstanceMap()
{
	SingletonManager::instance()->_unlockMutex();
}
SingleHolderInstance* SingleHolderInstance::findInstanceHolder(uint32_t lev,uint32_t runlev,const char* classType)
{
	return SingletonManager::instance()->_findInstanceHolder(lev,runlev,classType);
}
void SingleHolderInstance::uninit()
{
	SingletonManager::instance()->uninit();
}
}
}