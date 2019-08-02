#ifndef __CIRCLE_BUFFER_H__
#define __CIRCLE_BUFFER_H__
//
//  Copyright (c)1998-2012, Public Technology
//  All Rights Reserved.
//
//	Description:
//	$Id: ByteOrder.h 3 2013-01-21 06:57:38Z  $
//

#include "Base/IntTypes.h"
#include "Base/Defs.h"
#include "Base/String.h"
namespace Public {
namespace Base {

//环形buffer,线性不安全
class BASE_API CircleBuffer
{
public:
	struct BufferInfo
	{
		const char* bufferAddr;
		uint32_t	bufferLen;
	};
public:
	CircleBuffer(uint32_t buffersize);
	~CircleBuffer();


	//数据长度
	uint32_t dataLenght() const;
	
	//该函数会造成消费
	//return false表示无数据,获取指定长度 消费者数据，当length = -1 表示所有数据
	bool consumeBuffer(uint32_t pos, std::vector<BufferInfo>& buffer,uint32_t length = -1);

	//该函数会造成消费
	//获取一行数据，\r\n或 \r或\n为结束表示，return false表示失败，
	bool consumeLine(std::string& str,const std::string& flag="\r\n");

	//获取消费者地址 
	const char* getConsumeAddr() const;
	//后去消费者地址数据长度 
	uint32_t getConsumeLength() const;

	//该函数会造成数据消费
	//设置已消费的数据长度，return false 表示失败
	bool setConsumeLength(uint32_t length);


	//获取生产者所在的地址
	char* getProductionAddr() const;
	//获取生产者能生成的长度 
	uint32_t getProductionLength() const;

	//设置数据生成的长度
	bool setProductionLength(uint32_t length);

	//生成数据
	bool production(const char* str);
	bool production(const char* str, uint32_t len);
	bool production(const std::string& str);
	bool production(const String& str);

	//一下read函数不会产生消费
	//pos必须小于dataLenght，否则返回0，从结果无法判断成功失败
	char readChar(uint32_t pos) const;
	//获取数据，不会消耗，return flash 表示失败
	bool readBuffer(uint32_t pos,void* dst, uint32_t length) const;
	bool readBuffer(uint32_t pos, std::vector<BufferInfo>& buffer, uint32_t length = -1);

	//同getchar同意义
	char operator[](uint32_t pos) const;
private:
	struct CircleBufferInternal;
	CircleBufferInternal* internal;
};

}
}


#endif
