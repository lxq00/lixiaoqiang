//
//  Copyright (c)1998-2012, Chongqing Technology
//  All Rights Reserved.
//
//	Description:
//	$Id: PrintLog.cpp 11 2013-01-22 08:42:03Z $
//

#ifdef WIN32
#include <Windows.h>
#endif
#include "Base/PrintLog.h"
#include "Base/Mutex.h"
#include "Base/Thread.h"
#include "Base/Semaphore.h"
#include <list>
using namespace Public::Base;

namespace Public{
namespace Log{

class Log_4cplus:public Thread
{
public:
	Log_4cplus();
	~Log_4cplus();

	//log level ¥Ú”°µ»º∂  0:all 1:debug 2:info 3:warn 4:error 5:fatal 6:off
	bool init(const std::string& appName,const std::string& logpath,int logLevel);
	bool uninit();
	static Log_4cplus* instance();
	void print(LOG_Level lev,const char* str);
	std::string getLogPath();
	std::string getAppLogPath();
private:
	void threadProc();
	void checkAndDeleteLog();
private:
	Mutex			logMutex;
	std::string 	cfgFile;
	std::string		searchLogDirStr;
	std::string		logPath;
	struct LogInfo
	{
		std::string 	logStr;
		LOG_Level 		lev;
	};
	std::list<LogInfo> 	logList;
	Semaphore			logSem;
	uint64_t			currCheckLogDay;
};

};
};


