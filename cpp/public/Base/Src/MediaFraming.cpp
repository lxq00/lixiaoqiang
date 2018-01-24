#include <string.h>
#include "Base/MediaFraming.h"
#include "Base/BaseTemplate.h"
#include "Base/PrintLog.h"
#include "Base/Mutex.h"
#include "Base/Guard.h"
namespace Xunmei{
namespace Base{

#define DEFAULTMEDIAFRAMINGSIZE		1024*128
#define MAXMEDIAFRAMESIZE			1024*1024*5

struct MediaFraming::MediaFramingInternal
{
	MediaFramingInternal(IMallcFreeMemObjcPtr* ptr):mediaBufferMaxSize(0),mediaBufferStartAddr(NULL)
		,unUsedMediaBufferSize(0),memptr(ptr)
	{
		uint32_t realsize = 0;
		mediaBufferStartAddr = (char*)memptr->Malloc(DEFAULTMEDIAFRAMINGSIZE,realsize);

		if(mediaBufferStartAddr == NULL)
		{
			return;
		}
		mediaBufferMaxSize = realsize;
	}
	~MediaFramingInternal()
	{
		if(memptr != NULL)
		{
			memptr->Free(mediaBufferStartAddr);
		}
	}

	bool remallocSpaceSize(uint32_t mediasize,const char*& bufferstart)
	{
		//����Ƶ��С��������buffer��С�ˣ���Ҫ���¼���ռ��С��
		uint32_t needMediaBufferSize = min((uint32_t) MAXMEDIAFRAMESIZE , mediasize * 2);
		//������Ҫ����Ƶ�ռ��Сȷʵ���ڵ�ǰ�Ĵ�С,���·���
		if(needMediaBufferSize <= mediaBufferMaxSize)
		{
			return false;
		}
	
		//���·���ռ�
		uint32_t realsize = 0;
		char* newMediaBufferAddr = (char*)memptr->Malloc(needMediaBufferSize,realsize);
		if(newMediaBufferAddr == NULL)
		{
			return false;
		}
		//����ԭ�е�����
		memcpy(newMediaBufferAddr, bufferstart, unUsedMediaBufferSize);

		//�ͷ�����
		memptr->Free(mediaBufferStartAddr);

		//�ָ�ԭ��������
		mediaBufferMaxSize = realsize;
		bufferstart = mediaBufferStartAddr = newMediaBufferAddr;

		return true;
	}

	unsigned int 						mediaBufferMaxSize;
	char* 								mediaBufferStartAddr;
	unsigned int						unUsedMediaBufferSize;
	mediaCallback						callback;
	mediaCallbackEx						calblackEx;
	IMallcFreeMemObjcPtr*				memptr; 
	Mutex								mutex;
};


MediaFraming::MediaFraming(const mediaCallback& _callback,uint32_t maxsize,IMallcFreeMemObjcPtr* ptr):internal(NULL)
{
	internal = new MediaFramingInternal(ptr);
	internal->callback = _callback;
}
MediaFraming::MediaFraming(const mediaCallbackEx& callback,IMallcFreeMemObjcPtr* ptr):internal(NULL)
{
	internal = new MediaFramingInternal(ptr);
	internal->calblackEx = callback;
}
MediaFraming::~MediaFraming()
{
	if(internal->memptr != NULL && internal->mediaBufferStartAddr != NULL)
	{
		internal->memptr->Free(internal->mediaBufferStartAddr);
		internal->mediaBufferStartAddr = NULL;
	}
	SAFE_DELETE(internal);
}

char* MediaFraming::allocBuffer(uint32_t& size)
{
	Guard locker(internal->mutex);

	if (internal->mediaBufferMaxSize == internal->unUsedMediaBufferSize)
	{
		const char* tmp = internal->mediaBufferStartAddr;
		if(!internal->remallocSpaceSize(internal->mediaBufferMaxSize,tmp))
		{
			internal->unUsedMediaBufferSize = 0;
		}
	}

	size = internal->mediaBufferMaxSize - internal->unUsedMediaBufferSize;

	return internal->mediaBufferStartAddr + internal->unUsedMediaBufferSize;
}

void MediaFraming::clearBuffer()
{
	Guard locker(internal->mutex);

	internal->unUsedMediaBufferSize = 0;
}

void MediaFraming::addBufferPos(uint32_t size)
{
	Guard locker(internal->mutex);

	internal->unUsedMediaBufferSize += size;
	const char* mediaStartAddr = internal->mediaBufferStartAddr;
	while (internal->unUsedMediaBufferSize > 0)
	{
		if(internal->unUsedMediaBufferSize < MEDIAHEADER_LENGTH)
		{
			break;
		}
		shared_ptr<MediaPackage> media(new(std::nothrow) MediaPackage(mediaStartAddr,internal->memptr));
		if (media == NULL || !media->isValid())
		{
			if(internal->unUsedMediaBufferSize > MEDIAHEADER_LENGTH)
			{
				mediaStartAddr ++;
				internal->unUsedMediaBufferSize --;
				continue;
			}
			internal->unUsedMediaBufferSize = 0;
			break;
		}

		//����media��С���ж�̬����
		if(media->size() > internal->mediaBufferMaxSize)
		{
			if(internal->remallocSpaceSize(media->size(),mediaStartAddr))
			{
				continue;
			}
			//�ռ䲻���ˣ�����û�н����¿ռ���䣬�������������ǽ����������ģ�������Ҫ������
			mediaStartAddr ++;
			internal->unUsedMediaBufferSize --;
			continue;
		}

		uint32_t remainSize = media->getRemainSize();
		uint32_t mediabuffertotalsize = remainSize + media->getHeaderOffset();
		if(mediabuffertotalsize > internal->unUsedMediaBufferSize)
		{
			break;
		}
		media->addRemainData(mediaStartAddr + media->getHeaderOffset(),media->getRemainSize());
		if(!internal->callback.empty())
		{
			internal->callback(media.get());
		}
		if(!internal->calblackEx.empty())
		{
			internal->calblackEx(media);
		}

		mediaStartAddr += mediabuffertotalsize;
		internal->unUsedMediaBufferSize -= mediabuffertotalsize;
	}

	if(internal->unUsedMediaBufferSize > 0 && mediaStartAddr != internal->mediaBufferStartAddr)
	{
		memcpy(internal->mediaBufferStartAddr,mediaStartAddr,internal->unUsedMediaBufferSize);
	}
}

char* MediaFraming::getUnusedData(uint32_t& size)
{
	Guard locker(internal->mutex);

	size = internal->unUsedMediaBufferSize;

	return internal->mediaBufferStartAddr;
}

void MediaFraming::addRecvDataSize(uint32_t size)
{
	Guard locker(internal->mutex);

	internal->unUsedMediaBufferSize += size;
}
MediaPackage* MediaFraming::getMediaPacket()
{
	Guard locker(internal->mutex);

	const char* mediaStartAddr = internal->mediaBufferStartAddr;
	while (internal->unUsedMediaBufferSize > 0)
	{
		if(internal->unUsedMediaBufferSize < MEDIAHEADER_LENGTH)
		{
			break;
		}
		MediaPackage* media = new(std::nothrow) MediaPackage(mediaStartAddr,internal->memptr);
		if (media == NULL || !media->isValid())
		{
			SAFE_DELETE(media);
			if(internal->unUsedMediaBufferSize > MEDIAHEADER_LENGTH)
			{
				mediaStartAddr ++;
				internal->unUsedMediaBufferSize --;
				continue;
			}
			internal->unUsedMediaBufferSize = 0;
			return NULL;
		}

		unsigned int mediaLen = media->size();
		if (mediaLen > internal->unUsedMediaBufferSize)
		{
			SAFE_DELETE(media);
			break;
		}
		media->addRemainData(mediaStartAddr + media->getHeaderOffset(),media->getRemainSize());
		
		mediaStartAddr += media->size();
		internal->unUsedMediaBufferSize -= media->size();

		if(internal->unUsedMediaBufferSize > 0 && mediaStartAddr != internal->mediaBufferStartAddr)
		{
			memmove(internal->mediaBufferStartAddr,mediaStartAddr,internal->unUsedMediaBufferSize);
		}

		return media;
	}
	if(internal->unUsedMediaBufferSize > 0 && mediaStartAddr != internal->mediaBufferStartAddr)
	{
		memmove(internal->mediaBufferStartAddr,mediaStartAddr,internal->unUsedMediaBufferSize);
	}

	return NULL;
}

};
};

