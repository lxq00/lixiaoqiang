//
//  Copyright (c)1998-2012, Public Technology
//  All Rights Reserved.
//
//	Description:
//	$Id: File.h 252 2013-12-18 04:40:28Z  $
//
#include "RTSP/RTSPBuffer.h"

namespace Public {
namespace RTSP {

struct RTSPBuffer::RTSPBufferInternal
{
	String		data;
	const char* buffer;
	size_t		size;

	RTSPBufferInternal() :buffer(NULL), size(0) {}

	char* alloc(size_t _size)
	{
		data.alloc(_size + RTSPBUFFERHEADSPACE);
		size = 0;
		buffer = data.c_str() + RTSPBUFFERHEADSPACE;

		return (char*)buffer;
	}

	void set(const char* str, uint32_t _size)
	{
		if (str && _size > 0)
		{
			buffer = alloc(_size);
			size = _size;
			memcpy((char*)buffer, str, size);
		}
	}
};

RTSPBuffer::RTSPBuffer()
{
	internal = new RTSPBufferInternal();
}
RTSPBuffer::RTSPBuffer(const char* str)
{
	internal = new RTSPBufferInternal();

	if (str) internal->set(str, strlen(str));
}
RTSPBuffer::RTSPBuffer(const char* str, uint32_t size)
{
	internal = new RTSPBufferInternal();

	if (str && size > 0) internal->set(str, size);
}
RTSPBuffer::RTSPBuffer(const std::string& str)
{
	internal = new RTSPBufferInternal();
	internal->set(str.c_str(), str.length());
}
RTSPBuffer::RTSPBuffer(const String& str)
{
	internal = new RTSPBufferInternal();
	internal->set(str.c_str(), str.length());
}
RTSPBuffer::RTSPBuffer(const std::string& str, const char* buffer, size_t size)
{
	internal = new RTSPBufferInternal();
	internal->set(str.c_str(),str.length());
	if (buffer && size > 0)
	{
		size_t offset = buffer - str.c_str();

		internal->buffer = internal->buffer + offset;
		internal->size = size;
	}
}
RTSPBuffer::RTSPBuffer(const String& str, const char* buffer, size_t size)
{
	internal = new RTSPBufferInternal();

	if (buffer && size > 0 && buffer - str.c_str() > RTSPBUFFERHEADSPACE)
	{
		internal->data = str;
		internal->buffer = buffer;
		internal->size = size;
	}
	else
	{
		internal->set(str.c_str(), str.length());
		if (buffer && size > 0)
		{
			size_t offset = buffer - str.c_str();

			internal->buffer = internal->buffer + offset;
			internal->size = size;
		}
	}
}
RTSPBuffer::RTSPBuffer(const RTSPBuffer& str, const char* buffer, size_t size)
{
	internal = new RTSPBufferInternal();

	internal->data = str.internal->data;
	internal->buffer = str.internal->buffer;
	internal->size = str.internal->size;

	if (buffer != NULL && size > 0)
	{
		internal->buffer = buffer;
		internal->size = size;
	}
}
RTSPBuffer::RTSPBuffer(const RTSPBuffer& str)
{
	internal = new RTSPBufferInternal();

	internal->data = str.internal->data;
	internal->buffer = str.internal->buffer;
	internal->size = str.internal->size;
}
RTSPBuffer::~RTSPBuffer()
{
	SAFE_DELETE(internal);
}
char* RTSPBuffer::alloc(size_t size)
{
	return internal->alloc(size);
}
void RTSPBuffer::resize(size_t size)
{
	if (size <= internal->data.length() - RTSPBUFFERHEADSPACE)
	{
		internal->size = size;
	}
	else
	{
		internal->size = 0;
	}
}

const char* RTSPBuffer::buffer() const
{
	return internal->buffer;
}
size_t RTSPBuffer::size() const
{
	return internal->size;
}

const String& RTSPBuffer::dataptr()const
{
	return internal->data;
}
RTSPBuffer& RTSPBuffer::operator=(const RTSPBuffer& str)
{
	internal->data = str.internal->data;
	internal->buffer = str.internal->buffer;
	internal->size = str.internal->size;

	return *this;
}

}
}