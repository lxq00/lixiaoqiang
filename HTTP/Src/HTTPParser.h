#pragma once
#include "HTTP/HTTPPublic.h"
#include "HTTPDefine.h"
namespace Public {
namespace HTTP {

template<typename OBJECT>
class HTTPParser
{
public:
	HTTPParser(HTTPBuffer::HTTPBufferType type = HTTPBuffer::HTTPBufferType_String):bufferlen(0), headerHaveFind(false), inputContentLen(0), ContentLen(-1)
	{
		object = make_shared<OBJECT>(type);
	}
	virtual ~HTTPParser() {}
protected:
	bool parseContent()
	{
		const char* tmpbuf = buffer;

		if (ContentLen == -1)
		{
			Value tmp = object->header(Content_Length);
			if (!tmp.empty()) ContentLen = tmp.readInt();
			//ÇëÇóÃüÁî£¬Ä¬ÈÏcontentlen==0
			else if (typeid(OBJECT).name() == typeid(HTTPRequest).name()) ContentLen = 0;
		}

		int contentlentmp = bufferlen;

		if (ContentLen >= 0)
		{
			contentlentmp = min(ContentLen - inputContentLen, bufferlen);
		}

		bool ret = contentlentmp == 0 ? true : parseContent(tmpbuf, contentlentmp);
		inputContentLen += contentlentmp;
		tmpbuf += contentlentmp;
		bufferlen -= contentlentmp;

		if (bufferlen > 0 && tmpbuf != buffer)
		{
			memcpy(buffer, tmpbuf, bufferlen);
		}

		return contentIsOk();
	}
	bool parseHeader()
	{
		const char* tmpbuf = buffer;
		int pos = String::indexOf(buffer, bufferlen, HTTPHEADEREND);
		if (pos == -1 && bufferlen >= MAXRECVBUFFERLEN) return false;
		if (pos == -1)
		{
			return false;
		}
		tmpbuf = buffer + pos + strlen(HTTPHEADEREND);
		bufferlen -= pos + strlen(HTTPHEADEREND);

		const char* firsttmp = parseFirstLine(buffer, tmpbuf - buffer);
		if (firsttmp == NULL) return false;

		parseHeader(firsttmp, tmpbuf - firsttmp);
		headerHaveFind = true;

		if (bufferlen > 0 && tmpbuf != buffer)
		{
			memcpy(buffer, tmpbuf, bufferlen);
		}

		return true;
	}

	bool intpulen(int len)
	{
		if (bufferlen + len > MAXRECVBUFFERLEN)
		{
			return false;
		}

		char* tmpbuf = buffer;
		bufferlen += len;
		
		return true;
	}
public:
	bool contentIsOk()
	{
		if (ContentLen == -1) return false;

		return ContentLen == inputContentLen;
	}
private:
	virtual const char* parseFirstLine(const char* buffer, int len) = 0;
	bool parseHeader(const char* buffer, int len)
	{
		std::vector<std::string> values = String::split(buffer, len, HTTPHREADERLINEEND);
		for (uint32_t i = 0; i < values.size(); i++)
		{
			std::vector<std::string> tmp = String::split(values[i], ":");
			if (tmp.size() != 0)
			{
				object->headers()[String::strip(tmp[0])] = tmp.size() == 1 ? "" : String::strip(tmp[1]);
			}
		}

		return true;
	}
	bool parseContent(const char* buffer, int len)
	{
		return object->content()->write(buffer, len);
	}
protected:
	char* readfbuffer() { return buffer + bufferlen; }
	int readbufferlen() { return MAXRECVBUFFERLEN - bufferlen; };
	int bufferdatalen() { return bufferlen; }
	bool headerIsOk() { return headerHaveFind; }
public:
	shared_ptr<OBJECT> object;
private:
	char buffer[MAXRECVBUFFERLEN];
	int  bufferlen;
	int  inputContentLen;
	int  ContentLen;
	bool headerHaveFind;
};

}
}

