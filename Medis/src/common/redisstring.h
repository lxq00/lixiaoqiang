#pragma once
#include "memory.h"

class RedisString
{
	struct RedisBuffer
	{
		char*		buffer;
		uint32_t	bufferlen;
		uint32_t	datalen;

		RedisBuffer(uint32_t len):datalen(0),bufferlen(len)
		{
			buffer = (char*)Memory::instance()->Malloc(bufferlen);
		}
		~RedisBuffer()
		{
			Memory::instance()->Free(buffer);
		}
	};
public:
	RedisString(){}
	RedisString(const RedisString& string):buffer(string.buffer){}
	RedisString(const std::string& string) 
	{
		buffer = make_shared<RedisBuffer>(string.length());
		if (buffer->buffer == NULL)return;

		buffer->datalen = string.length();
		memcpy(buffer->buffer, string.c_str(), buffer->datalen);
	}
	~RedisString() {}

	uint32_t length() const { return buffer == NULL ? 0 : buffer->datalen; }
	inline bool operator == (const RedisString &t) const {
		return buffer.get() == t.buffer.get();
	}
	operator std::string() const
	{
		return std::string(ptr(), length());
	}
	const char* ptr() const 
	{
		static char* emtptyptr = "";
		return buffer == NULL ? emtptyptr : buffer->buffer;
	}
private:
	shared_ptr<RedisBuffer>	buffer;
};