#ifndef __PUBLIC_LOG_H__
#define __PUBLIC_LOG_H__
#include "Log/Defs.h"
#include<string>
using namespace std;

namespace Public{
namespace Log{

class LOG_API LogSystem
{
public:
	/// 打印 Log库 版本信息
	static void  printLibVersion();
	//appName 应用程序名称、不能带符号 level 打印等级  0:off 1:fatal 2:error 3:warn 4:info 5:trace 6:debug 
	static void init(const std::string& appName,const std::string& logpath = "",int logLevel = 6);
	static void uninit();
	static std::string getLogPath();
	static std::string getAppLogPath();
};

};
};

#endif //__PUBLIC_LOG_H__
