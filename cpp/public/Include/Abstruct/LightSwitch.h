#ifndef __SWITCHLIGHT_H__
#define __SWITCHLIGHT_H__

#include "Abstruct/Defs.h"
#include "Abstruct/DeviceObjct.h"

namespace Public{
namespace Abstruct {

//开关灯接口定义
class ABSTRUCT_API LightSwith:public IDeviceObject
{
public:
	LightSwith(const std::string& id, const std::string& fid):IDeviceObject(DEVOBJECTTYPE_LIGHTSWITH,id,fid) {}
	virtual ~LightSwith() {}

	//打开灯
	virtual bool turnOn() = 0;

	//关闭灯
	virtual bool turnOff() = 0;

	//置反
	//当前为关的时候切换为开状态，当前为开的状态切换为关的状态
	virtual bool toggle() = 0;

	
};



}
}

#endif //__SWITCHLIGHT_H__
