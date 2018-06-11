#ifndef __DEVICE_IO_H__
#define __DEVICE_IO_H__
#include "Abstruct/Defs.h"
#include "Abstruct/DeviceObjct.h"
#include "LightSwitch.h"

namespace Public{
namespace Abstruct {

//IO接口定义
class ABSTRUCT_API IO:public IDeviceObject
{
public:
	IO(const std::string& id, const std::string& fid) :IDeviceObject(DEVOBJECTTYPE_IO,id, fid) {}
	virtual ~IO() {}

};

}
}

#endif //__DEVICE_IO_H__
