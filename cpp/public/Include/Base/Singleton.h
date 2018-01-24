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
	SINGLETON_Levl_Base = 0,		//Base�����ⵥ���ȼ�
	SINGLETON_Levl_Network,			//Network����ⵥ���ȼ�
	SINGLETON_Levl_XMQ,				//XMQͨѶ�ⵥ���ȼ�
	SINGLETON_Levl_XMS,				//XMS�����ȼ�
	SINGLETON_Levl_APP = 10,		//�������õ����ȼ�
}SINGLETON_Levl;			//ע�����ȼ�Խ�ߵĵ������ͷ�Խ��,ͬһ�����ȼ���Ӧ���ͷ�˳�����

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

///�������뵥����API����api���Խ������������base�У����Կ��Ƶ����ͷ�˳��
///param[in] lev		SINGLETON_Levl ����
///param[in] runlev		����ͬһ��lev�µĲ�ͬ�����ļ���
///param[out] T*		���ص����ĵ�ַ
template<typename T>
inline T* CreateSingleton(uint32_t lev,uint32_t runLev)
{
	SingleHolderInstance* instanceHolder = NULL;
	///���Ҹõ�����holder
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
	///��ͨ��holder����������
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
