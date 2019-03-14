#pragma once
#include "HTTP/HTTPPublic.h"
#include "HTTPDefine.h"
namespace Public {
namespace HTTP {

template<typename OBJECT>
struct parseFirstLine
{
	static const char* parse(const shared_ptr<OBJECT>&obj, const char* buffer, int len) { return buffer; }
};

template<>
struct parseFirstLine<HTTPRequest>
{
	static const char* parse(const shared_ptr<HTTPRequest>&object, const char* buffer, int len)
	{
		int pos = String::indexOf(buffer, len, HTTPHREADERLINEEND);
		if (pos == -1) return NULL;

		std::vector<std::string> tmp = String::split(buffer, pos, " ");
		if (tmp.size() != 3) return NULL;

		object->method() = tmp[0];
		object->url() = tmp[1];

		return buffer + pos + strlen(HTTPHREADERLINEEND);
	}
};

template<>
struct parseFirstLine<HTTPResponse>
{
	static const char* parse(const shared_ptr<HTTPResponse>&object, const char* buffer, int len)
	{
		int pos = String::indexOf(buffer, len, HTTPHREADERLINEEND);
		if (pos == -1) return NULL;

		std::vector<std::string> tmp = String::split(buffer, pos, " ");
		if (tmp.size() < 2) return NULL;

		object->statusCode() = Value(tmp[1]).readInt();

		std::string errstr;
		for (uint32_t i = 2; i < tmp.size(); i++)
		{
			errstr += (i == 2 ? "" : " ") + tmp[i];
		}

		object->errorMessage() = errstr;

		return buffer + pos + strlen(HTTPHREADERLINEEND);
	}
};




template<typename OBJECT>
class HTTPParser
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
	HTTPParser(const shared_ptr<OBJECT>& _obj,const HeaderParseSuccessCallback& callback)
		:headerHaveFind(false), inputContentLen(0), ContentLen(-1), object(_obj)
		, socketUsedTime(Time::getCurrentMilliSecond()),headerCallback(callback)
	{}
	virtual ~HTTPParser() {}

	const char* inputData(const char* buftmp, int len)
	{
		const char* tmp = buftmp;

		socketUsedTime = Time::getCurrentMilliSecond();
		
		if (!headerHaveFind)
		{
			buftmp = parseHeader(buftmp, len);
			len -= buftmp - tmp;
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
			contentlentmp = tmp - tmpbuf;
		}
		inputContentLen += contentlentmp;

		return tmpbuf + contentlentmp;
	}
	const char* parseHeader(const char* buffer,int bufferlen)
	{
		if (headerHaveFind) return buffer;

		shared_ptr<OBJECT> objtmp = object.lock();
		if (objtmp == NULL) return buffer;

		int pos = String::indexOf(buffer, bufferlen, HTTPHEADEREND);
		if (pos == -1) return buffer;

		const char* endheaderbuf = buffer + pos + strlen(HTTPHEADEREND);
		
		const char* firsttmp = parseFirstLine<OBJECT>::parse(objtmp,buffer, endheaderbuf - buffer);
		if (firsttmp == NULL) return buffer;

		parseHeader(objtmp,firsttmp, endheaderbuf - firsttmp);

		{
			Value tmp = objtmp->header(Content_Length);
			if (!tmp.empty()) ContentLen = tmp.readInt();
			//请求命令，默认contentlen==0
			else if (typeid(OBJECT).name() == typeid(HTTPRequest).name()) ContentLen = 0;

			//chunked 模块，等待结束
			Value transeencoding = objtmp->header(Transfer_Encoding);
			bool chunked = strcasecmp(transeencoding.readString().c_str(), CHUNKED) == 0;
			if (chunked) ContentLen = -1;
		}

		headerHaveFind = true;
		
		headerCallback(objtmp);

		return endheaderbuf;
	}
private:
	bool parseHeader(shared_ptr<OBJECT>& object,const char* buffer, int len)
	{
		std::vector<std::string> values = String::split(buffer, len, HTTPHREADERLINEEND);
		for (uint32_t i = 0; i < values.size(); i++)
		{
			std::string strtmp = values[i];
			const char* tmpstr = strtmp.c_str();
			const char* tmp = strchr(tmpstr, ':');
			if (tmp != NULL)
			{
				object->headers()[String::strip(std::string(tmpstr, tmp - tmpstr))] = String::strip(tmp + 1);
			}
		}

		return true;
	}
	const char* writeContent(shared_ptr<OBJECT>& object, const char* buffer, int len)
	{
		Value transeencoding = object->header(Transfer_Encoding);
		bool chunked = strcasecmp(transeencoding.readString().c_str(), CHUNKED) == 0;
		bool chunedfinish = false;
		const char* buftmp = object->content()->inputAndParse(buffer, len, chunked, chunedfinish);
		
		//如果时chunk，切已经结束
		if (chunked && chunedfinish)
		{
			//直接修改contentlen为上次输入的长度和当前使用的长度，让finish=true
			ContentLen = inputContentLen + (buftmp - buffer);
		}

		return buftmp;
	}
};

}
}

