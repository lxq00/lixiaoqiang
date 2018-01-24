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
		//当视频大小大于整个buffer大小了，需要重新计算空间大小了
		uint32_t needMediaBufferSize = min((uint32_t) MAXMEDIAFRAMESIZE , mediasize * 2);
		//当新需要的视频空间大小确实大于当前的大小,重新分配
		if(needMediaBufferSize <= mediaBufferMaxSize)
		{
			return false;
		}
	
		//重新分配空间
		uint32_t realsize = 0;
		char* newMediaBufferAddr = (char*)memptr->Malloc(needMediaBufferSize,realsize);
		if(newMediaBufferAddr == NULL)
		{
			return false;
		}
		//备份原有的数据
		memcpy(newMediaBufferAddr, bufferstart, unUsedMediaBufferSize);

		//释放数据
		memptr->Free(mediaBufferStartAddr);

		//恢复原来的数据
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

		//根据media大小进行动态分配
		if(media->size() > internal->mediaBufferMaxSize)
		{
			if(internal->remallocSpaceSize(media->size(),mediaStartAddr))
			{
				continue;
			}
			//空间不够了，但又没有进行新空间分配，这样整个数据是解析不出来的，所有需要丢数据
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

