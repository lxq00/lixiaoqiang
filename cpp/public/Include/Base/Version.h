//
//  Copyright (c)1998-2014, Public Technology
//  All Rights Reserved.
//
//
//	Description:
//	$Id: Version.h 3 2013-01-21 06:57:38Z  $

#ifndef __BASE_VERSION_H__
#define __BASE_VERSION_H__

#include "Base/Defs.h"
#include "Base/Time.h"
#include "Base/String.h"
#include <stdio.h>

namespace Public {
namespace Base {


///版本信息.
struct BASE_API Version
{
	int Major; //主要版本.
	int Minor; //次要版本.
	int Build; //修订号.
	std::string Revision; //修订号.

public:
	Version();
	std::string toString() const;
	bool parseString(const std::string& versionstr);
	bool operator < (const Version& version) const;
	bool operator > (const Version& version) const;
	bool operator == (const Version& version) const;
};

/// \class Version
/// \brief 版本类，每个组件、程序、模块都应该定义各自的版本结构对象
class BASE_API AppVersion:public Version
{
public:
	
	std::string	name;			///< 组件或程序或模块名称
	SystemTime date;		///< 编译日期，使用__DATE__宏

	static	SystemTime appDate;		///< 应用程序编译日期

	/// 构造函数，版本对象一般作为全局对象构造
	/// \param name 	[in] 组件或程序或模块名称
	/// \param major 	[in] 主版本号
	/// \param minor 	[in] 次版本号
	/// \param revision 	[in] 修订版本号
	/// \param svnString 	[in] svn版本号的字符串
	/// \param dataString	[in]	时间字符串
	AppVersion(const std::string& name, int major, int minor, int build, const std::string& revision, const std::string& dateString);

	/// 版本信息打印
	void print() const;

	/// 设置应用程序编译日期
	/// \param dateString [in] 时间字符串
	static void setAppDate(const std::string& dateString);

};

} // namespace Base
} // namespace Public

#endif //__BASE_VERSION_H__


