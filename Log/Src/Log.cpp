#include "Log/Log.h"
#include "LogInternal.h"
#include "../version.inl"
using namespace Public::Base;

namespace Public{
namespace Log{

void  LogSystem::printLibVersion()
{
	Public::Base::AppVersion appVersion("Log Lib", r_major, r_minor, r_build, versionalias, __DATE__);
	appVersion.print();
}

static bool printLogOn = false;
static shared_ptr<LogInternal> log;

void printlog(const LogPrintInfo& info)
{
	shared_ptr<LogInternal> logptr = log;
	if(printLogOn && logptr != NULL)
	{
		logptr->print(info);
	}
}

void LogSystem::init(const std::string& appName,const std::string& logpath, LOG_Level logLevel)
{
	if(!printLogOn)
	{
		log = make_shared<LogInternal>();
		log->init(appName,logpath,logLevel);

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
		log = NULL;
	}
}

};
};

