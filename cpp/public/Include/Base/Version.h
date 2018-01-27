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


///�汾��Ϣ.
struct BASE_API Version
{
	int Major; //��Ҫ�汾.
	int Minor; //��Ҫ�汾.
	int Build; //�޶���.
	std::string Revision; //�޶���.


	Version()
	{
		Major = Minor = Build  = 0;
	}
	std::string toString() const
	{
		char ver[128];
		snprintf_x(ver, 127, "%d.%d.%d", Major, Minor, Build);

		return Revision == "" ? ver:std::string(ver)+"-"+ Revision;
	}
	bool parseString(const std::string& versionstr)
	{
#ifndef WIN32
#define sscanf_s sscanf
#endif
		Major = Minor = Build = 0;
		Revision = "";

		std::string versontmp = versionstr;
		const char* revsiontmp = strchr(versontmp.c_str(), '-');
		if (revsiontmp != NULL)
		{
			Revision = revsiontmp + 1;
			versontmp = std::string(versontmp.c_str(), revsiontmp - revsiontmp + 1);
		}
		
		if (sscanf_s(versontmp.c_str(), "%d.%d.%d.%d", &Major, &Minor, &Build, &Revision) == 4)
		{
			return true;
		}
		else if (sscanf_s(versontmp.c_str(), "%d.%d.%d", &Major, &Minor, &Build) == 3)
		{
			return true;
		}
		else if (sscanf_s(versontmp.c_str(), "%d.%d", &Major, &Minor) == 2)
		{
			return true;
		}
		else if (sscanf_s(versontmp.c_str(), "%d", &Major) == 1)
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
			(Major == version.Major && Minor == version.Minor && Build == version.Build && Revision != "" &&  version.Revision == "") ||
			(Major == version.Major && Minor == version.Minor && Build == version.Build && Revision != "" &&  version.Revision != "" && strcmp(Revision.c_str(),version.Revision.c_str()) < 0);
	}
	bool operator > (const Version& version) const
	{
		return Major > version.Major ||
			(Major == version.Major && Minor > version.Minor) ||
			(Major == version.Major && Minor == version.Minor && Build > version.Build) ||
			(Major == version.Major && Minor == version.Minor && Build == version.Build && Revision == "" && version.Revision != "") ||
			(Major == version.Major && Minor == version.Minor && Build == version.Build && Revision != "" &&  version.Revision != "" && strcmp(Revision.c_str(), version.Revision.c_str()) > 0);
	}
	bool operator == (const Version& version) const
	{
		return toString() == version.toString();
	}
};

/// \class Version
/// \brief �汾�࣬ÿ�����������ģ�鶼Ӧ�ö�����Եİ汾�ṹ����
class BASE_API AppVersion:public Version
{
public:
	
	char	name[32];			///< ���������ģ������
	SystemTime date;		///< �������ڣ�ʹ��__DATE__��

	static	SystemTime appDate;		///< Ӧ�ó����������

	/// ���캯�����汾����һ����Ϊȫ�ֶ�����
	/// \param name 	[in] ���������ģ������
	/// \param major 	[in] ���汾��
	/// \param minor 	[in] �ΰ汾��
	/// \param revision 	[in] �޶��汾��
	/// \param svnString 	[in] svn�汾�ŵ��ַ���
	/// \param dataString	[in]	ʱ���ַ���
	AppVersion(const std::string& name, int major, int minor, int build, const std::string& revision, const std::string& dateString);

	/// �汾��Ϣ��ӡ
	void print() const;

	/// ����Ӧ�ó����������
	/// \param dateString [in] ʱ���ַ���
	static void setAppDate(const std::string& dateString);

};

} // namespace Base
} // namespace Public

#endif //__BASE_VERSION_H__


