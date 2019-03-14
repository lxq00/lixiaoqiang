//
//  Copyright (c)1998-2012, Chongqing Technology
//  All Rights Reserved.
//
//	Description:
//	$Id: log4cplus.cpp 11 2014年6月5日13:50:35Z $
//

#include "LogInternal.h"

using namespace Public::Base;

namespace Public{
namespace Log{

#define MAXLOGFILESIZE		1024*1024
#define ONEDAYTIME			(24*60*60)
#define ONEHOURE			(60*60)

LogInternal::LogInternal():Thread("LogInternal"),currCheckLogDay(0)
{}
LogInternal::~LogInternal()
{}

bool LogInternal::init(const std::string& _appName,const std::string& _logpath, LOG_Level logLevel)
{
	appName = _appName;
	printlev = logLevel;
#ifndef DEBUG
	printlev = LOG_Level_INFO;
#endif
	if (_logpath != "")
	{
		logPath = _logpath + "/" + appName;
	}
	else
	{
#ifdef WIN32
		char systempath[513];
		GetSystemDirectory(systempath, 512);
		const char* syspathtmp = strcasestr(systempath, "Windows\\system32");
		if (syspathtmp == NULL)
		{
			syspathtmp = strcasestr(systempath, "Windows/system32");
		}
		if (syspathtmp != NULL)
		{
			*(char*)syspathtmp = 0;
			std::string systemdatapath = std::string(systempath) + "ProgramData\\ZVan";
			logPath = systemdatapath + "\\IVS-III\\Log\\"+appName;
		}
#else
		logPath = std::string("/home/.ZVan/IVS-III/Log/")+ appName;
#endif
	}
	createThread();
	
	return true;
}
bool LogInternal::uninit()
{
	cancelThread();
	logSem.post();
	destroyThread();
	
	return true;
}


#define MAXLOGSTACKSIZE		10000
void LogInternal::print(const LogPrintInfo& pinfo)
{
	LogInfo info;

	info.logStr = pinfo.logstr;
	info.loglev = pinfo.loglev;
	info.logprex = pinfo.logprex;

	Guard locker(logMutex);

	if(logList.size() >= MAXLOGSTACKSIZE)
	{
		logList.front();
		logList.pop_front();
	}
	
	logList.push_back(info);
	logSem.post();
}

void LogInternal::threadProc()
{
	int logindex = 0;
	uint64_t logsize = 0;
	uint64_t curfilehour = 0;
	FILE* fd = NULL;
	std::string currlogfilename;
	while(looping())
	{
		checkAndDeleteLog();

		//检测文件是否改切换
		if (fd != NULL)
		{
			Time nowtime = Time::getCurrentTime();
			uint64_t nowtimestmap = nowtime.makeTime();
			uint64_t nowhour = (uint64_t)(nowtimestmap / ONEHOURE);

			if (nowhour != curfilehour)
			{
				fclose(fd);
				logindex = 0;
				fd = NULL;
				if (logsize == 0)
				{
					File::remove(currlogfilename);
				}
                logsize = 0;
			}
			else if (logsize >= MAXLOGFILESIZE)
			{
				fclose(fd);
				logindex++;
				logsize = 0;
				fd = NULL;
			}
		}

		logSem.pend(500);

		if (fd == NULL)
		{
			if (!File::access(logPath.c_str(), File::accessExist))
			{
				File::makeDirectory(logPath.c_str());
			}

			Time nowtime = Time::getCurrentTime();
			curfilehour = (uint64_t)(nowtime.makeTime() / ONEHOURE);

			char buffer[256] = { 0 };
			snprintf(buffer, 255, "%s/%s_logFile_%d_%04d-%02d-%02d_%02d%02d%02d_%02d.log", logPath.c_str(), appName.c_str(), Process::getProcessId(),
				nowtime.year, nowtime.month, nowtime.day, nowtime.hour, nowtime.minute, nowtime.second, logindex);
			logsize = 0;
			currlogfilename = buffer;
			fd = fopen(currlogfilename.c_str(), "wb+");
		}

		if (fd == NULL)
		{
			continue;
		}

		LogInfo info;
		{
			Guard locker(logMutex);
			if(logList.size() <= 0)
			{
				continue;
			}
			info = logList.front();
			logList.pop_front();
		}

		

		if (info.loglev > printlev) continue;
		
		int writelen = fwrite(info.logprex.c_str(), 1, info.logprex.length(), fd);
		if (writelen > 0) logsize += writelen;
		
		char* logstr = (char*)info.logStr.c_str();
		int logstrlen = info.logStr.length();
		while (logstrlen > 0 && (logstr[logstrlen - 1] == '\n' || logstr[logstrlen - 1] == '\r')) logstrlen--;
		
		writelen = fwrite(logstr, 1, logstrlen, fd);
		if (writelen > 0) logsize += writelen;

		writelen = fwrite("\r\n", 1, 2, fd);
		if (writelen > 0) logsize += writelen;
		
		fflush(fd);
	}

	if (fd != NULL)
	{
		fclose(fd);
		if (logsize == 0)
		{
			File::remove(currlogfilename);
		}
	}
}


void LogInternal::checkAndDeleteLog()
{
#define MAXLOGSAVYDAYS		30

	Public::Base::Time currTime = Public::Base::Time::getCurrentTime();

	if(currTime.makeTime() /ONEDAYTIME  == currCheckLogDay)
	{
		return;
	}

	uint64_t lastLogFileTime = currTime.makeTime() - MAXLOGSAVYDAYS * ONEDAYTIME;

	Directory browser(logPath);
	while(1)
	{
		Directory::Dirent dir;
		if(!browser.read(dir))
		{
			break;
		}
		if(dir.Type == Directory::Dirent::DirentType_File)
		{
			if (dir.FileSize == 0)
			{
				File::remove((dir.Path + PATH_SEPARATOR + dir.Name).c_str());
			}
			else if(dir.CreatTime.makeTime() < lastLogFileTime)
			{
				File::remove((dir.Path + PATH_SEPARATOR + dir.Name).c_str());
			}
		}
	}
	currCheckLogDay = currTime.makeTime() /ONEDAYTIME ;
}

}; // na
}; // namespace Public

