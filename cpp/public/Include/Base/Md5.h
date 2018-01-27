//
//  Copyright (c)1998-2012, Public Technology
//  All Rights Reserved.
//
//	Description:
//	$Id: Md5.h 3 2013-01-21 06:57:38Z jiangwei $
//

#ifndef __BASE_MD5_H__
#define __BASE_MD5_H__

#include <stddef.h>
#include "Base/IntTypes.h"
#include "Defs.h"


namespace Public {
namespace Base {


/// \class MD5 
/// \brief MD5 算法处理类
class BASE_API Md5
{
	Md5(Md5 const&);
	Md5& operator=(Md5 const&);

public:
	/// 构造函数
	Md5();

	/// 析构函数
	~Md5();

	///	初始化
	void init();

	///	追加要做MD5摘要的数据，该接口可调用多次，分别追加数据。
	///	\param [in] data 数据指针
	/// \param [in] size 数据长度
	void update(uint8_t const* data, size_t size);

	///	生成16字节MD5摘要
	///	\param [out]  摘要输出缓存，不可小于16字节！
	std::string final();

	///	生成32字节MD5摘要哈希(16进制小写字符串)
	///	\param [out] hash 摘要哈希输出缓存，不可小于32字节！
	std::string hex();

private:
	struct Md5Internal;
	Md5Internal* internal;

};


} // namespace Base
} // namespace Public

#endif// __BASE_MD5_H__


