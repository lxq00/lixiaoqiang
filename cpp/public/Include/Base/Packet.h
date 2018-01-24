//
//  Copyright (c)1998-2012, Xunmei Technology
//  All Rights Reserved.
//
//	Description:
//	$Id: Packet.h 255 2014-01-23 03:28:32Z lixiaoqiang $

#ifndef __BASE_PACKET_H__
#define __BASE_PACKET_H__
		
#include "Base/IntTypes.h"
#include "Base/Func.h"
#include "Defs.h"
#include "Base/DynamicMemPool.h"

namespace Xunmei {
namespace Base {

////////////////////////////////////////////////////////////////////////////////
/// \class Packet
/// \brief 数据包缓存管理
class BASE_API Packet
{
	struct PacketInternal;
public:

	/// 缺省构造函数,生成一个无效的包,需要赋值才能使用
	Packet(IMallcFreeMemObjcPtr* ptr);

	/// 构造函数,生成一个可以使用的包
	/// \param dataSize 	[in]  	申请的包包含的有效数据缓冲的字节数
	Packet(IMallcFreeMemObjcPtr* ptr,uint32_t size);

	/// 析构函数,完成对包数据引用计数递减
	virtual ~Packet();

	/// 判断是否为有效包
	virtual bool isValid(void) const;

	/// 得到包缓冲的起始位置的指针
	/// \retval 包的首地址
	uint8_t* getBuffer(void) const;

	uint32_t size(void) const;
protected:
	void reinit(uint32_t size);
	void removePacket(void);
	char* remalloc(uint32_t& size);
	bool resetBuffer(uint8_t* buffer,uint32_t size,uint32_t maxSize);
	bool addDataLength(uint32_t len);
	int getDataLen() const;
private:
	PacketInternal*			internal;
	IMallcFreeMemObjcPtr*	memptr;
};

} // namespace Base
} // namespace Xunmei

#endif	// __BASE_PACKET_H__

