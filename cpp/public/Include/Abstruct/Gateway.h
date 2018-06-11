#ifndef __SWITCHLIGHT_GATEWAY_H__
#define __SWITCHLIGHT_GATEWAY_H__

#include "Abstruct/Defs.h"
#include "Abstruct/DeviceObjct.h"

namespace Public{
namespace Abstruct {

//开关灯接口定义
class ABSTRUCT_API Gateway:public IDeviceObject
{
public:
	Gateway(const std::string& id, const std::string& fid):IDeviceObject(DEVOBJECTTYPE_GATEWAY,id,fid) {}
	virtual ~Gateway() {}	
};



}
}

#endif //__SWITCHLIGHT_GATEWAY_H__
