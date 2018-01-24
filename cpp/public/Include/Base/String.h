//
//  Copyright (c)1998-2014, Public Technology
//  All Rights Reserved.
//
//
//	Description:
//	$Id: String.h 227 2013-10-30 01:10:30Z linyang $

#ifndef __BASE_STRING_H__
#define __BASE_STRING_H__

#include <stdio.h>
#include <string.h>
#include "BaseTemplate.h"
#include "Defs.h"
#include <string>
#include "Func.h"

inline void stringToLower(const char* strcstr,char* deststr)
{
	while(*strcstr)
	{
		*deststr = tolower(*strcstr);
		strcstr++;
		deststr++;
	}
	*deststr = 0;
}

inline void stringToUpper(const char* strcstr, char* deststr)
{
	while (*strcstr)
	{
		*deststr = toupper(*strcstr);
		strcstr++;
		deststr++;
	}
	*deststr = 0;
}

inline std::string stringToLowerEx(const std::string& srcstring)
{
	std::string dststring = srcstring;
	
	stringToLower(srcstring.c_str(), (char*)dststring.c_str());

	return dststring;
}

inline std::string stringToUpperEx(const std::string& srcstring)
{
	std::string dststring = srcstring;

	stringToUpper(srcstring.c_str(), (char*)dststring.c_str());

	return dststring;
}

#ifdef WIN32
#define strcasecmp _stricmp
#define snprintf _snprintf
#define strncasecmp _strnicmp
inline const char* strcasestr(const char* srcstr,const char* substr)
{
	if(srcstr == NULL || substr == NULL) 
	{
		return NULL;
	}
	char* srcptr = new(std::nothrow) char[strlen(srcstr) + 1];
	char* subptr = new(std::nothrow) char[strlen(substr) + 1];
	if(srcptr == NULL || subptr == NULL) 
	{
		Public::Base::SAFE_DELETEARRAY(srcptr);
		Public::Base::SAFE_DELETEARRAY(subptr); 
		return NULL;
	}
	stringToLower(srcstr,srcptr);
	stringToLower(substr,subptr);

	const char* ptmp = strstr(srcptr,subptr);
	if(ptmp != NULL)
	{
		ptmp = srcstr + (ptmp - srcptr);
	}
	Public::Base::SAFE_DELETEARRAY(srcptr);
	Public::Base::SAFE_DELETEARRAY(subptr);
	return ptmp;
}
#endif


namespace Public {
namespace Base {


///	安全的字符串连接
///	\param [out] dest:目标buffer,如果目标buffer没有'\0'结束，会设最后一个字符为'\0',返回0
///	\param [in]	dstBufLen,目标buffer空间大小,该函数最多写入dstBufLen-1个字符，并且在写入字符后面添加'\0'字符
///	\param [in] src:
///	\param [in] srcCopyCount: 拷贝src的长度
///		执行完成后，如果有copy，那么，Dest一定可以以0结束
///	\retval 返回的结果是从src copy到dest中的字符数目
size_t BASE_API strncat(char* dest, size_t dstBufLen, const char* src, const size_t srcCopyCount);

///	安全的字符串copy函数
///	\param [out] dst,目标buffer
///	\param [in]	dstBufLen,目标buffer空间大小,该函数最多写入dstBufLen-1个字符，并且在写入字符后面添加'\0'字符
///	\param [in]	src,源buffer
///	\param [in] srcCopyCount
///	\retval 要copy的字符数码,在dstBufLen-1空间允许的情况下，最多copy的字符数目为srcCopyCount,并且在后面添加'\0'字符
size_t BASE_API strncpy(char* dst, size_t dstBufLen, const char* src, size_t srcCopyCount);

///	增强的snprintf，保证'\0'，返回实际写入长度，
///	方便支持 len += snprintf_x( buf + len, maxlen - len, "xxxx", ... ); 的连续写法
///	当实际buffer不够时，保证\'0'，返回maxlen - 1，（原版snprintf，VC会返回-1且不保证'\0'，gcc会返回假设buffer足够时的写入长度）
///	（但返回maxlen-1时无法区分长度刚刚好还是出错了，可以简化都当出错处理，或者都当不出错不处理）
///	也可用于需要限长度且保证'\0'时的字符串拷贝，取代strncpy，（注意原版strncpy不保证'\0'）
///	即 strncpy( dst, src, siz - 1 ); dst[size - 1] = '\0'; 相当于 snprintf( dst, siz, "%s", src );
///	\param [out] buf 输出缓存
///	\param [in] maxlen 输出缓存最大字节数
///	\param [in] fmt 格式字符串
///	\retval 返回实际写入长度
int  BASE_API snprintf_x(char* buf, int maxlen, const char* fmt, ... );

/// Ansi(Gb2312)转换字符串为utf8格式
/// \param [in] inbuf 源字符串
/// \param [in]  inlen 源字符串的长度
/// \param [out] outbuf 目标字符串
/// \param [in] outlen 存储目标的字符串最大值
/// \retval > 0转换后的长度, 
///         <= 0 出错
int BASE_API ansi2utf8(char *inbuf,size_t inlen,char *outbuf,size_t outlen);


/// utf8转换字符串为 Ansi(Gb2312)格式
/// \param [in] inbuf 源字符串
/// \param [in]  inlen 源字符串的长度
/// \param [out] outbuf 目标字符串
/// \param [in] outlen 存储目标的字符串最大值
/// \retval > 0转换后的长度, 
///         <= 0 出错
int BASE_API utf82ansi(char *inbuf,size_t inlen,char *outbuf,size_t outlen);

/// Ansi(Gb2312) 转换字符串为utf8格式 扩展接口(linux 只支持转换后结果不超过1024bytes)
/// \param [in] inbuf 源字符串
/// \param [in]  inlen 源字符串的长度
/// \retval 空string 转换失败
///         非空string 转换结果
std::string BASE_API ansi2utf8Ex(char *inbuf,size_t inlen);


/// utf8转换字符串为 Ansi(Gb2312)格式  扩展接口(linux 只支持转换后结果不超过1024bytes)
/// \param [in] inbuf 源字符串
/// \param [in]  inlen 源字符串的长度
/// \retval 空string 转换失败
///         非空string 转换结果
std::string BASE_API utf82ansiEx(char *inbuf,size_t inlen);
/// 通过标识符解析url字段
/// \param [in] url 要解析的url串
/// \param [in] key 解析的字段名字
/// \param [in] headSymbol 解析的字段名称首部分割符
/// \param [in] tailSymbol 解析的字段名称尾部分割符
/// \param [out] value 解析后的结果返回值
bool BASE_API parseUrlByKey(std::string &url, char* key, char headSymbol,  char tailSymbol, std::string & value);


std::string BASE_API ansi2utf8Ex(const std::string& inString);

std::string BASE_API utf82ansiEx(const std::string& inString);

///Function3<std::string,const std::string& inputstr,const std::string& fromcharset,const std::string& tocharset>EncodeCallback
typedef Function3<std::string,const std::string&,const std::string&,const std::string& > CharsetEncodeCallback;

void BASE_API registeCharsetEncode(const CharsetEncodeCallback& callback);



} // namespace Base
} // namespace Public

#endif// __BASE_STRING_H__


