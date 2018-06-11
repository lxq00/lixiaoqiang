//
//  Copyright (c)1998-2012, Chongqing Technology
//  All Rights Reserved.
//
//	Description:
//	$Id: log4cplus.cpp 11 2014年6月5日13:50:35Z $
//

#ifdef WIN32
#include <Windows.h>
#include <process.h>
#else
#include <sys/types.h>
#include <unistd.h>
#endif
#include <string>
#include <string.h>
#include "log4cplus/logger.h"
#include "log4cplus/consoleappender.h"
#include "log4cplus/fileappender.h"
#include "log4cplus/layout.h"
#include "log4cplus/loggingmacros.h"
#include "log4cplus/configurator.h"
#include "Base/Base.h"
#include "log4cplus.h"

using namespace log4cplus;
using namespace log4cplus::helpers;
using namespace Public::Base;

namespace Public{
namespace Log{

#ifdef DEBUG
#define PRINTMINTRANS	"DEBUG"
#define WRITETRANS		"TRACE"
#else
#define PRINTMINTRANS	"INFO"
#define WRITETRANS		"INFO"
#endif
///////////////////////////////////////////////Log配置缺省配置信息///////////////////////////
 
#define  logCfgFileInfo(path,app,pid) \
	std::string("log4cplus.rootLogger=debug, ALL_MSGS, CONSOLE\r\n ") + \
	std::string("\r\n ") + \
	std::string("#FileAppender\r\n ") + \
	std::string("log4cplus.appender.ALL_MSGS=log4cplus::AsyncAppender\r\n ") + \
	std::string("log4cplus.appender.ALL_MSGS=log4cplus::DailyRollingFileAppender\r\n ") + \
	std::string("log4cplus.appender.ALL_MSGS.File="+path+"/" + app + "_logFile_"+pid+".log\r\n") + \
	std::string("#log4cplus.appender.ALL_MSGS.layout=log4cplus::TTCCLayout\r\n ") + \
	std::string("log4cplus.appender.ALL_MSGS.filters.1=log4cplus::spi::LogLevelRangeFilter\r\n ") + \
	std::string("log4cplus.appender.ALL_MSGS.filters.1.LogLevelMin="WRITETRANS"\r\n ") + \
	std::string("log4cplus.appender.ALL_MSGS.filters.1.LogLevelMax=FATAL\r\n ") + \
	std::string("log4cplus.appender.ALL_MSGS.filters.1.AcceptOnMatch=true\r\n ") + \
	std::string("log4cplus.appender.ALL_MSGS.layout=log4cplus::PatternLayout\r\n ") + \
	std::string("log4cplus.appender.ALL_MSGS.layout.ConversionPattern=%D{%Y-%m-%d %H:%M:%S,%Q} %-5p - %m%n\r\n ") + \
	std::string("log4cplus.appender.ALL_MSGS.DatePattern=%Y-%m-%d-%H\r\n") + \
	std::string("#MONTHLY, WEEKLY, DAILY,TWICE_DAILY, HOURLY, MINUTELY:分别表示按月、按周、按天、半天、按小时、按分钟的时间跨度分割日志文件 \r\n") + \
	std::string("log4cplus.appender.ALL_MSGS.Schedule=HOURLY\r\n") + \
	std::string("#log4cplus.appender.ALL_MSGS.Append=ture\r\n") + \
	std::string("log4cplus.appender.ALL_MSGS.MaxBackupIndex=100\r\n") + \
	std::string("log4cplus.appender.ALL_MSGS.MaxBackupDays=7\r\n") + \
	std::string("log4cplus.appender.ALL_MSGS.MaxSizeRollBackups=100\r\n") + \
	std::string("log4cplus.appender.ALL_MSGS.MaxFileSize=1024KB\r\n") + \
	std::string("\r\n ") + \
	std::string("# CONSOLE\r\n ") + \
	std::string("log4cplus.appender.CONSOLE=log4cplus::AsyncAppender\r\n ") + \
	std::string("log4cplus.appender.CONSOLE=log4cplus::ConsoleAppender\r\n ") + \
	std::string("log4cplus.appender.CONSOLE.layout=log4cplus::TTCCLayout\r\n ") + \
	std::string("log4cplus.appender.CONSOLE.filters.1=log4cplus::spi::LogLevelRangeFilter\r\n ") + \
	std::string("log4cplus.appender.CONSOLE.filters.1.LogLevelMin="PRINTMINTRANS"\r\n ") + \
	std::string("log4cplus.appender.CONSOLE.filters.1.LogLevelMax=FATAL\r\n ") + \
	std::string("log4cplus.appender.CONSOLE.filters.1.AcceptOnMatch=true\r\n ") + \
	std::string("log4cplus.appender.CONSOLE.layout=log4cplus::PatternLayout\r\n ") + \
	std::string("log4cplus.appender.CONSOLE.layout.ConversionPattern=%D{%Y-%m-%d %H:%M:%S} %-5p - %m%n\r\n ") + \
	std::string("\r\n ") + \
	std::string("\r\n ") + \
	std::string("\r\n ");
	
///////////////////////////////////////////////Log配置缺省配置信息 END///////////////////////////


class Log4cplusEventInit
{
	Log4cplusEventInit():initFlag(false){}
public:
	~Log4cplusEventInit()
	{
		Logger::uninit();
	}

	static Log4cplusEventInit* instance()
	{
		static Log4cplusEventInit event;

		return &event;
	}
	std::string init(const std::string& appName,const std::string& logpath,int logLevel,std::string& logPath)
	{
		if(initFlag)
		{
			return searchLogDirStr;
		}

		searchLogDirStr = File::getExcutableFileFullPath() + "/Log";

		if (!File::access(searchLogDirStr,File::accessExist))
		{
			File::makeDirectory(searchLogDirStr.c_str());
		}

		char pidstr[32];
#ifdef WIN32
		sprintf(pidstr,"%d",_getpid());
#else
		sprintf(pidstr,"%d",getpid());
#endif
		File file;
		std::string cfgFilename = searchLogDirStr + "/LogConfigInfo.properties_" + pidstr;

		//if(!File::access(cfgFilename.c_str(),File::accessExist))
		{
			file.open(cfgFilename.c_str(), File::modeCreate | File::modeReadWrite);

			std::string configdata = logCfgFileInfo(searchLogDirStr,appName,pidstr);

			file.write((void*)configdata.c_str(),configdata.size());
			file.flush();
			file.close();
		}	

		log4cplus::PropertyConfigurator::doConfigure(cfgFilename);

		Logger::getRoot().setLogLevel(logLevel*10000);

		{
			File::remove(cfgFilename.c_str());
		}


		initFlag = true;

		return searchLogDirStr;
	}
private:
	bool			initFlag;
	std::string		searchLogDirStr;
};


Log_4cplus::Log_4cplus():Thread("Log_4cplus"),currCheckLogDay(0)
{}
Log_4cplus::~Log_4cplus()
{}

bool Log_4cplus::init(const std::string& appName,const std::string& logpath,int logLevel)
{
	searchLogDirStr = Log4cplusEventInit::instance()->init(appName,logpath,logLevel,logPath);

	createThread();
	
	return true;
}
bool Log_4cplus::uninit()
{
	cancelThread();
	logSem.post();
	destroyThread();
	
	return true;
}
Log_4cplus* Log_4cplus::instance()
{
	static Log_4cplus log;
	return &log;
}
std::string Log_4cplus::getLogPath()
{
	return logPath;
}
std::string Log_4cplus::getAppLogPath()
{
	return searchLogDirStr;
}

#define MAXLOGSTACKSIZE		10000
void Log_4cplus::print(LOG_Level tag,const char* str)
{
	Guard locker(logMutex);

	if(logList.size() >= MAXLOGSTACKSIZE)
	{
		logList.front();
		logList.pop_front();
	}
	LogInfo info;

	info.logStr = str;
	info.lev = tag;

	logList.push_back(info);
	logSem.post();
}

void Log_4cplus::threadProc()
{
	LogInfo info;
	while(looping())
	{
		checkAndDeleteLog();

		logSem.pend(500);
		{
			Guard locker(logMutex);
			if(logList.size() <= 0)
			{
				continue;
			}
			info = logList.front();
			logList.pop_front();
		}

		char* logstr = (char*)info.logStr.c_str();

		while(1)
		{
			int len = strlen(logstr);
			if(len <= 0)
			{
				break;
			}
			if(logstr[len - 1] == '\r' || logstr[len - 1] == '\n')
			{
				logstr[len - 1] = 0;
			}
			else
			{
				break;
			}
		}
		
		switch(info.lev) 
		{ 
		case LOG_Level_TRACE:
			LOG4CPLUS_TRACE(log4cplus::Logger::getRoot(), logstr);
			break;
		case LOG_Level_DEBUG:
			LOG4CPLUS_DEBUG(log4cplus::Logger::getRoot(), logstr);
			break;
		case LOG_Level_INFO:
			LOG4CPLUS_INFO(log4cplus::Logger::getRoot(), logstr);
			break;
		case LOG_Level_WARN:
			LOG4CPLUS_WARN(log4cplus::Logger::getRoot(), logstr);
			break;
		case LOG_Level_ERROR:
			LOG4CPLUS_ERROR(log4cplus::Logger::getRoot(), logstr);
			break;
		case LOG_Level_FATAL:
			LOG4CPLUS_FATAL(log4cplus::Logger::getRoot(), logstr);
			break;
		}
	}
}


void Log_4cplus::checkAndDeleteLog()
{
#define ONEDAYTIME			(24*60*60)
#define MAXLOGSAVYDAYS		30

	Public::Base::Time currTime = Public::Base::Time::getCurrentTime();

	if(currTime.makeTime() /ONEDAYTIME  == currCheckLogDay)
	{
		return;
	}

	uint64_t lastLogFileTime = currTime.makeTime() - MAXLOGSAVYDAYS * ONEDAYTIME;

	FileBrowser browser(searchLogDirStr);
	browser.order(FileBrowser::OrderType_CrteatTime,FileBrowser::OrderMode_Asc);
	uint32_t index = 0;
	while(1)
	{
		Directory::Dirent dir;
		if(!browser.read(dir,index))
		{
			break;
		}
		if(dir.Type == Directory::Dirent::DirentType_File)
		{
			if(dir.CreatTime.makeTime() < lastLogFileTime)
			{
				File::remove((dir.Path + "/" + dir.Name).c_str());
			}
		}
		index ++;
	}
	currCheckLogDay = currTime.makeTime() /ONEDAYTIME ;
}

}; // na
}; // namespace Public

