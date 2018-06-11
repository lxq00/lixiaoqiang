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
		DeviceObjectType devType;	//�豸�ͺ�
		std::string identify;		//�豸��ʶ  �磺mac+gwid+channelid
		std::string fatherIdentify; //����ʶ
		std::string name;          //�豸����
		std::string models;        //�豸�ͺ�
		std::string series;        //�豸ϵ��
	}ObjectAttr;
	typedef Function2<void, const std::string& /*id*/, IDeviceObject*> ExtAttrsChanagedNotifyCallback;
public:
	//idΪ�豸��ʶ �磺mac+gwid+channelid
	IDeviceObject(DeviceObjectType type, const std::string& id,const std::string& fid)
	{
		m_attr.devType = type;
		m_attr.identify = id; 
		m_attr.fatherIdentify = fid;
	}
	virtual ~IDeviceObject() {}

	DeviceObjectType getType() const { return m_attr.devType; }

	std::string getId()const { return m_attr.identify; }//�豸��ʶ  �磺mac+gwid+channelid

	//��ȡ�豸����
	//���Բ��ð���DeviceObjectInfo�����ж��������
	virtual const ObjectAttr& getAttrs()const { return m_attr; }

	//��������
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

	//��ȡ�豸����
	//���Բ��ð���DeviceObjectInfo�����ж��������
	virtual std::map<std::string, Json::Value> getExtAttrs()
	{
		Guard locker(m_mutex);
		return m_extAttr;
	}

	//ע�����Ա������
	//���Բ��ð���DeviceObjectInfo�����ж��������
	//ÿ����Ҫ��ȫ�����Է���
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
