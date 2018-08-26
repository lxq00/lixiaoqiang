//
//  Copyright (c)1998-2012, Public Technology
//  All Rights Reserved.
//
//	Description:
//	$Id: Process.h 3 2013-01-21 06:57:38Z  $

#ifndef __BASE_PROCESS_H__
#define __BASE_PROCESS_H__
#include "Base/Shared_ptr.h"
#ifdef WIN32
	#include <Windows.h>
#else
	#include <sys/types.h>
#endif

#include "Defs.h"
namespace Public{
namespace Base{


/// \interface Process 
/// \brief 进程生成类
#ifdef WIN32
	#define ProcessHandle HANDLE 
#else
	#define ProcessHandle pid_t  
#endif
	
class BASE_API Process
{
public:
	/// 构造函数
	Process() {};

	/// 虚析构函数
	virtual ~Process() {};

	/// 创建进程
	/// \param name [in] 进程名称 
	/// \param argc [in] 参数个数
	/// \param argv [in] 参数列表
	/// \param relation 关系进程，当创建进程关闭后，子进程自动退出
	/// \retval != NULL 成功
	/// \retval == NULL 失败
	static shared_ptr<Process> createProcess(const std::string& name,const std::deque<std::string>& argv = std::deque<std::string>(),bool relation = true);

	/// 获得进程的句柄
	/// \retval 句柄
	virtual ProcessHandle getHandle() = 0;
	

	///获取进程Pid
	virtual int getPid() = 0;

	///判断进程是否存在
	virtual bool exists() = 0;

	/// 
	/// 杀死进程
	/// \retval true 成功
	/// \retval false 失败
	virtual bool kill() = 0;

	/// 
	/// 杀死进程
	/// \retval true 成功
	/// \retval false 失败
	/// \param pid [in] 进程pid 
	static bool killByPid(int pid);

	/// 
	/// 检测进程是否存在
	/// \retval true 成功
	/// \retval false 失败
	/// \param pid [in] 进程pid 
	static bool existByPid(int pid);

	static uint32_t getProcessId();
};


class BASE_API ProcessInfo
{
public:
	ProcessInfo(){}
	virtual ~ProcessInfo(){}

	//获取指定进程的进程信息,process 进程对象
	static shared_ptr<ProcessInfo> getProcessInfo(const shared_ptr<Process>& process,uint32_t timeout = 100);

	//根据进程ID获取进程信息,pid为进程ID号
	static shared_ptr<ProcessInfo> getProcessInfo(int pid,uint32_t timeout = 100);

	//获取当前进程的进程信息
	static shared_ptr<ProcessInfo> getCurrProcessInfo(uint32_t timeout = 100);

	//提交内存，单位M
	virtual bool pagefileMemory(uint32_t& mem) = 0;

	//工作内存，单位M
	virtual bool workMemory(uint32_t& mem) = 0;

	//工作线程数
	virtual bool threadNum(uint16_t& num) = 0;

	//当前CPU使用率 0 ~ 100,timeout 使用超时时间 单位ms
	virtual bool cpuUsage(uint16_t& usage) = 0;
};


} // namespace Base
} // namespace Public

#endif //__BASE_PROCESS_H__

