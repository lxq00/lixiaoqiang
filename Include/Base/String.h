//
//  Copyright (c)1998-2014, Public Technology
//  All Rights Reserved.
//
//
//	Description:
//	$Id: String.h 227 2013-10-30 01:10:30Z  $

#ifndef __BASE_STRING_H__
#define __BASE_STRING_H__

#include <stdio.h>
#include <string.h>
#include "BaseTemplate.h"
#include "Defs.h"
#include <string>
#include "Func.h"


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


class BASE_API String
{
	struct StringInternal;
	StringInternal* internal;
public:
	String(const shared_ptr<IMempoolInterface>& mempool = shared_ptr<IMempoolInterface>());
	String(const char* str, const shared_ptr<IMempoolInterface>& mempool = shared_ptr<IMempoolInterface>());
	String(const char* str, size_t len, const shared_ptr<IMempoolInterface>& mempool = shared_ptr<IMempoolInterface>());
	String(const std::string& str, const shared_ptr<IMempoolInterface>& mempool = shared_ptr<IMempoolInterface>());
	String(const String& str);
	~String();

	const char* c_str() const;
	size_t length() const;
	void resize(size_t size);



	String& operator = (const char* str);
	String& operator = (const std::string& str);
	String& operator = (const String& str);

	String& operator +=(const char* str);
	String& operator +=(const std::string& str);
	String& operator +=(const String& str);

	String& append(const char* str,size_t size);
	String& append(const std::string& str);
	String& append(const String& str);
public:
	//字符串转小写
	static std::string tolower(const std::string& src);

	//字符串转大写
	static std::string toupper(const std::string& src);


	/// Ansi(Gb2312) 转换字符串为utf8格式 扩展接口(linux 只支持转换后结果不超过1024bytes)
	/// \param [in] src 源字符串
	/// \retval 空string 转换失败
	///         非空string 转换结果
	static std::string ansi2utf8(const std::string& src);

	/// utf8转换字符串为 Ansi(Gb2312)格式  扩展接口(linux 只支持转换后结果不超过1024bytes)
	/// \param [in] src 源字符串
	/// \retval 空string 转换失败
	///         非空string 转换结果
	static std::string utf82ansi(const std::string& src);

	//split() 方法用于把一个字符串分割成字符串数组
	static std::vector<std::string> split(const std::string& src, const std::string& howmany);
	static std::vector<std::string> split(const char* src,size_t len, const std::string& howmany);

	//左右清理,默认清理空格/回车/tabl
	static std::string strip(const std::string& src,const std::vector<std::string>& howmany = defaultHowmany());
	// 右清理,
	static std::string ltrip(const std::string& src, const std::vector<std::string>& howmany = defaultHowmany());
	// 右清理,
	static std::string rtrip(const std::string& src, const std::vector<std::string>& howmany = defaultHowmany());

	//左右清理,默认清理空格/回车/tabl
	static std::vector<std::string> defaultHowmany();

	//从字符串中从头开始查找指定字符串，返回位置，未找到返回-1
	static size_t indexOf(const std::string& src, const std::string& fromindex);
	static size_t indexOf(const char* src, size_t len, const std::string& fromindex);

	//从字符串中从头开始查找指定字符串，返回位置，未找到返回-1 忽略大小写
	static size_t indexOfByCase(const std::string& src, const std::string& fromindex);
	static size_t indexOfByCase(const char* src, size_t len, const std::string& fromindex);

	//从字符串中从尾开始查找指定字符串，返回位置，未找到返回-1
	static size_t lastIndexOf(const std::string& src, const std::string& fromindex);
	static size_t lastIndexOf(const char* src, size_t len, const std::string& fromindex);

	//从字符串中从尾开始查找指定字符串，返回位置，未找到返回-1 忽略大小写
	static size_t lastIndexOfByCase(const std::string& src, const std::string& fromindex);
	static size_t lastIndexOfByCase(const char* src, size_t len, const std::string& fromindex);

	static std::string replace(const std::string& src,const std::string& substr,const std::string& replacement);
    
    static std::string snprintf_x(int maxsize, const char* fmt, ...);
};

#ifdef WIN32
#define strcasecmp _stricmp
#define snprintf _snprintf
#define strncasecmp _strnicmp


inline	const char* strcasestr(const char* srcstr, const char* substr)
{
	if (srcstr == NULL || substr == NULL)
	{
		return NULL;
	}
	std::string srcstrtmp = String::tolower(srcstr);
	std::string substrtmp = String::tolower(substr);

	const char* ptmp = strstr(srcstrtmp.c_str(), substrtmp.c_str());
	if (ptmp != NULL)
	{
		ptmp = srcstr + (ptmp - srcstrtmp.c_str());
	}
	return ptmp;
}

#endif

} // namespace Base
} // namespace Public

#endif// __BASE_STRING_H__


