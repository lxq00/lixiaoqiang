#ifndef __XMS_MEDIAFRAMING_H__
#define __XMS_MEDIAFRAMING_H__
#include "Base/Defs.h"
#include "Base/MediaPackage.h"
#include "Base/DynamicMemPool.h"
#include "Base/Func.h"
#include "Base/IntTypes.h"

namespace Xunmei{
namespace Base{

///该对象不支持多线程调用
class BASE_API MediaFraming
{
	struct MediaFramingInternal;
public:
	///注：回调MediaPackage*参数外部不能删除
	typedef Function1<void,const MediaPackage*>  mediaCallback;
	typedef Function1<void,const shared_ptr<MediaPackage>& >  mediaCallbackEx;
public:
	//maxsize参数弃用
	MediaFraming(const mediaCallback& callback = NULL,uint32_t maxsize = 256*1024,IMallcFreeMemObjcPtr* ptr = GDynamicMemObjcPtr);
	MediaFraming(const mediaCallbackEx& callback,IMallcFreeMemObjcPtr* ptr = GDynamicMemObjcPtr);

	~MediaFraming();

	char* allocBuffer(uint32_t& size);

	//该接口的数据通过callback 回调返回
	void addBufferPos(uint32_t size);
	
	char* getUnusedData(uint32_t& size);

	//这两个接口必须匹配使用
	void addRecvDataSize(uint32_t size);
	MediaPackage* getMediaPacket();

	void clearBuffer();
private:
	MediaFramingInternal*				internal;
};


};
};


#endif //__XMS_MEDIAFRAMING_H__
