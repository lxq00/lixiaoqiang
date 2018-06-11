#ifndef __ABSTRUCT_DRIVER_H__
#define __ABSTRUCT_DRIVER_H__
#include "Abstruct/Defs.h"
#include "Abstruct/DeviceObjct.h"
#include "Abstruct/LightSwitch.h"
#include "Abstruct/Gateway.h"
#include "Abstruct/Module.h"
#include "Abstruct/IO.h"
#include "Abstruct/LightDimmer.h"
#include "Abstruct/LightRGB.h"
#include "Abstruct/Blind.h"
namespace Public {
namespace Abstruct {


//��������������ͼ�֧���豸����
class ABSTRUCT_API Driver
{
public:
	//������������
	Driver() {}
	virtual ~Driver() {}

	//�ֶ����һ���豸 
	virtual bool addDevice(const std::list<Json::Value>& devCfg) = 0;

	//��ȡ������ģ��İ汾��
	virtual AppVersion getVersion() const = 0;

	//ɨ���豸֧�ֵ�ʵ������б�
	virtual bool scanDeviceList(std::list<shared_ptr<IDeviceObject>>& objList) = 0;

	//�����豸��Ϣ����
	virtual bool updateDeviceConfig(const std::list<Json::Value>& devcfgs) = 0;

	//��ʱ��ˢ�½ӿڣ�ÿһ��ˢ��һ��
	virtual void onPoolTimerProc() = 0;
};


}
}

//����ģ��������ӿڣ��ýӿڴ�������������������ָ����Ҫ�ⲿʹ�����Ƿ�
extern "C" ABSTRUCT_API Public::Abstruct::Driver*  buildDriver();

#endif //__ABSTRUCT_FACTORY_H__
