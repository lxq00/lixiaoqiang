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

//����RTSP�ڲ��㿽�����ڴ�ռ�
class RTSP_API RTSPBuffer
{
public:
	RTSPBuffer();

	//�ڲ�Ҫ��һ�ο���
	RTSPBuffer(const char* str);
	RTSPBuffer(const char* str, size_t size);
	RTSPBuffer(const std::string& str);
	RTSPBuffer(const String& str);

	//��buffer - str.c_str() > RTSPBUFFERHEADSPACE ,�㿽��
	RTSPBuffer(const std::string& str, const char* buffer, size_t size);
	RTSPBuffer(const String& str, const char* buffer, size_t size);
	RTSPBuffer(const RTSPBuffer& str, const char* buffer, size_t size);

	//��������
	RTSPBuffer(const RTSPBuffer& buffer);
	~RTSPBuffer();

	//����ռ䣬���Զ�Ԥ��RTSPBUFFERHEADSPACE�ĳ���
	char* alloc(size_t size);
	void resize(size_t size);

	//��ǰ�ռ�ĵ�ַ
	const char* buffer() const;
	size_t size() const;

	//��ʼ����
	const String& dataptr()const;

	RTSPBuffer& operator=(const RTSPBuffer& buffer);
private:
	struct RTSPBufferInternal;
	RTSPBufferInternal* internal;
};

}
}