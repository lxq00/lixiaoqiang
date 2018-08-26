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
#include "Base/Base.h"
using namespace Public::Base;

namespace Public{
namespace Log{

class LogInternal :public Thread
{
public:
	LogInternal();
	~LogInternal();

	//log level ¥Ú”°µ»º∂  0:all 1:debug 2:info 3:warn 4:error 5:fatal 6:off
	bool init(const std::string& appName,const std::string& logpath, LOG_Level logLevel);
	bool uninit();
	void print(const LogPrintInfo& info);
private:
	void threadProc();
	void checkAndDeleteLog();
private:
	Mutex			logMutex;
	std::string		logPath;
	std::string		appName;
	struct LogInfo
	{
		std::string 	logStr;
		LOG_Level			loglev;
		std::string			logprex;
	};
	LOG_Level			printlev;
	std::list<LogInfo> 	logList;
	Semaphore			logSem;
	uint64_t			currCheckLogDay;
};

};
};


