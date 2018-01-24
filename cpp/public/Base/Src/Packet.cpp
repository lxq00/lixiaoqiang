//
//  Copyright (c)1998-2012, Chongqing Xunmei Technology
//  All Rights Reserved.
//
//	Description:
//	$Id: Packet.cpp 255 2014-01-23 03:28:32Z lixiaoqiang $
//

////////////////////////////////////////////////////////////////////////////////////
// Packet
////////////////////////////////////////////////////////////////////////////////////
#include "Base/IntTypes.h"
#include "Base/Packet.h"
#include "Base/PrintLog.h"
#include "Base/StaticMemPool.h"
#include <memory>
#include <string.h>
namespace Xunmei{
namespace Base{

///额外安全预留长度/包含额外数据的大小
#define EXTRASAVELEN	128

struct Packet::PacketInternal
{
	uint8_t*				buffer;						//包缓冲区首地址
	uint32_t				packetSize;					//包总大小
	uint32_t				dataLegnth;					//数据长度
	uint32_t				buffersize;					//缓冲区总大小
};

Packet::Packet(IMallcFreeMemObjcPtr* ptr) :internal(NULL),memptr(ptr)
{
}

Packet::Packet(IMallcFreeMemObjcPtr* ptr,uint32_t size) : internal(NULL),memptr(ptr)
{
	reinit(size);
}
Packet::~Packet()
{
	removePacket();
}
uint8_t* Packet::getBuffer() const
{
	return isValid() ? internal->buffer : NULL;
}

uint32_t Packet::size() const
{
	return isValid() ? internal->packetSize : 0;
}

void Packet::removePacket()
{
	if (NULL != internal)
	{
		if(memptr != NULL)
		{
			memptr->Free(internal->buffer);
		}
		delete internal;
	}
	internal = NULL;
}

bool Packet::isValid() const
{
	return (NULL != internal && NULL != internal->buffer) ? true : false;
}

void Packet::reinit(uint32_t size)
{
	if(internal != NULL && internal->buffersize < size)
	{
		removePacket();
	}
	if(internal == NULL)
	{
		internal = new PacketInternal();
	}
	if (NULL != internal)
	{
		if(memptr != NULL)
		{
			internal->buffer = (uint8_t*)memptr->Malloc(size + EXTRASAVELEN,internal->buffersize);
		}
		if(internal->buffer != NULL)
		{
			memset(internal->buffer,0,size);
		}		
		internal->dataLegnth = 0;
		internal->packetSize = size;
	}
}

char* Packet::remalloc(uint32_t& size)
{
	if(internal != NULL && internal->buffer != NULL && internal->buffersize >= size)
	{
		size = internal->buffersize;
		return (char*)internal->buffer;
	}

	char* newaddr = NULL;
	if(memptr != NULL)
	{
		newaddr = (char*)memptr->Malloc(size + EXTRASAVELEN,size);
	}

	return newaddr;
}
bool Packet::resetBuffer(uint8_t* buffer,uint32_t size,uint32_t maxSize)
{
	if(internal != NULL && internal->buffer == buffer)
	{
		internal->packetSize = size;
		return true;
	}
	if(internal != NULL)
	{
		if(memptr != NULL)
		{
			memptr->Free(internal->buffer);
		}
	}
	if(internal == NULL)
	{
		internal = new PacketInternal();
	}
	internal->buffer = buffer;
	internal->buffersize = maxSize;
	internal->packetSize = size;
	
	return true;
}
bool Packet::addDataLength(uint32_t len)
{
	if(internal == NULL || internal->buffer == NULL || internal->packetSize < len + internal->dataLegnth)
	{
		return false;
	}

	internal->dataLegnth += len;

	return true;
}

int Packet::getDataLen() const
{
	if(internal == NULL)
	{
		return 0;
	}

	return internal->dataLegnth;
}

} // namespace Base
} // namespace Xunmei

