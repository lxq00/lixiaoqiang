#ifndef __SWITCHLIGHT_MODULE_H__
#define __SWITCHLIGHT_MODULE_H__

#include "Abstruct/Defs.h"
#include "Abstruct/DeviceObjct.h"

namespace Public{
namespace Abstruct {

//开关灯接口定义
class ABSTRUCT_API Module:public IDeviceObject
{
public:
	Module(const std::string& id, const std::string& fid):IDeviceObject(DEVOBJECTTYPE_MODULE,id,fid) {}
	virtual ~Module() {}
};



}
}

#endif //__SWITCHLIGHT_MODULE_H__
