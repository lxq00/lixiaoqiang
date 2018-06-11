#ifndef __DIMMERLIGHT_H__
#define __DIMMERLIGHT_H__

#include "Abstruct/Defs.h"
#include "Abstruct/DeviceObjct.h"
#include "LightSwitch.h"

namespace Public{
namespace Abstruct {

//���صƽӿڶ���
class ABSTRUCT_API LightDimmer:public LightSwith
{
public:
	LightDimmer(const std::string& id, const std::string& fid) :LightSwith(id, fid) { m_attr.devType = DEVOBJECTTYPE_LIGHTDIMMER; }
	virtual ~LightDimmer() {}

	//���õƹ�����ֵ��
	// brightness ���� 0-100 �ٷֱ�
	// traveltime ����ʱ�䣬��λ��,0��ʾ����ִ��
	virtual bool turnOn(uint8_t brightness) = 0;
};



}
}

#endif //__SWITCHLIGHT_H__
