//
//  Copyright (c)1998-2012,  Public Technology
//  All Rights Reserved.
//
//	Description:
//	$Id: String.cpp 227 2013-10-30 01:10:30Z  $
//
#ifdef WIN32
#include <windows.h>
#else
#include <errno.h>
#include <string.h>
#include <iconv.h>
#endif
#include <stdarg.h>
#include <stdio.h>

#include "Base/ByteOrder.h"
#include "Base/String.h"
#include "Base/PrintLog.h"

namespace Public {
namespace Base {

static const char* emtpystr = "";

struct String::StringInternal
{
	struct StringBufer
	{
		char* buffer;
		size_t bufferSize;
		size_t dataLength;
		shared_ptr<IMempoolInterface> mempool;

		StringBufer() :buffer(NULL), bufferSize(0), dataLength(0) {}
		~StringBufer()
		{
			if (buffer != NULL)
			{
				if (mempool) mempool->Free(buffer);
				else SAFE_DELETEARRAY(buffer);
			}

			buffer = NULL;
			bufferSize = 0;
			dataLength = 0;
		}
	};

	shared_ptr<StringBufer> buffer;
	shared_ptr<IMempoolInterface> mempool;


	void alloc(size_t size)
	{
		shared_ptr<StringBufer> newbuffer;
		if (size > 0)
		{
			newbuffer = make_shared<StringBufer>();
			newbuffer->mempool = mempool;

			if (mempool) newbuffer->buffer = (char*)mempool->Malloc(size, newbuffer->bufferSize);
			else 
			{
				newbuffer->bufferSize = size;
				newbuffer->buffer = new char[size];
			}
		}

		buffer = newbuffer;
	}

	void setval(const char* ptr, size_t size)
	{
		if (ptr == NULL || size <= 0) return;

		if (buffer == NULL || size > buffer->bufferSize)
		{
			alloc(size);
		}

		if (buffer == NULL || buffer->buffer == NULL) return;

		memcpy(buffer->buffer, ptr, size);
		buffer->dataLength = size;
	}

	void append(const char* ptr, size_t size)
	{
		if (ptr == NULL || size <= 0) return;

		if (buffer == NULL || size + buffer->dataLength > buffer->bufferSize)
		{
			shared_ptr<StringBufer> nowbuffer = buffer;

			alloc(buffer == NULL ? size : size + buffer->dataLength);

			if (nowbuffer != NULL && buffer != NULL)
			{
				memcpy(buffer->buffer, nowbuffer->buffer, nowbuffer->dataLength);
				buffer->dataLength = nowbuffer->dataLength;
			}
		}

		if (buffer == NULL || buffer->buffer == NULL || buffer->dataLength + size > buffer->bufferSize) return;
		memcpy(buffer->buffer + buffer->dataLength, ptr, size);
		buffer->dataLength += size;
	}
};

String::String(const shared_ptr<IMempoolInterface>& mempool)
{
	internal = new StringInternal;
	internal->mempool = mempool;
}
String::String(const char* str, const shared_ptr<IMempoolInterface>& mempool)
{
	internal = new StringInternal;
	internal->mempool = mempool;

	if (str != NULL)
		internal->setval(str, strlen(str));
}
String::String(const char* str, size_t len, const shared_ptr<IMempoolInterface>& mempool)
{
	internal = new StringInternal;
	internal->mempool = mempool;

	if(str != NULL && len > 0)
		internal->setval(str, len);
}
String::String(const std::string& str, const shared_ptr<IMempoolInterface>& mempool)
{
	internal = new StringInternal;
	internal->mempool = mempool;
	internal->setval(str.c_str(), str.length());
}
String::String(const String& str)
{
	internal = new StringInternal;
	internal->mempool = str.internal->mempool;
	internal->buffer = str.internal->buffer;
}
String::~String()
{
	SAFE_DELETE(internal);
}

const char* String::c_str() const
{
	if (internal == NULL || internal->buffer == NULL || internal->buffer->buffer == NULL) return emtpystr;

	return internal->buffer->buffer;
}
size_t String::length() const
{
	if (internal == NULL || internal->buffer == NULL) return 0;

	return internal->buffer->dataLength;
}
String&  String::resize(size_t size)
{
	if (internal->buffer != NULL || size <= internal->buffer->bufferSize)
	{
		internal->buffer->dataLength = size;
	}

	return *this;
}
String&  String::alloc(size_t size)
{
	internal->alloc(size);

	return *this;
}
String& String::operator = (const char* str)
{
	if (str != NULL)
		internal->setval(str, strlen(str));
	return *this;
}
String& String::operator = (const std::string& str)
{
	internal->setval(str.c_str(), str.length());
	return *this;
}
String& String::operator = (const String& str)
{
	internal->buffer = str.internal->buffer;
	return *this;
}

String& String::operator +=(const char* str)
{
	if (str != NULL)
		internal->append(str, strlen(str));
	return *this;
}
String& String::operator +=(const std::string& str)
{
	internal->append(str.c_str(), str.length());
	return *this;
}
String& String::operator +=(const String& str)
{
	if (str.internal->buffer && str.internal->buffer->buffer)
		internal->append(str.internal->buffer->buffer, str.internal->buffer->dataLength);
	return *this;
}

String& String::append(const char* str, size_t size)
{
	if (str != NULL)
	{
		if (size == 0) size = strlen(str);

		internal->append(str, size);
	}		

	return *this;
}
String& String::append(const std::string& str)
{
	internal->append(str.c_str(), str.length());

	return *this;
}
String& String::append(const String& str)
{
	internal->append(str.c_str(), str.length());

	return *this;
}

} // namespace Base
} // namespace Public

