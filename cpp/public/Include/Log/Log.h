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
	/// ��ӡ Log�� �汾��Ϣ
	static void  printLibVersion();
	//appName Ӧ�ó������ơ����ܴ����� level ��ӡ�ȼ�  0:off 1:fatal 2:error 3:warn 4:info 5:trace 6:debug 
	static void init(const std::string& appName,const std::string& logpath = "",int logLevel = 6);
	static void uninit();
	static std::string getLogPath();
	static std::string getAppLogPath();
};

};
};

#endif //__PUBLIC_LOG_H__
