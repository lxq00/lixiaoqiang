#ifndef __INSTANCEOBJECTDEFINELOCKER_H__
#define __INSTANCEOBJECTDEFINELOCKER_H__

#include "Defs.h"
#include "Base/Func.h"
#include "Base/Thread.h"
#include "Base/BaseTemplate.h"
#include "Base/Mutex.h"
namespace Xunmei {
namespace Base {


typedef enum
{
	SINGLETON_Levl_Base = 0,		//Base基础库单件等级
	SINGLETON_Levl_Network,			//Network网络库单件等级
	SINGLETON_Levl_XMQ,				//XMQ通讯库单件等级
	SINGLETON_Levl_XMS,				//XMS单件等级
	SINGLETON_Levl_APP = 10,		//其他引用单件等级
}SINGLETON_Levl;			//注：优先级越高的单件，释放越早,同一个优先级的应用释放顺序随机

class BASE_API SingleHolderInstance
{
public:
	SingleHolderInstance();
	virtual ~SingleHolderInstance();
	void addInstanceObjectHolder(uint32_t lev,uint32_t runlev,const char* classTypeFlag);

	static void lockInstanceMap();
	static void unlockInstanceMap();
	static SingleHolderInstance* findInstanceHolder(uint32_t lev,uint32_t runlev,const char* classType);
	static void uninit();
};

template<typename T>
class SingleHolder:public SingleHolderInstance
{
public:
	SingleHolder(uint32_t lev,uint32_t runlev):obj(NULL){addInstanceObjectHolder(lev,runlev,typeid(T).name());}
	~SingleHolder(){SAFE_DELETE(obj);}
	void lock(){mutex.enter();}
	void unlock(){mutex.leave();}
	T* getObjc(){return obj;}
	void setObjc(T* _obj){obj = _obj;}
private:
	Mutex			mutex;
	T*				obj;
};

///创建申请单件的API，该api可以将单件对象放于base中，可以控制单件释放顺序
///param[in] lev		SINGLETON_Levl 定义
///param[in] runlev		区分同一个lev下的不同单件的级别
///param[out] T*		返回单件的地址
template<typename T>
inline T* CreateSingleton(uint32_t lev,uint32_t runLev)
{
	SingleHolderInstance* instanceHolder = NULL;
	///先找该单件的holder
	{
		SingleHolderInstance::lockInstanceMap();
		instanceHolder = SingleHolderInstance::findInstanceHolder(lev,runLev,typeid(T).name());
		if(instanceHolder == NULL)
		{
			instanceHolder = new SingleHolder<T>(lev,runLev);
		}
		SingleHolderInstance::unlockInstanceMap();
	}
	if(instanceHolder == NULL)
	{
		return NULL;
	}
	SingleHolder<T>* sigletion = (SingleHolder<T>*)instanceHolder;
	T* singleton = NULL;
	///在通过holder来创建单件
	{
		sigletion->lock();
		singleton = sigletion->getObjc();
		if(singleton == NULL)
		{
			singleton = new T();
			sigletion->setObjc(singleton);
		}
		sigletion->unlock();
	}	

	return singleton;
}

}
}


#endif //__INSTANCEOBJECTDEFINELOCKER_H__
