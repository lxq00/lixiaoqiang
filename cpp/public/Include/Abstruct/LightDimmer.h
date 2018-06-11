#ifndef __DIMMERLIGHT_H__
#define __DIMMERLIGHT_H__

#include "Abstruct/Defs.h"
#include "Abstruct/DeviceObjct.h"
#include "LightSwitch.h"

namespace Public{
namespace Abstruct {

//开关灯接口定义
class ABSTRUCT_API LightDimmer:public LightSwith
{
public:
	LightDimmer(const std::string& id, const std::string& fid) :LightSwith(id, fid) { m_attr.devType = DEVOBJECTTYPE_LIGHTDIMMER; }
	virtual ~LightDimmer() {}

	//设置灯光亮度值，
	// brightness 亮度 0-100 百分比
	// traveltime 调节时间，单位秒,0表示马上执行
	virtual bool turnOn(uint8_t brightness) = 0;
};



}
}

#endif //__SWITCHLIGHT_H__
