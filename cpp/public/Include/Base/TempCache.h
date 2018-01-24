#ifndef __TEMPCACHE_H__
#define __TEMPCACHE_H__
#include "Base/IntTypes.h"
#include "Base/Mutex.h"
#include "Base/Guard.h"
#include "Base/Time.h"
#include "Base/Func.h"
#include "Base/Singleton.h"
#include "Base/Shared_ptr.h"
#include <map>
#include <list>
#include <string>
using namespace std;
namespace Xunmei{
namespace Base{

class BASE_API ICacheIterator
{
public:
	struct CacheSizeInfo
	{
		std::string		keyname;
		std::string		valname;
		uint32_t		valCount;
		uint64_t		totalMem;
	};
public:
	ICacheIterator();
	virtual ~ICacheIterator();
	virtual void runCheck(uint64_t nowtime) = 0;
	virtual void getCacheSizeInfo(CacheSizeInfo& info) = 0;
	static Mutex* getLocker();
protected:
	void addToManager();
	void delToManager();
};

#define CACHETIMEOUT		1000
template<typename k,typename v>
class CacheIterator:public ICacheIterator
{
public:
	typedef Function2<void,k,v>		StatusNotifyFunc;
public:
	CacheIterator(){addToManager();}
	~CacheIterator(){delToManager();}
	bool addTemp(k key,v val,uint64_t t = CACHETIMEOUT,const StatusNotifyFunc& func = NULL)
	{
		Guard locker(tempMutex);
		typename std::map<k,tempItem>::iterator iter = tempList.find(key);
		if(iter != tempList.end())
		{
			iter->second.value = val;
			iter->second.timeout = t;
			iter->second.func = func;
			iter->second.updateTime = Time::getCurrentMilliSecond();
			return true;
		}
		tempItem item;
		item.value = val;
		item.timeout = t;
		item.func = func;
		item.key = key;
		item.updateTime = Time::getCurrentMilliSecond();
		tempList.insert(pair<k,tempItem>(key,item));

		return true;
	}
	bool getTemp(k key,v& val)
	{
		Guard locker(tempMutex);
		typename std::map<k,tempItem>::iterator iter = tempList.find(key);
		if(iter == tempList.end())
		{
			return false;
		}
		val = iter->second.value;
		return true;
	}
	void cleanTemp(k key)
	{
		Guard locker(tempMutex);
		typename std::map<k,tempItem>::iterator iter = tempList.find(key);
		if(iter != tempList.end())
		{
			tempList.erase(iter);
		}
	}
	static CacheIterator* instance()
	{
	//	Guard locker(ICacheIterator::getLocker());

		return CreateSingleton<CacheIterator<k, v> >(SINGLETON_Levl_Base, 3);
	}
private:
	void getCacheSizeInfo(CacheSizeInfo& info)
	{
		{
			Guard locker(tempMutex);
			info.valCount = tempList.size();
		}

		info.keyname = typeid(k).name();
		info.valname = typeid(v).name();		
		info.totalMem = (sizeof(v) + sizeof(tempItem)) * info.valCount;
	}
	void runCheck(uint64_t nowtime)
	{
		typename std::map<k,tempItem>::iterator iter1;
		typename std::map<k,tempItem>::iterator iter2;

		std::list<tempItem> 		timeoutList;

		tempMutex.enter();
		for(iter1 = tempList.begin();iter1 != tempList.end();iter1 = iter2)
		{
			iter2 = iter1;
			iter2 ++;
			if(nowtime >= iter1->second.updateTime && nowtime - iter1->second.updateTime > iter1->second.timeout)
			{
				timeoutList.push_back(iter1->second);
				tempList.erase(iter1);
			}
		}
		tempMutex.leave();

		typename std::list<tempItem>::iterator iter;
		for(iter = timeoutList.begin();iter != timeoutList.end();iter ++)
		{
			if(!(iter->func ==NULL))
			{
				iter->func(iter->key,iter->value);
			}
		}
	}
private:
	struct tempItem
	{
		k					key;
		uint64_t			updateTime;
		v					value;
		uint64_t			timeout;
		StatusNotifyFunc	func;
	};
	Mutex						tempMutex;
	std::map<k,tempItem>		tempList;
};


template<typename k,typename v>
bool AddToTempCache(k key,v val,uint64_t timeout = CACHETIMEOUT ,const Function2<void,k,v>& TimeoutNotify = NULL)
{
	return CacheIterator<k,v>::instance()->addTemp(key,val,timeout,TimeoutNotify);
}
// template<typename v>
// bool AddToTempCache(const char* key,v val,uint64_t timeout = CACHETIMEOUT ,Function2<void,const char*,v> TimeoutNotify = NULL)
// {
// 	ERROR("请使用 AddToTempCache<std::string,v> 来替换 AddToTempCache<const char*,v>");
// }
// template<typename v>
// bool AddToTempCache(const unsigned char* key,v val,uint64_t timeout = CACHETIMEOUT,Function2<void,const unsigned char*,v> TimeoutNotify = NULL )
// {
// 	ERROR("请使用 AddToTempCache<std::string,v> 来替换 AddToTempCache<const char*,v>");
// }

template<typename k,typename v>
bool GetFromTempCache(k key,v& val)
{
	return CacheIterator<k,v>::instance()->getTemp(key,val);
}

template<typename v>
bool GetFromTempCache(const char* key,v& val)
{
	return GetFromTempCache<std::string,v>((const char*)key,val);
}
template<typename v>
bool GetFromTempCache(const unsigned char* key,v& val)
{
	return GetFromTempCache<std::string,v>((const char*)key,val);
}

template<typename k,typename v>
void CleanTempCache(k key)
{
	return CacheIterator<k,v>::instance()->cleanTemp(key);
}





template<typename k, typename v>
class CacheIteratorEx :public ICacheIterator
{
public:
	typedef Function2<void, const k&, const shared_ptr<v>& >		StatusNotifyFunc;
public:
	CacheIteratorEx() { addToManager(); }
	~CacheIteratorEx() { delToManager(); }
	bool addTemp(const k& key, const shared_ptr<v>& val, uint64_t t = CACHETIMEOUT, const StatusNotifyFunc& func = NULL)
	{
		Guard locker(tempMutex);
		typename std::map<k, tempItem>::iterator iter = tempList.find(key);
		if (iter != tempList.end())
		{
			iter->second.value = val;
			iter->second.timeout = t;
			iter->second.func = func;
			iter->second.updateTime = Time::getCurrentMilliSecond();
			return true;
		}
		tempItem item;
		item.value = val;
		item.timeout = t;
		item.func = func;
		item.key = key;
		item.updateTime = Time::getCurrentMilliSecond();
		tempList.insert(pair<k, tempItem>(key, item));

		return true;
	}
	bool getTemp(const k& key, shared_ptr<v>& val)
	{
		Guard locker(tempMutex);
		typename std::map<k, tempItem>::iterator iter = tempList.find(key);
		if (iter == tempList.end())
		{
			return false;
		}
		val = iter->second.value;
		return true;
	}
	void cleanTemp(const k& key)
	{
		Guard locker(tempMutex);
		typename std::map<k, tempItem>::iterator iter = tempList.find(key);
		if (iter != tempList.end())
		{
			tempList.erase(iter);
		}
	}
	static CacheIteratorEx* instance()
	{
		Guard locker(ICacheIterator::getLocker());

		static CacheIteratorEx<k, v> CacheInter;

		return &CacheInter;
	}
private:
	void getCacheSizeInfo(CacheSizeInfo& info)
	{
		{
			Guard locker(tempMutex);
			info.valCount = tempList.size();
		}

		info.keyname = typeid(k).name();
		info.valname = typeid(v).name();
		info.totalMem = (sizeof(v) + sizeof(tempItem)) * info.valCount;
	}
	void runCheck(uint64_t nowtime)
	{
		typename std::map<k, tempItem>::iterator iter1;
		typename std::map<k, tempItem>::iterator iter2;

		std::list<tempItem> 		timeoutList;

		tempMutex.enter();
		for (iter1 = tempList.begin(); iter1 != tempList.end(); iter1 = iter2)
		{
			iter2 = iter1;
			iter2++;
			if (nowtime - iter1->second.updateTime > iter1->second.timeout)
			{
				timeoutList.push_back(iter1->second);
				tempList.erase(iter1);
			}
		}
		tempMutex.leave();

		typename std::list<tempItem>::iterator iter;
		for (iter = timeoutList.begin(); iter != timeoutList.end(); iter++)
		{
			if (!(iter->func == NULL))
			{
				iter->func(iter->key, iter->value);
			}
		}
	}
private:
	struct tempItem
	{
		k					key;
		uint64_t			updateTime;
		shared_ptr<v>		value;
		uint64_t			timeout;
		StatusNotifyFunc	func;
	};
	Mutex						tempMutex;
	std::map<k, tempItem>		tempList;
};


template<typename k, typename v>
bool AddToTempCacheEx(const k& key, const shared_ptr<v>& val, uint64_t timeout = CACHETIMEOUT, const Function2<void, const k&, const shared_ptr<v>&>& TimeoutNotify = NULL)
{
	typename CacheIteratorEx<k, v>::StatusNotifyFunc nofity(TimeoutNotify);
	return CacheIteratorEx<k, v>::instance()->addTemp(key, val, timeout, nofity);
}
template<typename k, typename v>
bool GetFromTempCacheEx(const k& key, shared_ptr<v>& val)
{
	return CacheIteratorEx<k, v>::instance()->getTemp(key, val);
}

template<typename k, typename v>
void CleanTempCacheEx(const k& key)
{
	return CacheIteratorEx<k, v>::instance()->cleanTemp(key);
}



} // namespace Base
} // namespace Xunmei


#endif //__TEMPCACHE_H__
