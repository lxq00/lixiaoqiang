//
//  Copyright (c)1998-2012, Public Technology
//  All Rights Reserved.
//
//	Description:
//	$Id: Base64.h 3 2013-01-21 06:57:38Z  $

#ifndef __BASE_BASE64_H__
#define __BASE_BASE64_H__

#include "Defs.h"
#include "Base/IntTypes.h"

namespace Public {
namespace Base {

/// \class Base64
/// \brief Base64算法相关的方法类
class BASE_API Base64 	
{
public:
	/// Base64 编码
	/// \param [in] src 编码前的数据缓存
	/// \reval 返回编码后的文本字节
	static std::string encode(const std::string& src);

	/// Base64 解码
	/// \param [in] src 解码后的数据缓存
	/// \reval 返回解码后的数据字节
	static std::string decode(const std::string& src);
};

} // namespace Base
} // namespace Public

#endif// __BASE_BASE64_H__

