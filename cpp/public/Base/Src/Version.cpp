//
//  Copyright (c)1998-2012, Chongqing Xunmei Technology
//  All Rights Reserved.
//
//	Description:
//	$Id: Version.cpp 31 2013-02-05 04:30:06Z jiangwei $
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "Base/PrintLog.h"
#include "Base/Version.h"
#include "Base/String.h"
namespace {

static const char* month[] = {
	"Jan", "Feb", "Mar", "Apr", "May", "Jun",
		"Jul", "Aug", "Sep", "Oct", "Nov", "Dec",
};

} // namespace noname

namespace Xunmei {
namespace Base {

SystemTime AppVersion::appDate = Time::getCurrentTime();

AppVersion::AppVersion(const char* name, int major, int minor, int build, const char* svnString, const char* dateString)
{
	::strncpy(this->name, name, sizeof(this->name) - 1);
	this->Major = major;
	this->Minor = minor;
	this->Build = build;

	if (svnString[0] >= '0' && svnString[0] <= '9')
	{
		this->Revision = atoi(svnString);
	}
	else
	{
		this->Revision = 0;
		sscanf(svnString, "%*[$a-zA-Z:]%d", &this->Revision);
	}

	// 得到编译时间
	int i = 0;
	for (i = 0; i < 12; i++) {
		if( strncmp(month[i], dateString, 3) == 0 )
			break;
	}
	date.month = i+1;
	sscanf(dateString + 3, "%d %d", &date.day, &date.year);
}

void AppVersion::print() const
{
	infof("*************************************************\r\n");
	infof("%s Version:%d.%d.%d svn:%d Built in %d/%02d/%02d\r\n", name, this->Major, this->Minor, this->Build, this->Revision, date.year, date.month, date.day);
	infof("*************************************************\r\n\r\n");

}

void AppVersion::setAppDate(const char* dateString)
{
	int i = 0;
	for (i = 0; i < 12; i++) {
		if( strncmp(month[i], dateString, 3) == 0 )
			break;
	}
	appDate.month = i+1;
	sscanf(dateString + 3, "%d %d", &appDate.day, &appDate.year);
}

} // namespace Base
} // namespace Xunmei

