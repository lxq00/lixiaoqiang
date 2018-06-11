#ifndef __SWITCHLIGHT_H__
#define __SWITCHLIGHT_H__

#include "Abstruct/Defs.h"
#include "Abstruct/DeviceObjct.h"

namespace Public{
namespace Abstruct {

//���صƽӿڶ���
class ABSTRUCT_API LightSwith:public IDeviceObject
{
public:
	LightSwith(const std::string& id, const std::string& fid):IDeviceObject(DEVOBJECTTYPE_LIGHTSWITH,id,fid) {}
	virtual ~LightSwith() {}

	//�򿪵�
	virtual bool turnOn() = 0;

	//�رյ�
	virtual bool turnOff() = 0;

	//�÷�
	//��ǰΪ�ص�ʱ���л�Ϊ��״̬����ǰΪ����״̬�л�Ϊ�ص�״̬
	virtual bool toggle() = 0;

	
};



}
}

#endif //__SWITCHLIGHT_H__
