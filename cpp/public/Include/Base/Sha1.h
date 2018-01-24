//
//  Copyright (c)1998-2012, Xunmei Technology
//  All Rights Reserved.
//
//	Description:
//	$Id: Md5.h 3 2013-01-21 06:57:38Z  $
//

#ifndef __BASE_SHA1_H__
#define __BASE_SHA1_H__

#include <stddef.h>
#include "Base/IntTypes.h"
#include "Defs.h"


namespace Xunmei {
namespace Base {


/// \class Sha1 
/// \brief Sha1 算法处理类
class BASE_API Sha1
{
public:
	typedef enum
	{
		REPORT_HEX = 0,
		REPORT_DIGIT = 1,
		REPORT_HEX_SHORT = 2
	}REPORT_TYPE;
public:
	Sha1();
	~Sha1();

	bool hashFile(const std::string& file);

	///	数据编码。
	///	\param [in] data 数据指针
	/// \param [in] size 数据长度
	bool input(uint8_t const* data, size_t size);

	///数据一切都放好，计算hash值
	bool Final();

	/// 获取结果
	/// \param [in] result 40位编码数据
	bool result(uint32_t ret[5]);

	///格式化结果值
	bool report(std::string& str,REPORT_TYPE type);
private:
	class Sha1Internal;
	Sha1Internal* internal;
};


} // namespace Base
} // namespace Xunmei

#endif// __BASE_SHA1_H__


