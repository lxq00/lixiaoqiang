//
//  Copyright (c)1998-2012, Chongqing Xunmei Technology
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

namespace Xunmei {
namespace Base {

CharsetEncodeCallback charsetCallback;

void BASE_API registeCharsetEncode(const CharsetEncodeCallback& callback)
{
	charsetCallback = callback;
}

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


size_t BASE_API strncat(char* dest, size_t dstBufLen, const char* src, const size_t srcCopyCount)
{
	size_t len = strlen(dest);
	len = dstBufLen - len - 1 > srcCopyCount ? srcCopyCount:dstBufLen - len - 1;
	::strncat(dest, src, len);
#ifdef WIN32
	dest[dstBufLen - 1] = '\0';
#endif
	return len;
}

size_t BASE_API strncpy(char* dst, size_t dstBufLen, const char* src, size_t srcCopyCount)
{
	size_t count = dstBufLen - 1 > srcCopyCount?srcCopyCount:dstBufLen-1;
	::strncpy(dst, src, count);
	dst[count] = '\0';
	return strlen(dst);
}

int BASE_API snprintf_x(char* buf, int maxlen, const char* fmt, ... )
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
std::string BASE_API ansi2utf8Ex(const std::string& inString)
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
	if(charsetCallback == NULL)
	{
		//logerror("ansi2utf8Ex error\r\n");

		return inString;
	}

	return charsetCallback(inString,"gb2312","utf8");
}
int BASE_API ansi2utf8(char *inbuf,size_t inlen,char *outbuf,size_t outlen)
{
	if(inbuf == NULL || inlen == 0 || outbuf == NULL || outlen == 0)
	{
		return 0;
	}
	std::string outputstr = ansi2utf8Ex(std::string(inbuf,inlen));
	size_t outputlen = outlen > outputstr.length() ? outputstr.length() : outlen;
	memcpy(outbuf,outputstr.c_str(),outputlen);
	if(outputlen < outlen)
	{
		outbuf[outputlen] = 0;
	}

	return outputlen;
}

/// Ansi(Gb2312) 转换字符串为utf8格式 扩展接口(linux只支持转换后结果不超过1024bytes)
/// \param [in] inbuf 源字符串
/// \param [in]  inlen 源字符串的长度
/// \retval 空string 转换失败
///         非空string 转换结果
std::string BASE_API ansi2utf8Ex(char *inbuf,size_t inlen)
{
	if(inbuf == NULL || inlen == 0)
	{
		return "";
	}
	return ansi2utf8Ex(std::string(inbuf,inlen));
}

std::string BASE_API utf82ansiEx(const std::string& inString)
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

	if(charsetCallback == NULL)
	{
		//logdebug("utf82ansiEx error\r\n");

		return inString;
	}

	return charsetCallback(inString,"utf8","gb2312");
}

int BASE_API utf82ansi(char *inbuf,size_t inlen,char *outbuf,size_t outlen)
{
	if(inbuf == NULL || inlen == 0 || outbuf == NULL || outlen == 0)
	{
		return 0;
	}
	std::string outputstr = utf82ansiEx(std::string(inbuf,inlen));
	size_t outputlen = outlen > outputstr.length() ? outputstr.length() : outlen;
	memcpy(outbuf,outputstr.c_str(),outputlen);
	if(outputlen < outlen)
	{
		outbuf[outputlen] = 0;
	}

	return outputlen;
}


/// utf8转换字符串为 Ansi(Gb2312)格式  扩展接口(只支持转换后结果不超过1024bytes)
/// \param [in] inbuf 源字符串
/// \param [in]  inlen 源字符串的长度
/// \retval 空string 转换失败
///         非空string 转换结果
std::string BASE_API utf82ansiEx(char *inbuf,size_t inlen)
{
	if(inbuf == NULL || inlen == 0)
	{
		return "";
	}
	return utf82ansiEx(std::string(inbuf,inlen));
}

/// 通过标识符解析url字段
/// \param [in] url 要解析的url串
/// \param [in] key 解析的字段名字
/// \param [in] headSymbol 解析的字段名称首部分割符
/// \param [in] tailSymbol 解析的字段名称尾部分割符
/// \param [out] value 解析后的结果返回值
bool parseUrlByKey(std::string &url, char* key, char headSymbol,  char tailSymbol, std::string & value)
{
	int bPos = url.find(key);
	int ePos = -1;
	if ((unsigned int)bPos == std::string::npos)
	{
		return false;
	}
	bPos = url.find(headSymbol, bPos) + 1;
	if ((unsigned int)bPos == std::string::npos)
	{
		return false;
	}

	if (tailSymbol == '\0')
	{
		ePos = url.length();
	}
	else
		ePos = url.find(tailSymbol, bPos);

	if ((unsigned int)ePos == std::string::npos)
	{
		return false;
	}

	value = url.substr(bPos, ePos - bPos);
	return true;
}
} // namespace Base
} // namespace Xunmei

