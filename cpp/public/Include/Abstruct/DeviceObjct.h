#ifndef __ABSTUCT_DEVICEOBJ_H__
#define __ABSTUCT_DEVICEOBJ_H__

#include "Abstruct/Defs.h"
#include "Public/DeviceDefine.h"
#include "Base/Base.h"
#include "JSON/json.h"
using namespace Public::Base;
namespace Public {
namespace Abstruct {

class ABSTRUCT_API IDeviceObject
{
public:
	typedef struct {
		DeviceObjectType devType;	//设备型号
		std::string identify;		//设备标识  如：mac+gwid+channelid
		std::string fatherIdentify; //父标识
		std::string name;          //设备名称
		std::string models;        //设备型号
		std::string series;        //设备系列
	}ObjectAttr;
	typedef Function2<void, const std::string& /*id*/, IDeviceObject*> ExtAttrsChanagedNotifyCallback;
public:
	//id为设备标识 如：mac+gwid+channelid
	IDeviceObject(DeviceObjectType type, const std::string& id,const std::string& fid)
	{
		m_attr.devType = type;
		m_attr.identify = id; 
		m_attr.fatherIdentify = fid;
	}
	virtual ~IDeviceObject() {}

	DeviceObjectType getType() const { return m_attr.devType; }

	std::string getId()const { return m_attr.identify; }//设备标识  如：mac+gwid+channelid

	//获取设备属性
	//属性不用包含DeviceObjectInfo对象中定义的属性
	virtual const ObjectAttr& getAttrs()const { return m_attr; }

	//更新名称
	virtual bool setName(const std::string& name) 
	{
		m_attr.name = name; 
		return true; 
	}

	virtual bool updateExtAttrs(const std::string& key,const Json::Value& val)
	{
		Guard locker(m_mutex);
		m_extAttr[key] = val;

		return true;
	}

	//获取设备属性
	//属性不用包含DeviceObjectInfo对象中定义的属性
	virtual std::map<std::string, Json::Value> getExtAttrs()
	{
		Guard locker(m_mutex);
		return m_extAttr;
	}

	//注册属性变更监听
	//属性不用包含DeviceObjectInfo对象中定义的属性
	//每次需要将全部属性返回
	virtual bool registeAttrsChangedCallback(const ExtAttrsChanagedNotifyCallback& callback)
	{
		m_attrsChangedCallback = callback;
		return true;
	}

	virtual void updateState(uint32_t state) = 0;
protected:
	void onExtAttrChanged()
	{
		m_attrsChangedCallback(m_attr.identify, this);
	}
protected:
	Mutex					m_mutex;
	ObjectAttr				m_attr;
	std::map<std::string, Json::Value>  m_extAttr;
	ExtAttrsChanagedNotifyCallback m_attrsChangedCallback;
};

}
}

#endif //__ABSTUCT_DEVICEOBJ_H__
