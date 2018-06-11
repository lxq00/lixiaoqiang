#ifndef __COREDUMP_H__
#define __COREDUMP_H__
#include "Base/Time.h"
#include "Base/IntTypes.h"
#include "Defs.h"

namespace Public {
namespace Base {


class BASE_API CoreDump
{
public:
	//自动生成core dump,save path 为dump文件保存路径
	static void audoCoreDump(const std::string& savePath="./");

	//取消自动dump
	static void cleanAudoCoreDump();

	//获取dump崩溃记录
	static bool getCoreDumpRecord(std::list<Time>& record,const std::string& savePath="./");

	static bool cleanAndSyncReocrd(const std::string& savePath="./");

	//清除dump信息，maxDay保存多少天的记录，maxNum保存多少个dump
	static bool cleanCoreDumpReocrdByTimeout(uint32_t maxDay = 15,uint32_t maxNum = 3,const std::string& savePath="./");

	//清除多有dump及先关记录
	static bool cleanCoreDumpReocrd(const std::string& savePath="./");

	//打包压缩dump文件
	static bool getCoreDumpFileList(std::list<std::string>& filelist,const std::string& savePath="./");
};


}
}
#endif //__COREDUMP_H__
