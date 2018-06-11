#ifndef __BLIND_H__
#define __BLIND_H__
#include "Abstruct/Defs.h"
#include "Abstruct/DeviceObjct.h"

namespace Public{
namespace Abstruct {

//窗帘接口定义
class ABSTRUCT_API Blind:public IDeviceObject
{
public:
	Blind(const std::string& id, const std::string& fid) :IDeviceObject(DEVOBJECTTYPE_BLIND,id, fid) { }
	virtual ~Blind() {}

	//打开窗帘
	virtual bool turnOn() = 0;

	//停止工作
	virtual bool stop() = 0;

	//关闭窗帘
	virtual bool turnOff() = 0;

	//设置窗帘位置百分比，0-100
	virtual bool turnOnByPosition(uint8_t position) { return false; }

	//设置窗帘角度，0-90
	virtual bool turnOnByAngle(uint8_t position) { return false; }

	//置反
	//当前为关的时候切换为开状态，当前为开的状态切换为关的状态
	virtual bool toggle() = 0;

};

}
}

#endif //__BLIND_H__
