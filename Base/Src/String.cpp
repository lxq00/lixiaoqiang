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


/// 兼容不同平台下的函数
typedef int (*snprintfFunc)(char *buffer, size_t count, const char *format, ... );
extern snprintfFunc snprintf;

/// 兼容不同平台下的函数
typedef int (*stricmpFunc)(const char* str1, const char* str2);
extern stricmpFunc stricmp;

#ifdef WIN32
snprintfFunc snprintf = ::_snprintf;
stricmpFunc stricmp = ::_stricmp;
#else
	snprintfFunc snprintf = ::snprintf;
	stricmpFunc stricmp = ::strcasecmp;
#endif


size_t strncat(char* dest, size_t dstBufLen, const char* src, const size_t srcCopyCount)
{
	size_t len = strlen(dest);
	len = dstBufLen - len - 1 > srcCopyCount ? srcCopyCount:dstBufLen - len - 1;
	::strncat(dest, src, len);
#ifdef WIN32
	dest[dstBufLen - 1] = '\0';
#endif
	return len;
}

size_t strncpy(char* dst, size_t dstBufLen, const char* src, size_t srcCopyCount)
{
	size_t count = dstBufLen - 1 > srcCopyCount?srcCopyCount:dstBufLen-1;
	::strncpy(dst, src, count);
	dst[count] = '\0';
	return strlen(dst);
}

int snprintf_x(char* buf, int maxlen, const char* fmt, ... )
{
	va_list arg;
	int count;
	va_start (arg, fmt);

#ifdef WIN32
	count = _vsnprintf(buf, maxlen, fmt, arg);
	if ( count < 0 || count == maxlen) {
		count = maxlen  - 1;
		buf[maxlen - 1] = '\0';
	}
#else
	count = ::vsnprintf(buf, maxlen, fmt, arg);
	if (count >= maxlen - 1)
		buf[maxlen - 1] = '\0';
	else if (count < 0) {
		va_end(arg);
		return 0;
	}
#endif
	va_end (arg);
	return (int)::strlen(buf);
}

std::string String::tolower(const std::string& src)
{
	std::string tmp;
	const char* tmpbuf = src.c_str();
	size_t len = src.length();
	for (size_t i = 0; i < len; i++)
	{
		char tmpc = ::tolower(tmpbuf[i]);
		tmp.push_back(tmpc);
	}

	return std::move(tmp);
}

//字符串转大写
std::string String::toupper(const std::string& src)
{
	std::string tmp;
	const char* tmpbuf = src.c_str();
	size_t len = src.length();
	for (size_t i = 0; i < len; i++)
	{
		char tmpc = ::toupper(tmpbuf[i]);
		tmp.push_back(tmpc);
	}

	return std::move(tmp);
}


std::string String::ansi2utf8(const std::string& inString)
{
	do 
	{
#ifdef WIN32
		int len = MultiByteToWideChar(CP_ACP, 0, inString.c_str(), (int)inString.length(), NULL, 0);
		wchar_t *wstr = new wchar_t[len + 1];
		int tmplen  = MultiByteToWideChar(CP_ACP, 0,inString.c_str(), (int)inString.length(), wstr, len);
		len = WideCharToMultiByte(CP_UTF8, 0, wstr, tmplen, NULL, 0, NULL, NULL);
		char *buf = new char[len + 1];
		WideCharToMultiByte(CP_UTF8, 0, wstr, tmplen,buf, len, NULL, NULL);
		delete []wstr;
		std::string tmp(buf, len);
		delete []buf;
		return std::move(tmp);
#else
		iconv_t cd = iconv_open("utf-8","gb2312");
		if(cd == (iconv_t)-1)
		{
			break;
		}
		char *pin = (char*)inString.c_str();
		size_t inlen = inString.length();

		size_t outlen = inlen*3;
		size_t outbuflen = outlen;
		char *outbuf = new char[outlen];
		char *deloutbuf = outbuf;
		
		size_t translen = iconv(cd,&pin,&inlen,&outbuf,&outlen);
		if (translen == (size_t)-1) 
		{
			iconv_close(cd);
			delete []deloutbuf;
			break;
		}
		iconv_close(cd);
		std::string tmp(deloutbuf, outbuflen - outlen);
		delete []deloutbuf;
		return tmp;
#endif
	} while (0);

	return "";
}
std::string  String::utf82ansi(const std::string& inString)
{
	do 
	{
#ifdef WIN32
		int len = MultiByteToWideChar(CP_UTF8, 0, inString.c_str(), (int)inString.length(), NULL, 0);
		wchar_t *wstr = new wchar_t[len + 1];
		int lentmp = MultiByteToWideChar(CP_UTF8, 0,inString.c_str(), (int)inString.length(), wstr, len);
		len = WideCharToMultiByte(CP_ACP, 0, wstr, lentmp, NULL, 0, NULL, NULL);
		char *buf = new char[len + 1];
		WideCharToMultiByte(CP_ACP, 0, wstr, lentmp,buf, len, NULL, NULL);
		delete []wstr;
		std::string tmp(buf, len);

		delete []buf;
		return std::move(tmp);
#else
		iconv_t cd = iconv_open("gb2312","utf-8");
		if(cd == (iconv_t)-1)
		{
			break;
		}

		size_t outlen = inString.length();
		size_t outbuflen = outlen;
		char *outbuf = new char[outlen];
		char *deloutbuf = outbuf;

		char *pin = (char*)inString.c_str();
		size_t inlen = inString.length();

		size_t translen = iconv(cd,&pin,&inlen,&outbuf,&outlen);
		if (translen == (size_t)-1) 
		{
			iconv_close(cd);
			delete []deloutbuf;
			break;
		}
		iconv_close(cd);
		std::string tmp(deloutbuf, outbuflen - outlen);
		delete []deloutbuf;
		return std::move(tmp);
#endif
	} while (0);

	return "";
}

std::vector<std::string> String::split(const std::string& src, const std::string& howmany)
{
	return split(src.c_str(), src.length(), howmany.c_str());
}

const char* mystrstr(const char* src, size_t len, const char* findstr)
{
	const char* tmp = src;
	const char* tmpend = src + len;
	size_t findlend = strlen(findstr);

	while (tmp < tmpend && size_t(tmpend - tmp) >= findlend)
	{
		if (memcmp(tmp, findstr, findlend) == 0)
		{
			return tmp;
		}
		tmp++;
	}

	return NULL;
}

std::vector<std::string> String::split(const char* src, size_t len, const std::string& howmany)
{
	if (src == NULL || len <= 0) return std::vector<std::string>();

	std::vector<std::string> values;

	const char* tmp = src;
	const char* tmpend = src + len;

	while (tmp < tmpend)
	{
		std::string valuestr;
		const char* findstr = mystrstr(tmp, tmpend - tmp, howmany.c_str());
		if (findstr == NULL)
		{
			valuestr = strip(std::string(tmp,tmpend-tmp));
		}
		else
		{
			valuestr = strip(std::string(tmp, findstr - tmp));
			tmp = findstr + howmany.length();
		}

		if (valuestr != "") values.push_back(valuestr);
		if (findstr == NULL) break;
	}

	return std::move(values);
}
std::vector<std::string> String::defaultHowmany()
{
	std::vector<std::string> howtmp;

	howtmp.push_back(" ");
	howtmp.push_back("\r");
	howtmp.push_back("\n");
	howtmp.push_back("\t");

	return std::move(howtmp);
}

std::string String::strip(const std::string& src, const std::vector<std::string>& howmany)
{
	std::string tmp = ltrip(src, howmany);
	
	tmp = rtrip(tmp, howmany);

	return std::move(tmp);
}



// 右清理,
std::string String::ltrip(const std::string& src, const std::vector<std::string>& howmany)
{
	const char* tmp = src.c_str();

	while (*tmp)
	{
		bool havedo = false;
		for(uint32_t i = 0;i < howmany.size();i ++)
		{
			if (strncasecmp(tmp, howmany[i].c_str(), howmany[i].length()) == 0)
			{
				tmp += howmany[i].length();
				havedo = true;
			}
		}
		if (!havedo) break;
	}

	return std::move(string(tmp));
}
// 右清理,
std::string String::rtrip(const std::string& src, const std::vector<std::string>& howmany)
{
	const char* tmp = src.c_str();
	const char* tmpend = tmp + src.length();

	while (tmpend > tmp)
	{
		bool havedo = false;
		for (uint32_t i = 0; i < howmany.size(); i++)
		{
			if (tmpend - tmp >= (int)howmany[i].length() && strncasecmp(tmpend - howmany[i].length(), howmany[i].c_str(), howmany[i].length()) == 0)
			{
				tmpend -= howmany[i].length();
				havedo = true;
			}
		}
		if (!havedo) break;
	}

	return std::move(std::string(tmp, tmpend - tmp));
}

size_t String::indexOf(const std::string& src, const std::string& fromindex)
{
	return indexOf(src.c_str(), src.length(), fromindex);
}
size_t String::indexOf(const char* src, size_t len, const std::string& fromindex)
{
	const char* tmp = src;
	const char* tmpend = tmp + len;
	const char* fromtmp = fromindex.c_str();
	size_t fromlen = fromindex.length();
	while (size_t(tmpend - tmp) >= fromlen)
	{
		if (strncmp(tmp, fromtmp, fromlen) == 0)  return tmp - src;
		tmp++;
	}

	return -1;
}
size_t String::indexOfByCase(const std::string& src, const std::string& fromindex)
{
	return indexOfByCase(src.c_str(), src.length(), fromindex);
}
size_t String::indexOfByCase(const char* src, size_t len, const std::string& fromindex)
{
	const char* tmp = src;
	const char* tmpend = tmp + len;
	const char* fromtmp = fromindex.c_str();
	size_t fromlen = fromindex.length();
	while (size_t(tmpend - tmp) >= fromlen)
	{
		if (strncasecmp(tmp, fromtmp, fromlen) == 0)  return tmp - src;
		tmp++;
	}

	return -1;
}
size_t String::lastIndexOf(const std::string& src, const std::string& fromindex)
{
	return lastIndexOf(src.c_str(), src.length(), fromindex);
}
size_t String::lastIndexOf(const char* src, size_t len, const std::string& fromindex)
{
	const char* tmp = src;
	const char* tmpend = tmp + len;
	const char* fromtmp = fromindex.c_str();
	size_t fromlen = fromindex.length();
	while (size_t(tmpend - tmp) >= fromlen)
	{
		if (strncmp(tmpend - fromlen, fromtmp, fromlen) == 0)  return tmpend - fromlen - tmp;
		tmpend--;;
	}

	return -1;
}
size_t String::lastIndexOfByCase(const std::string& src, const std::string& fromindex)
{
	return lastIndexOfByCase(src.c_str(), src.length(), fromindex);
}
size_t String::lastIndexOfByCase(const char* src, size_t len, const std::string& fromindex)
{
	const char* tmp = src;
	const char* tmpend = tmp + len;
	const char* fromtmp = fromindex.c_str();
	size_t fromlen = fromindex.length();
	while (size_t(tmpend - tmp) >= fromlen)
	{
		if (strncasecmp(tmpend - fromlen, fromtmp, fromlen) == 0)  return tmpend - tmp;
		tmpend--;;
	}

	return -1;
}

std::string String::replace(const std::string& src, const std::string& substr, const std::string& replacement)
{
	const char* srctmp = src.c_str();
	std::string strtmp;

	while (1)
	{
		const char* tmp = strstr(srctmp, substr.c_str());
		if (tmp == NULL)
		{
			strtmp += srctmp;
			break;
		}
		strtmp += std::string(srctmp, tmp - srctmp);
		strtmp += replacement;
		srctmp = tmp + substr.length();
	}

	return std::move(strtmp);
}

std::string String::snprintf_x(int maxsize, const char* fmt, ...)
{
    std::string strbuf;
    strbuf.resize(maxsize);
    char* buf = (char*)strbuf.c_str();
    va_list arg;
    int count;
    va_start(arg, fmt);

#ifdef WIN32
    count = _vsnprintf(buf, maxsize, fmt, arg);
    if (count < 0 || count == maxsize) {
        count = maxsize - 1;
        buf[maxsize - 1] = '\0';
    }
#else
    count = ::vsnprintf(buf, maxsize, fmt, arg);
    if (count >= maxsize - 1)
        buf[maxsize - 1] = '\0';
    else if (count < 0) {
        va_end(arg);
        return 0;
    }
#endif
    va_end(arg);
    return strbuf.c_str();
}





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


	void resize(size_t size)
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
			resize(size);
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

			resize(buffer == NULL ? size : size + buffer->dataLength);

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
	static const char* emtpystr = "";
	if (internal == NULL || internal->buffer == NULL || internal->buffer->buffer == NULL) return emtpystr;

	return internal->buffer->buffer;
}
size_t String::length() const
{
	if (internal == NULL || internal->buffer == NULL) return 0;

	return internal->buffer->dataLength;
}
void String::resize(size_t size)
{
	internal->resize(size);
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
	if (str != NULL && size >= 0)
		internal->append(str, size);

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

