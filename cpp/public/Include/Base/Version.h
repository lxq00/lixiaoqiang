//
//  Copyright (c)1998-2014, Public Technology
//  All Rights Reserved.
//
//
//	Description:
//	$Id: Version.h 3 2013-01-21 06:57:38Z jiangwei $

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
	int Build; //内部版本号.
	int Revision; //修订号.


	Version()
	{
		Major = Minor = Build = Revision = 0;
	}
	std::string toString()
	{
		char ver[128];
		snprintf_x(ver, 127, "%d.%d.%d.%d", Major, Minor, Build, Revision);
		return std::string(ver);
	}
	bool parseString(const std::string& versionstr)
	{
#ifndef WIN32
#define sscanf_s sscanf
#endif
		Major = Minor = Build = Revision = 0;
		if (sscanf_s(versionstr.c_str(), "%d.%d.%d.%d", &Major, &Minor, &Build, &Revision) == 4)
		{
			return true;
		}
		else if (sscanf_s(versionstr.c_str(), "%d.%d.%d", &Major, &Minor, &Build) == 3)
		{
			return true;
		}
		else if (sscanf_s(versionstr.c_str(), "%d.%d", &Major, &Minor) == 2)
		{
			return true;
		}
		else if (sscanf_s(versionstr.c_str(), "%d", &Major) == 1)
		{
			return true;
		}
		return false;
	}
	bool operator < (const Version& version) const
	{
		return Major < version.Major ||
			(Major == version.Major && Minor < version.Minor) ||
			(Major == version.Major && Minor == version.Minor && Build < version.Build) ||
			(Major == version.Major && Minor == version.Minor && Build == version.Build && Revision < version.Revision);
	}
	bool operator > (const Version& version) const
	{
		return Major > version.Major ||
			(Major == version.Major && Minor > version.Minor) ||
			(Major == version.Major && Minor == version.Minor && Build > version.Build) ||
			(Major == version.Major && Minor == version.Minor && Build == version.Build && Revision > version.Revision);
	}
	bool operator == (const Version& version) const
	{
		return Major == version.Major && Minor == version.Minor && Build == version.Build && Revision == version.Revision;
	}
};

/// \class Version
/// \brief 版本类，每个组件、程序、模块都应该定义各自的版本结构对象
class BASE_API AppVersion:public Version
{
public:
	
	char	name[32];			///< 组件或程序或模块名称
	SystemTime date;		///< 编译日期，使用__DATE__宏

	static	SystemTime appDate;		///< 应用程序编译日期

	/// 构造函数，版本对象一般作为全局对象构造
	/// \param name 	[in] 组件或程序或模块名称
	/// \param major 	[in] 主版本号
	/// \param minor 	[in] 次版本号
	/// \param revision 	[in] 修订版本号
	/// \param svnString 	[in] svn版本号的字符串
	/// \param dataString	[in]	时间字符串
	AppVersion(const char* name, int major, int minor, int build, const char* svnString, const char* dateString);

	/// 版本信息打印
	void print() const;

	/// 设置应用程序编译日期
	/// \param dateString [in] 时间字符串
	static void setAppDate(const char* dateString);

};

} // namespace Base
} // namespace Public

#endif //__BASE_VERSION_H__


