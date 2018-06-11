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


//工厂，定义该类型及支持设备类型
class ABSTRUCT_API Driver
{
public:
	//构造析构函数
	Driver() {}
	virtual ~Driver() {}

	//手动添加一个设备 
	virtual bool addDevice(const std::list<Json::Value>& devCfg) = 0;

	//获取该驱动模块的版本号
	virtual AppVersion getVersion() const = 0;

	//扫描设备支持的实物对象列表
	virtual bool scanDeviceList(std::list<shared_ptr<IDeviceObject>>& objList) = 0;

	//更新设备信息属性
	virtual bool updateDeviceConfig(const std::list<Json::Value>& devcfgs) = 0;

	//定时器刷新接口，每一秒刷新一次
	virtual void onPoolTimerProc() = 0;
};


}
}

//驱动模块总输出接口，该接口创建驱动对象，驱动对象指针需要外部使用者是否
extern "C" ABSTRUCT_API Public::Abstruct::Driver*  buildDriver();

#endif //__ABSTRUCT_FACTORY_H__
