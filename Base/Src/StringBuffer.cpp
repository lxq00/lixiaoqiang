#include "Base/StringBuffer.h"

namespace Public{
namespace Base {

struct StringBuffer::StringBufferInternal
{
	String			dataobj;
	const char*		buffer;
	size_t			size;

	StringBufferInternal():buffer(NULL),size(0){}
};

StringBuffer::StringBuffer()
{
	internal = new StringBufferInternal;
}
StringBuffer::StringBuffer(const char* str, const shared_ptr<IMempoolInterface>& mempool)
{
	internal = new StringBufferInternal;
	set(str, mempool);
}
StringBuffer::StringBuffer(const char* str, size_t size, const shared_ptr<IMempoolInterface>& mempool)
{
	internal = new StringBufferInternal;
	set(str, size, mempool);
}
StringBuffer::StringBuffer(const std::string& str, const shared_ptr<IMempoolInterface>& mempool)
{
	internal = new StringBufferInternal;
	set(str,mempool);
}
StringBuffer::StringBuffer(const String& dataobj, const char* str, size_t size)
{
	internal = new StringBufferInternal;
	set(dataobj,str,size);
}
StringBuffer::StringBuffer(const StringBuffer& buffer, const char* str, size_t size)
{
	internal = new StringBufferInternal;
	set(buffer,str,size);
}
StringBuffer::~StringBuffer()
{
	SAFE_DELETE(internal);
}
const char* StringBuffer::buffer() const
{
	if (internal->buffer == NULL) return internal->dataobj.c_str();

	return internal->buffer;
}
size_t StringBuffer::size() const
{
	return internal->size;
}
const String& StringBuffer::dataobj() const
{
	return internal->dataobj;
}
StringBuffer& StringBuffer::operator = (const char* str)
{
	set(str);

	return *this;
}
StringBuffer& StringBuffer::operator = (const std::string& str)
{
	set(str);

	return *this;
}
StringBuffer& StringBuffer::operator = (const String& str)
{
	set(str);
	
	return *this;
}
StringBuffer& StringBuffer::operator = (const StringBuffer& buffer)
{
	set(buffer);

	return *this;
}
void StringBuffer::set(const char* str, const shared_ptr<IMempoolInterface>& mempool)
{
	set(str, str == NULL ? 0 : strlen(str), mempool);
}
void StringBuffer::set(const char* str, size_t size, const shared_ptr<IMempoolInterface>& mempool)
{
	internal->dataobj = "";
	internal->buffer = NULL;
	internal->size = 0;

	if (str != NULL && size > 0)
	{
		internal->dataobj = String(str, size, mempool);
		internal->buffer = internal->dataobj.c_str();
		internal->size = internal->dataobj.length();
	}
}
void StringBuffer::set(const std::string& str, const shared_ptr<IMempoolInterface>& mempool)
{
	set(str.c_str(), str.length(), mempool);
}
void StringBuffer::set(const String& strtmp, const char* str, size_t size)
{
	internal->dataobj = strtmp;
	internal->buffer = internal->dataobj.c_str();
	internal->size = internal->dataobj.length();

	if (str != NULL && size > 0)
	{
		internal->buffer = str;
		internal->size = size;
	}
}
void StringBuffer::set(const StringBuffer& buffer, const char* str, size_t size)
{
	internal->dataobj = buffer.internal->dataobj;
	internal->buffer = buffer.internal->buffer;
	internal->size = buffer.internal->size;

	if (str != NULL && size != 0)
	{
		internal->buffer = str;
		internal->size = size;
	}
}

}
}