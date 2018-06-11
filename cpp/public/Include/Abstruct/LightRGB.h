#ifndef __SWITCHLIGHTRGB_H__
#define __SWITCHLIGHTRGB_H__
#include "Abstruct/Defs.h"
#include "Abstruct/DeviceObjct.h"
#include "LightSwitch.h"

namespace Public{
namespace Abstruct {

//开关灯接口定义
class ABSTRUCT_API LightRGB:public LightSwith
{
public:
	LightRGB(const std::string& id, const std::string& fid) :LightSwith(id, fid) { m_attr.devType = DEVOBJECTTYPE_LIGHTRGB; }
	virtual ~LightRGB() {}

	//设置RGB调色灯颜色，范围：0 -> 255
	virtual bool setColor(int red, int green, int blue) = 0;
};

}
}

#endif //__SWITCHLIGHTRGB_H__
