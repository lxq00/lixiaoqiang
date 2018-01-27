//
//  Copyright (c)1998-2012, Chongqing Public Technology
//  All Rights Reserved.
//
//	Description:
//	$Id: String.cpp 227 2013-10-30 01:10:30Z linyang $
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
	int len = src.length();
	for (int i = 0; i < len; i++)
	{
		char tmpc = ::tolower(tmpbuf[i]);
		tmp.push_back(tmpc);
	}

	return tmp;
}

//字符串转大写
std::string String::toupper(const std::string& src)
{
	std::string tmp;
	const char* tmpbuf = src.c_str();
	int len = src.length();
	for (int i = 0; i < len; i++)
	{
		char tmpc = ::toupper(tmpbuf[i]);
		tmp.push_back(tmpc);
	}

	return tmp;
}


std::string String::ansi2utf8(const std::string& inString)
{
	do 
	{
#ifdef WIN32
		int len = MultiByteToWideChar(CP_ACP, 0, inString.c_str(), inString.length(), NULL, 0);
		wchar_t *wstr = new wchar_t[len + 1];
		int tmplen  = MultiByteToWideChar(CP_ACP, 0,inString.c_str(), inString.length(), wstr, len);
		len = WideCharToMultiByte(CP_UTF8, 0, wstr, tmplen, NULL, 0, NULL, NULL);
		char *buf = new char[len + 1];
		WideCharToMultiByte(CP_UTF8, 0, wstr, tmplen,buf, len, NULL, NULL);
		delete []wstr;
		std::string tmp(buf, len);
		delete []buf;
		return tmp;
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
		int len = MultiByteToWideChar(CP_UTF8, 0, inString.c_str(), inString.length(), NULL, 0);
		wchar_t *wstr = new wchar_t[len + 1];
		int lentmp = MultiByteToWideChar(CP_UTF8, 0,inString.c_str(), inString.length(), wstr, len);
		len = WideCharToMultiByte(CP_ACP, 0, wstr, lentmp, NULL, 0, NULL, NULL);
		char *buf = new char[len + 1];
		WideCharToMultiByte(CP_ACP, 0, wstr, lentmp,buf, len, NULL, NULL);
		delete []wstr;
		std::string tmp(buf, len);

		delete []buf;
		return tmp;
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
		return tmp;
#endif
	} while (0);

	return "";
}

} // namespace Base
} // namespace Public

