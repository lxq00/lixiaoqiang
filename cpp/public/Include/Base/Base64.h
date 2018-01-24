//
//  Copyright (c)1998-2012, Public Technology
//  All Rights Reserved.
//
//	Description:
//	$Id: Base64.h 3 2013-01-21 06:57:38Z jiangwei $

#ifndef __BASE_BASE64_H__
#define __BASE_BASE64_H__

#include "Defs.h"

namespace Public {
namespace Base {

/// \class Base64
/// \brief Base64算法相关的方法类
class BASE_API Base64 	
{
public:
	/// 计算存放编码后数据的缓冲区字节数
	static int encodelen(int len);

	/// Base64 编码
	/// \param [in] dst 存放编码后的文本缓存
	/// \param [in] src 编码前的数据缓存
	/// \param [in] len 编码前的数据字节数
	/// \reval 返回编码后的文本字节数, 不包含字符串结束符
	static int encode(char* dst, const char* src, int len);

	/// 计算存放解码后数据的缓冲区字节数
	/// \param src [in] 需要解码的数据源指针
	/// \reval 解码后的缓冲数据大小
	static int decodelen(const char* src);

	/// Base64 解码
	/// \param [in] dst 解码前的文本缓存
	/// \param [in] src 解码后的数据缓存
	/// \reval 返回解码后的数据字节数
	static int decode(char* dst, const char* src);
};

} // namespace Base
} // namespace Public

#endif// __BASE_BASE64_H__

