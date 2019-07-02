//
//  Copyright (c)1998-2012, Public Technology
//  All Rights Reserved.
//
//
//	Description:
//	$Id: Guid.h 89 2013-05-08 10:43:01Z  $
//
//


#ifndef __BASE_GUID_H__
#define __BASE_GUID_H__

#include "Defs.h"
#include "Base/IntTypes.h"

namespace Public{
namespace Base {


/// \class Guid
/// \brief 统一的Guid
///
class BASE_API Guid
{
public:
	/// 创建guid
	static Guid createGuid();

	/// 创建guid
	static Guid *createGuidEx();

	/// 缺省构造函数
	/// \note "00000000-0000-0000-0000-000000000000" 
    Guid();
		
  	/// 构造函数
  	/// \param guid [in] guid的地址
  	/// \param isstream [in] 是否是流串(16bytes) 还是字符串(37 bytes)
	/// \note  字符串模式(isstream == false)时,guid 必须保证内存空间是>=37且最后一个为'\0'否则即便前面36个bytes正确,也会设置失败(windows)
	Guid(const char *guid, bool isstream = true);

	/// 拷贝构造函数
	Guid(const Guid& other);

	/// 赋值
	Guid& operator=(const Guid& other);
	
	/// 比较
	bool operator<(const Guid& other) const;

	/// 等于
	bool operator==(const Guid &other) const;

	/// 等于 流串
	bool operator==(const char * other) const;
	
	/// 不等于
	bool operator!=(const Guid &other) const;

	/// 不等于 流串
	bool operator!=(const char * other) const;	
	
	/// 析构函数
	virtual ~Guid();

	/// 是否有效
	/// \retval true 有效 
	///         false 无效
	/// \note 主要用于 采用传入字符串方式构造后，检查是否有效
	bool valid() const;

	/// 设置Guid
  	/// \param guid [in] guid的地址
  	/// \param isstream [in] 是否是流串(16bytes) 还是字符串(37 bytes)
	/// \retval true 成功
	///         false 失败
	/// \note 字符串模式(isstream == false) guid 必须保证内存空间是>=37且最后一个为'\0',否则即便前面36个bytes正确,也会设置失败(windows)
	bool setGuid(const char *guid, bool isstream = true);
	
	/// 获得Guid
	/// \retval != NULL 流串(16bytes)
	///         == NULL 无效
	const unsigned char *getBitStream() const;

	/// 获得Guid
	/// \retval string.lenth() <= 0 无效	
	///         其他字符串 (36个bytes)
	std::string getStringStream() const;

	/// 获得Guid
	/// \param str [out] 字符串
	/// \retval true 成功
	///         false 失败
	/// \note str 保证至少有 37个bytes的空间
	bool getStringStream(char *str) const;
	
private:
	unsigned char buffer[16];
};


} // namespace Base
} // namespace Public

#endif //__BASE_GUID_H__

