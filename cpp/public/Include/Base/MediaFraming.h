#ifndef __XMS_MEDIAFRAMING_H__
#define __XMS_MEDIAFRAMING_H__
#include "Base/Defs.h"
#include "Base/MediaPackage.h"
#include "Base/DynamicMemPool.h"
#include "Base/Func.h"
#include "Base/IntTypes.h"

namespace Xunmei{
namespace Base{

///�ö���֧�ֶ��̵߳���
class BASE_API MediaFraming
{
	struct MediaFramingInternal;
public:
	///ע���ص�MediaPackage*�����ⲿ����ɾ��
	typedef Function1<void,const MediaPackage*>  mediaCallback;
	typedef Function1<void,const shared_ptr<MediaPackage>& >  mediaCallbackEx;
public:
	//maxsize��������
	MediaFraming(const mediaCallback& callback = NULL,uint32_t maxsize = 256*1024,IMallcFreeMemObjcPtr* ptr = GDynamicMemObjcPtr);
	MediaFraming(const mediaCallbackEx& callback,IMallcFreeMemObjcPtr* ptr = GDynamicMemObjcPtr);

	~MediaFraming();

	char* allocBuffer(uint32_t& size);

	//�ýӿڵ�����ͨ��callback �ص�����
	void addBufferPos(uint32_t size);
	
	char* getUnusedData(uint32_t& size);

	//�������ӿڱ���ƥ��ʹ��
	void addRecvDataSize(uint32_t size);
	MediaPackage* getMediaPacket();

	void clearBuffer();
private:
	MediaFramingInternal*				internal;
};


};
};


#endif //__XMS_MEDIAFRAMING_H__
