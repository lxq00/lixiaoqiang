#ifndef __BLIND_H__
#define __BLIND_H__
#include "Abstruct/Defs.h"
#include "Abstruct/DeviceObjct.h"

namespace Public{
namespace Abstruct {

//�����ӿڶ���
class ABSTRUCT_API Blind:public IDeviceObject
{
public:
	Blind(const std::string& id, const std::string& fid) :IDeviceObject(DEVOBJECTTYPE_BLIND,id, fid) { }
	virtual ~Blind() {}

	//�򿪴���
	virtual bool turnOn() = 0;

	//ֹͣ����
	virtual bool stop() = 0;

	//�رմ���
	virtual bool turnOff() = 0;

	//���ô���λ�ðٷֱȣ�0-100
	virtual bool turnOnByPosition(uint8_t position) { return false; }

	//���ô����Ƕȣ�0-90
	virtual bool turnOnByAngle(uint8_t position) { return false; }

	//�÷�
	//��ǰΪ�ص�ʱ���л�Ϊ��״̬����ǰΪ����״̬�л�Ϊ�ص�״̬
	virtual bool toggle() = 0;

};

}
}

#endif //__BLIND_H__
