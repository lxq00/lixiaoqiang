#include "Log/Log.h"
#include "log4cplus.h"
#include "Base/PrintLog.h"
#include "Base/BaseTemplate.h"
#include "Base/Version.h"
#include "../version.inl"
using namespace Public::Base;


void setAppName(const std::string& appName);

namespace Public{
namespace Log{

void  LogSystem::printLibVersion()
{
	Public::Base::AppVersion appVersion("Log Lib", r_major, r_minor, r_build, versionalias, __DATE__);
	appVersion.print();
}

static bool printLogOn = false;
void printlog(LOG_Level lev,char const* str)
{
	if(printLogOn)
	{
		Log_4cplus::instance()->print(lev,str);
	}
}

void LogSystem::init(const std::string& appName,const std::string& logpath,int logLevel)
{
	if(!printLogOn)
	{
#define LOGMAXLEVEL 6
		Log_4cplus::instance()->init(appName,logpath,LOGMAXLEVEL - logLevel);
//		setAppName(appName);

		printLogOn = true;
		setlogprinter(&printLogOn,printlog);
	}
}
void LogSystem::uninit()
{
	if(printLogOn)
	{
		printLogOn = false;
		cleanlogprinter(&printLogOn);

		Log_4cplus::instance()->uninit();
	}
}
std::string LogSystem::getLogPath()
{
	if(!printLogOn)
	{
		return "./";
	}
	return Log_4cplus::instance()->getLogPath();
}
std::string LogSystem::getAppLogPath()
{
	if(!printLogOn)
	{
		return "./";
	}
	return Log_4cplus::instance()->getAppLogPath();
}
};
};

