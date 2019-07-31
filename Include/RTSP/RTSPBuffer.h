//
//  Copyright (c)1998-2012, Public Technology
//  All Rights Reserved.
//
//	Description:
//	$Id: File.h 252 2013-12-18 04:40:28Z  $
//


#pragma once
#include "Defs.h"
#include "Base/Base.h"
#include "Network/Network.h"
using namespace Public::Base;
using namespace Public::Network;

namespace Public {
namespace RTSP {

#define RTSPBUFFERHEADSPACE 20 

//用于RTSP内部零拷贝的内存空间
class RTSP_API RTSPBuffer
{
public:
	RTSPBuffer();

	//内部要做一次拷贝
	RTSPBuffer(const char* str);
	RTSPBuffer(const char* str, size_t size);
	RTSPBuffer(const std::string& str);
	RTSPBuffer(const String& str);

	//当buffer - str.c_str() > RTSPBUFFERHEADSPACE ,零拷贝
	RTSPBuffer(const std::string& str, const char* buffer, size_t size);
	RTSPBuffer(const String& str, const char* buffer, size_t size);
	RTSPBuffer(const RTSPBuffer& str, const char* buffer, size_t size);

	//拷贝构造
	RTSPBuffer(const RTSPBuffer& buffer);
	~RTSPBuffer();

	//分配空间，会自动预留RTSPBUFFERHEADSPACE的长度
	char* alloc(size_t size);
	void resize(size_t size);

	//当前空间的地址
	const char* buffer() const;
	size_t size() const;

	//起始数据
	const String& dataptr()const;

	RTSPBuffer& operator=(const RTSPBuffer& buffer);
private:
	struct RTSPBufferInternal;
	RTSPBufferInternal* internal;
};

}
}