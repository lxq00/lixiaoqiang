//
//  Copyright (c)1998-2012, Public Technology
//  All Rights Reserved.
//
//	Description:
//	$Id: ByteOrder.h 3 2013-01-21 06:57:38Z  $
//

#include "Base/CircleBuffer.h"
namespace Public {
namespace Base {

struct CircleBuffer::CircleBufferInternal
{
	char*		bufferAddr;	//buffer地址
	uint32_t	bufferLen;	//buffer长度

	uint32_t	dataLen;	//数据长度 
	uint32_t	consumePos;	//消费者位置
	uint32_t	productionPos;	//生产者位置
};

CircleBuffer::CircleBuffer(uint32_t buffersize)
{
	internal = new CircleBufferInternal;
	internal->bufferLen = buffersize;
	internal->bufferAddr = new char[buffersize];
	internal->dataLen = 0;
	internal->consumePos = 0;
	internal->productionPos = 0;
}
CircleBuffer::~CircleBuffer()
{
	SAFE_DELETEARRAY(internal->bufferAddr);
	SAFE_DELETE(internal);
}

uint32_t CircleBuffer::dataLenght() const
{
	return internal->dataLen;
}
	
bool CircleBuffer::consumeBuffer(uint32_t pos, std::vector<BufferInfo>& buffer, uint32_t length)
{
	if (!readBuffer(pos, buffer, length)) return false;

	setConsumeLength(length);

	return true;
}

bool CircleBuffer::consumeLine(std::string& str, const std::string& flag)
{
	const char* flagaddr = flag.c_str();
	uint32_t flaglen = flag.length();
	uint32_t datalen = dataLenght();

	str.reserve(datalen + 10);

	uint32_t readpos = 0;
	bool havefindend = false;
	while (readpos < datalen)
	{
		str.append(1, readChar(readpos));
		if (readpos >= flaglen && memcmp(str.c_str() + readpos - flaglen, flagaddr, flaglen) == 0)
		{
			havefindend = true;
			break;
		}

		readpos++;
	}

	if (havefindend)
	{
		setConsumeLength(readpos);
	}

	return havefindend;
}

const char* CircleBuffer::getConsumeAddr() const
{
	return internal->bufferAddr + internal->consumePos;
}

uint32_t CircleBuffer::getConsumeLength() const
{
	if (internal->dataLen == 0) return 0;

	if (internal->productionPos <= internal->consumePos) return internal->bufferLen - internal->consumePos;
	else return internal->productionPos - internal->consumePos;
}
bool CircleBuffer::setConsumeLength(uint32_t length)
{
	if (length > internal->dataLen) return false;

	internal->dataLen -= length;
	internal->consumePos = (internal->consumePos + length) % internal->bufferLen;


	return true;
}

char* CircleBuffer::getProductionAddr() const
{
	return internal->bufferAddr + internal->productionPos;
}

uint32_t CircleBuffer::getProductionLength() const
{
	if (internal->dataLen == internal->bufferLen) return 0;

	if (internal->productionPos >= internal->consumePos) return internal->bufferLen - internal->productionPos;
	else return internal->consumePos - internal->productionPos;
}

bool CircleBuffer::setProductionLength(uint32_t length)
{
	if (length > internal->bufferLen - internal->dataLen) return false;

	internal->dataLen += length;
	internal->productionPos = (internal->productionPos + length) % internal->bufferLen;

	return true;
}

bool CircleBuffer::production(const char* bufferaddr)
{
	if (bufferaddr == NULL) return false;

	return production(bufferaddr, strlen(bufferaddr));
}
bool CircleBuffer::production(const char* bufferaddr, uint32_t bufferlen)
{
	if (bufferaddr == NULL || bufferlen <= 0) return false;

	if (bufferlen > internal->bufferLen - internal->dataLen) return false;

	while (bufferlen > 0)
	{
		uint32_t usedlen = 0;
		if (internal->productionPos >= internal->consumePos)
		{
			usedlen = min(internal->bufferLen - internal->productionPos, bufferlen);
		}
		else
		{
			usedlen = min(internal->consumePos - internal->productionPos, bufferlen);
		}

		memcpy(internal->bufferAddr + internal->productionPos, bufferaddr, usedlen);

		setProductionLength(usedlen);
		bufferlen -= usedlen;
		bufferaddr += usedlen;
	}

	return true;
}
bool CircleBuffer::production(const std::string& str)
{
	return production(str.c_str(), str.length());
}
bool CircleBuffer::production(const String& str)
{
	return production(str.c_str(), str.length());
}

char CircleBuffer::readChar(uint32_t pos) const
{
	if (pos >= internal->dataLen) return 0;

	uint32_t readpos = (internal->consumePos + pos) % internal->bufferLen;

	return internal->bufferAddr[readpos];
}
bool CircleBuffer::readBuffer(uint32_t pos, void* dstptr, uint32_t length) const
{
	if (pos < 0 || length + pos > internal->dataLen || dstptr == NULL || length <= 0) return false;

	char* buffer = (char*)dstptr;
	uint32_t readpos = (internal->consumePos + pos) % internal->bufferLen;
	while (length > 0)
	{
		uint32_t usedlen = 0;
		if (readpos >= internal->productionPos)
		{
			usedlen = min(internal->bufferLen - readpos, length);
		}
		else
		{
			usedlen = min(internal->productionPos - readpos, length);
		}

		memcpy(buffer, internal->bufferAddr + readpos, usedlen);

		readpos = (readpos + usedlen) % internal->bufferLen;
		
		length -= usedlen;
		buffer += usedlen;
	}

	return true;
}

bool CircleBuffer::readBuffer(uint32_t pos, std::vector<BufferInfo>& buffer, uint32_t length)
{
	if (length == -1) length = internal->dataLen;

	if (internal->dataLen == 0 || pos < 0 || length + pos > internal->dataLen) return false;

	uint32_t readpos = (internal->consumePos + pos) % internal->bufferLen;

	while (length > 0)
	{
		BufferInfo info;
		info.bufferAddr = internal->bufferAddr + readpos;

		//消费者在生产者后面，那么数据只有 生产者-消费者 长度
		if (readpos <= internal->productionPos)
		{
			info.bufferLen = min(length, internal->productionPos - readpos);
		}
		//消费者在生产者前面，那么数据由 buffer-消费者 + 生产者 长度
		else
		{
			info.bufferLen = min(internal->bufferLen - readpos, length);
		}

		buffer.push_back(info);

		readpos = (readpos + info.bufferLen) % internal->bufferLen;

		length -= info.bufferLen;
	}

	return true;
}
char  CircleBuffer::operator[](uint32_t pos) const
{
	return readChar(pos);
}

}
}

