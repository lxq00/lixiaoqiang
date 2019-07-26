#pragma once
#include "HTTP/HTTPPublic.h"
#include "HTTPDefine.h"
#include "HTTP/HTTPParse.h"
namespace Public {
namespace HTTP {

template<typename OBJECT>
struct parseHTTPHeader
{
	static void parse(const shared_ptr<OBJECT>&obj, const shared_ptr<HTTPParse::Header>& header) { }
};


template<>
struct parseHTTPHeader<HTTPRequest>
{
	static void parse(const shared_ptr<HTTPRequest>&object, const shared_ptr<HTTPParse::Header>& header)
	{
		object->method() = header->method;
		object->url() = header->url;
		object->headers() = header->headers;
	}
};

template<>
struct parseHTTPHeader<HTTPResponse>
{
	static void parse(const shared_ptr<HTTPResponse>&object, const shared_ptr<HTTPParse::Header>& header)
	{
		object->statusCode() = header->statuscode;
		object->errorMessage() = header->statusmsg;
		object->headers() = header->headers;
	}
};
template<typename OBJECT>
class HTTPProtocolParser:protected HTTPParse
{
public:
	typedef Function1<void, const shared_ptr<OBJECT>&> HeaderParseSuccessCallback;
private:
	int  inputContentLen;
	int  ContentLen;

	bool headerHaveFind;
	weak_ptr<OBJECT> object;

	uint64_t					socketUsedTime;
	HeaderParseSuccessCallback	headerCallback;
public:
	HTTPProtocolParser(const shared_ptr<OBJECT>& _obj,const HeaderParseSuccessCallback& callback)
		:headerHaveFind(false), inputContentLen(0), ContentLen(-1), object(_obj)
		, socketUsedTime(Time::getCurrentMilliSecond()),headerCallback(callback)
	{}
	virtual ~HTTPProtocolParser() {}

	const char* inputData(const char* buftmp, int len)
	{
		socketUsedTime = Time::getCurrentMilliSecond();
		
		if (!headerHaveFind)
		{
			uint32_t haveusedlen = 0;
			shared_ptr<HTTPParse::Header> header = parse(buftmp, len, haveusedlen);
			if (header != NULL)
			{
				parseHTTPHeader<OBJECT>::parse(object, header);
				headerHaveFind = true;
				headerCallback(objtmp);
			}
			
			
			buftmp += haveusedlen;
			len -= haveusedlen;
		}
		if (headerHaveFind && len > 0)
		{
			buftmp = parseContent(buftmp, len);
		}
		
		return buftmp;
	}
	bool isFinish()
	{
		return inputContentLen == ContentLen;
	}
	uint64_t  prevAliveTime() { return socketUsedTime; }
protected:
	const char* parseContent(const char* tmpbuf,int bufferlen)
	{
		if (!headerHaveFind) return tmpbuf;

		shared_ptr<OBJECT> objtmp = object.lock();
		if (objtmp == NULL) return tmpbuf;

		int contentlentmp = bufferlen;

		if (ContentLen >= 0)
		{
			contentlentmp = min(ContentLen - inputContentLen, bufferlen);
		}

		if(contentlentmp > 0)
		{
			const char* tmp = writeContent(objtmp,tmpbuf, contentlentmp);
			//修正实际已经使用的长度
			contentlentmp = int(tmp - tmpbuf);
		}
		inputContentLen += contentlentmp;

		return tmpbuf + contentlentmp;
	}
private:	
	const char* writeContent(shared_ptr<OBJECT>& object, const char* buffer, int len)
	{
		Value transeencoding = object->header(Transfer_Encoding);
		bool chunked = strcasecmp(transeencoding.readString().c_str(), CHUNKED) == 0;
		bool chunedfinish = false;
		const char* buftmp = object->content()->inputAndParse(buffer, len, chunked, chunedfinish);
		
		//如果是chunk，切已经结束
		if (chunked && chunedfinish)
		{
			//直接修改contentlen为上次输入的长度和当前使用的长度，让finish=true
			ContentLen = int(inputContentLen + (buftmp - buffer));
		}

		return buftmp;
	}
};

}
}

