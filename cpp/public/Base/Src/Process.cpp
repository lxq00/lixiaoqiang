//
//  Copyright (c)1998-2012,  Public Technology
//  All Rights Reserved.
//
//	Description:
//	$Id: Process.cpp 120 2013-05-22 07:47:37Z  $
//

#ifndef WIN32
#include <unistd.h>
#include <errno.h>
#include <signal.h>
#include <sys/wait.h>
#include <sys/prctl.h>
#include <sys/types.h>
#else
#include "Base/Base.h"
#include <TlHelp32.h>

#define PSAPI_VERSION 1
#include <Psapi.h>
#pragma comment(lib,"Psapi.lib")

#endif
#include <string>
#include "Base/Process.h"
#include "Base/PrintLog.h"
#include "Base/String.h"



namespace Public{
namespace Base{

class ProcessImpl : public Process
{
public:
	/// 构造函数
	ProcessImpl(const std::string& name, int argc , const char *argv[],bool relation) 
	:handle(0)
	{
#ifdef WIN32
		jobhandle = 0;
		STARTUPINFOA StartupInfo = {0};
		PROCESS_INFORMATION ProcessInformation = {0};
		StartupInfo.dwFlags = STARTF_USESHOWWINDOW;
		StartupInfo.wShowWindow = SW_HIDE;
#ifdef DEBUG
		StartupInfo.wShowWindow = SW_SHOWNORMAL;
#endif
		char param[512] = "";
		for (int i = 0 ; i < argc; i++)
		{
			Base::snprintf_x(param, 512, "%s %s", param, argv[i]);
		}

		if (!CreateProcessA((LPSTR)name.c_str(),((argc > 0)?(LPSTR)(param):NULL), NULL, NULL, FALSE, 0, NULL, NULL, &StartupInfo, &ProcessInformation))
		{
			DWORD ret = GetLastError();
			logwarn("Can't open process:%s: errno (%d) \n", name.c_str(), ret);
			isok = false;
			handle = NULL;
			return ;
		}
		handle = ProcessInformation.hProcess;
		pid = ProcessInformation.dwProcessId;
		
		if(relation)
		{
			createRelation();
		}		

		CloseHandle(ProcessInformation.hThread);
#else
		handle = fork();
		pid = (int)handle;
		if (handle == 0) // sub process
		{
			if(relation)
			{
				prctl(PR_SET_PDEATHSIG,SIGHUP);
			}
			if (argc > 0)
			{
				char *args[argc + 2];
				int i = 1;
				args[0] = (char *)name.c_str();
				for (; i < argc + 1; i++)
				{
					args[i] = (char *)argv[i - 1];
				}	
				args[i] = NULL;
				execv(name.c_str(), args);
			}
			else
			{
				execl(name.c_str(), name.c_str(), (char *)NULL);
			}
			//logwarn("Can't run sub process:%s:%s \n", name, strerror(errno));
			_exit(1);
		}
		if (handle == -1)
		{
			logwarn("Can't open process:%s:%s \n", name.c_str() strerror(errno));
			isok = false;
			return ;
		}
#endif
		isok = true;
	};

	/// 虚析构函数
	~ProcessImpl() 
	{
		kill();
#ifdef WIN32
		if(jobhandle != 0)
		{
			CloseHandle(jobhandle);
		}
#endif
	};

	/// 杀死进程
	/// \retval true 成功
	/// \retval false 失败
	bool kill()
	{
#ifdef WIN32
		if (isok && handle)
		{
			if (TerminateProcess(handle,255) == TRUE)
			{
				WaitForSingleObject(handle, 10*1000);
				CloseHandle(handle);
				isok = false;			
				handle = NULL;
				return true;
			}
			WaitForSingleObject(handle, 10*1000);
			CloseHandle(handle);	
			isok = false;			
			handle = NULL;

		}
#else
		if (isok && handle > 0)
		{
			if (::kill(handle, SIGKILL) == 0)
			{
				int status;
  				waitpid(handle, &status, 0);
				isok = false;
				handle = -1;
				return true;
			}
			isok = false;
			handle = -1;
		}
#endif
	 	return false;
	}
	inline bool isrunning() const
	{
		return isok;
	}
	ProcessHandle getHandle()
	{
		return handle;
	}

	int getPid()
	{
		return (int)pid;
	}
	bool exists()
	{
		return Process::existByPid(pid);
	}

private:
	void createRelation()
	{
#ifdef WIN32
		jobhandle = CreateJobObject(NULL,NULL);
		if(jobhandle)
		{
			JOBOBJECT_EXTENDED_LIMIT_INFORMATION extlimit;
			extlimit.BasicLimitInformation.LimitFlags = JOB_OBJECT_LIMIT_KILL_ON_JOB_CLOSE;
			SetInformationJobObject(jobhandle,JobObjectExtendedLimitInformation,&extlimit,sizeof(extlimit));
			AssignProcessToJobObject(jobhandle,handle);
		}
#else

#endif
	}
private:
#ifdef WIN32
	HANDLE			jobhandle;
#endif
	ProcessHandle	handle;
	int				pid;

	bool isok;

};



shared_ptr<Process> Process::createProcess(const std::string& name, int argc , const char *argv[],bool relation)
	
{
	shared_ptr<ProcessImpl> impl;
	impl = new ProcessImpl(name, argc, argv, relation);
	if (impl->isrunning())
	{
		return impl;
	}else{
		return shared_ptr<Process>();
	}

	return shared_ptr<Process>();
}

bool Process::existByPid(int pid)
{
#ifdef WIN32
	HANDLE hsnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS,0);
	bool isFind = false;

	PROCESSENTRY32 pe;
	pe.dwSize = sizeof(pe);
	if(!Process32First(hsnapshot,&pe))
	{
		CloseHandle(hsnapshot);
		return false;
	}
	if(pe.th32ProcessID == pid)
	{
		CloseHandle(hsnapshot);
		return true;
	}

	while(1)
	{
		pe.dwSize = sizeof(pe);
		if(!Process32Next(hsnapshot,&pe))
		{
			break;
		}
		if(pe.th32ProcessID == pid)
		{
			isFind = true;
			break;
		}
	}
	CloseHandle(hsnapshot);

	return isFind;
#else
	return ::kill(pid,0) != -1;
#endif
}

bool Process::killByPid(int pid)
{
#ifdef WIN32
	HANDLE hp = OpenProcess(SYNCHRONIZE | PROCESS_TERMINATE, FALSE, pid);
	if (WaitForSingleObject(hp, 5000) != WAIT_OBJECT_0)
	{
		TerminateProcess(hp, 0);
	}
	return true;
#else
	return ::kill((pid_t)pid, 9) == 0;
#endif
}



struct BaseProcessInfo:public ProcessInfo
{
	uint32_t pagefileMem;
	uint32_t workmem;
	uint32_t threads;
	uint32_t cpu;

	BaseProcessInfo(shared_ptr<Process> process,uint32_t timeout):pagefileMem(-1),workmem(-1),threads(-1),cpu(-1)
	{
#ifdef WIN32
		getProcessInfo(process->getHandle(),timeout);
#else
		getProcessInfo(process->getPid(),timeout);
#endif
	}
	BaseProcessInfo(uint32_t pid,uint32_t timeout):pagefileMem(-1),workmem(-1),threads(-1),cpu(-1)
	{
#ifdef WIN32
		if(pid == 0) pid = GetCurrentProcessId();
		HANDLE hProcessHandle = ::OpenProcess(PROCESS_ALL_ACCESS, FALSE, pid);
		getProcessInfo(hProcessHandle,timeout);
		CloseHandle(hProcessHandle);
#else
		if(pid == 0) pid = getpid();
		getProcessInfo(pid,timeout);
#endif
	}

	//提交内存，单位M
	bool pagefileMemory(uint32_t& mem)
	{
		if(pagefileMem != (uint32_t)-1)
		{
			mem = pagefileMem;
		}
		
		return pagefileMem != (uint32_t)-1;
	}

	//工作内存，单位M
	bool workMemory(uint32_t& mem)
	{
		if (workmem != (uint32_t)-1)
		{
			mem = workmem;
		}
		return workmem != (uint32_t)-1;
	}

	//工作线程数
	bool threadNum(uint16_t& num)
	{
		if (threads != (uint32_t)-1)
		{
			num = threads;
		}
		return threads != (uint32_t)-1;
	}

	//当前CPU使用率 0 ~ 100,timeout 使用超时时间 单位ms
	bool cpuUsage(uint16_t& usage)
	{
		if (cpu != (uint32_t)-1)
		{
			usage = cpu;
		}
		
		return cpu != (uint32_t)-1;
	}
#ifdef WIN32
	void getProcessInfo(HANDLE procHandle,uint32_t timeout)
	{
		getProcessMemInfo(procHandle, pagefileMem, workmem);
		getProcessThreads(procHandle, threads);
		cpu = getcpuage(procHandle,timeout);
	}

	// 获取进程内存大小
	bool getProcessMemInfo(HANDLE procHandle, uint32_t & pagefileMemory, uint32_t & workMemory)
	{
		PROCESS_MEMORY_COUNTERS memoryInfo = { 0 };
		memoryInfo.cb = sizeof(PROCESS_MEMORY_COUNTERS);
		if (!GetProcessMemoryInfo(procHandle, &memoryInfo, sizeof(PROCESS_MEMORY_COUNTERS)))
		{
			return false;
		}

		pagefileMemory = memoryInfo.PagefileUsage / 1024 / 1024;
		workMemory = memoryInfo.WorkingSetSize / 1024 / 1024;
		return true;
	}
	int getProcessThreads(HANDLE hProcess, uint32_t& threads)
	{
		DWORD dwProcessID = GetProcessId(hProcess);
		if (dwProcessID == NULL)
		{
			return false;
		}

		int threadnum = 0;
		HANDLE hProcessShot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
		PROCESSENTRY32 info = { 0 };
		info.dwSize = sizeof(PROCESSENTRY32);
		if (Process32First(hProcessShot, &info))
		{
			if (dwProcessID == info.th32ProcessID)
			{
				threadnum = info.cntThreads;
			}
			else
			{
				while (Process32Next(hProcessShot, &info))
				{
					if (dwProcessID == info.th32ProcessID)
					{
						threadnum = info.cntThreads;
						break;
					}
				}
			}
		}
		CloseHandle(hProcessShot);
		threads = threadnum;

		return true;
	}
	/// 时间转换
	uint64_t file_time_2_utc(const FILETIME* ftime)
	{
		LARGE_INTEGER li;

		assert(ftime);
		li.LowPart = ftime->dwLowDateTime;
		li.HighPart = ftime->dwHighDateTime;
		return li.QuadPart;
	}


	/// 获得CPU的核数
	int get_processor_number()
	{
		SYSTEM_INFO info;
		GetSystemInfo(&info);
		return (int)info.dwNumberOfProcessors;
	}

	int getcpuage(HANDLE hProcess,uint32_t timeout)
	{
		int64_t time1_sys,time1_now,time2_sys,time2_now;
		if(getcpuusageitem(hProcess,time1_now,time1_sys) != 0)
		{
			return -1;
		}

		Sleep(timeout);

		if(getcpuusageitem(hProcess,time2_now,time2_sys) != 0)
		{
			return -1;
		}

		int64_t system_time_delta = time2_sys - time1_sys;  
		int64_t time_delta = time2_now - time1_now;  

		if (time_delta == 0)  
			return -1;  

		// We add time_delta / 2 so the result is rounded.  
		int cpu = (int)((system_time_delta * 100 + time_delta / 2) / time_delta);  

		return cpu;
	}

	int getcpuusageitem(HANDLE hProcess,int64_t& nowtime,int64_t& systime)  
	{  
		//cpu数量  
		static int processor_count_ = -1;  
		//上一次的时间  
		static int64_t last_time_ = 0;  
		static int64_t last_system_time_ = 0;  


		FILETIME now;  
		FILETIME creation_time;  
		FILETIME exit_time;  
		FILETIME kernel_time;  
		FILETIME user_time;  
		int64_t system_time;  
		int64_t time;  
		int64_t system_time_delta;  
		int64_t time_delta;  

		int cpu = -1;  


		if(processor_count_ == -1)  
		{  
			processor_count_ = get_processor_number();  
		}  

		GetSystemTimeAsFileTime(&now);  

		if (!GetProcessTimes(hProcess, &creation_time, &exit_time,&kernel_time, &user_time))  
		{  
			// We don't assert here because in some cases (such as in the Task Manager)  
			// we may call this function on a process that has just exited but   we have  
			// not yet received the notification.  
			return -1;  
		}  
		systime = (file_time_2_utc(&kernel_time) + file_time_2_utc(&user_time))   /  processor_count_;  
		nowtime = file_time_2_utc(&now);  

		return 0;

		if ((last_system_time_ == 0) || (last_time_ == 0))  
		{  
			// First call, just set the last values.  
			last_system_time_ = system_time;  
			last_time_ = time;  
			return -1;  
		}  

		system_time_delta = system_time - last_system_time_;  
		time_delta = time - last_time_;  

		assert(time_delta != 0);  

		if (time_delta == 0)  
			return -1;  

		// We add time_delta / 2 so the result is rounded.  
		cpu = (int)((system_time_delta * 100 + time_delta / 2) / time_delta);  
		last_system_time_ = system_time;  
		last_time_ = time;  
		return cpu;  
	}  
#else
#define PROCESS_ITEM 14//进程CPU时间开始的项数

typedef struct        //声明一个occupy的结构体
{
	unsigned int user;  //从系统启动开始累计到当前时刻，处于用户态的运行时间，不包含 nice值为负进程。
	unsigned int nice;  //从系统启动开始累计到当前时刻，nice值为负的进程所占用的CPU时间
	unsigned int system;//从系统启动开始累计到当前时刻，处于核心态的运行时间
	unsigned int idle;  //从系统启动开始累计到当前时刻，除IO等待时间以外的其它等待时间iowait (12256) 从系统启动开始累计到当前时刻，IO等待时间(since 2.5.41)
}total_cpu_occupy_t;

typedef struct
{
	pid_t pid;//pid号
	unsigned int utime;  //该任务在用户态运行的时间，单位为jiffies
	unsigned int stime;  //该任务在核心态运行的时间，单位为jiffies
	unsigned int cutime;//所有已死线程在用户态运行的时间，单位为jiffies
	unsigned int cstime;  //所有已死在核心态运行的时间，单位为jiffies
}process_cpu_occupy_t;

unsigned int get_cpu_process_occupy(const pid_t p)
{
	char file[64] = {0};//文件名
	process_cpu_occupy_t t;

	FILE *fd;         //定义文件指针fd
	char line_buff[1024] = {0};  //读取行的缓冲区
	sprintf(file,"/proc/%d/stat",p);//文件中第11行包含着    
	fd = fopen (file, "r"); //以R读的方式打开文件再赋给指针fd
	if(fd == NULL)
	{
		return 0;
	}
	fgets (line_buff, sizeof(line_buff), fd); //从fd文件中读取长度为buff的字符串再存到起始地址为buff这个空间里

	sscanf(line_buff,"%u",&t.pid);//取得第一项
	const char* q = get_items(line_buff,PROCESS_ITEM);//取得从第14项开始的起始指针
	sscanf(q,"%u %u %u %u",&t.utime,&t.stime,&t.cutime,&t.cstime);//格式化第14,15,16,17项

	fclose(fd);     //关闭文件fd
	return (t.utime + t.stime + t.cutime + t.cstime);
}
unsigned int get_cpu_total_occupy()
{
	FILE *fd;         //定义文件指针fd
	char buff[1024] = {0};  //定义局部变量buff数组为char类型大小为1024
	total_cpu_occupy_t t;

	fd = fopen ("/proc/stat", "r"); //以R读的方式打开stat文件再赋给指针fd
	if(fd == NULL)
	{
		return 0;
	}
	fgets (buff, sizeof(buff), fd); //从fd文件中读取长度为buff的字符串再存到起始地址为buff这个空间里
	/*下面是将buff的字符串根据参数format后转换为数据的结果存入相应的结构体参数 */
	char name[16];//暂时用来存放字符串
	sscanf (buff, "%s %u %u %u %u", name, &t.user, &t.nice,&t.system, &t.idle);

	fclose(fd);     //关闭文件fd
	return (t.user + t.nice + t.system + t.idle);
}


float get_pcpu(pid_t p,uint32_t timeout)
{
	float pcpu = 0;
#if 0
	{
		float processnum = sysconf(_SC_NPROCESSORS_CONF);

		float cpu = 0;

		char buffer[256];
		sprintf(buffer,"top -b -n 1 -p %d |grep %d | awk '{print $9}'",p,p);
		FILE* fd = popen(buffer,"r");
		if(fd != NULL)
		{
			int readlen = fread(buffer,1,128,fd);
			if(readlen <= 0 || readlen > 128)
			{
				readlen = 0;
			}
			buffer[readlen] = 0;
			fclose(fd);
		}
		cpu = atof(buffer);
		pcpu =  cpu / processnum;
	}
#else
{
	unsigned int total1, total2, idle1, idle2;
	total1 = get_cpu_total_occupy();
	idle1 = get_cpu_process_occupy(p);

	usleep(500000);

	total2 = get_cpu_total_occupy();
	idle2 = get_cpu_process_occupy(p);


	pcpu = 100.0*(idle2 - idle1) / (total2 - total1);
}
#endif

	return pcpu;
}

const char* get_items(const char* buffer,int ie)
{
	assert(buffer);
	const char* p = buffer;//指向缓冲区
	int len = strlen(buffer);
	int count = 0;//统计空格数
	if (1 == ie || ie < 1)
	{
		return p;
	}
	int i;

	for (i=0; i<len; i++)
	{
		if (' ' == *p)
		{
			count++;
			if (count == ie-1)
			{
				p++;
				break;
			}
		}
		p++;
	}

	return p;
}

bool getProcessInfo(pid_t pid,uint32_t timeout)
{
	cpu = (uint32_t)get_pcpu(pid,timeout);

	char buffer[64];
	snprintf_x(buffer,63, "/proc/%d/status", pid);

	if (access(buffer, R_OK) != 0)
	{
		return false;
	}
	FILE* fd = fopen(buffer, "rb");
	if (fd != NULL)
	{
		char buffer[128];
		while (fgets(buffer, 128, fd) != NULL)
		{
			if (getValue(buffer, "VmSize:", pagefileMem))
			{
				pagefileMem /= 1024;
			}
			else if (getValue(buffer, "VmRSS:", workmem))
			{
				workmem /= 1024;
			}
			else if (getValue(buffer, "Threads:", threads))
			{
			}
		}
		fclose(fd);
	}

	return true;
}
bool getValue(const char* buffer, const std::string& key, uint32_t& val)
{
	char* tmp = (char*)strstr(buffer, key.c_str());
	if (tmp == NULL)
	{
		return false;
	}
	tmp += key.length();
	while (*tmp)
	{
		if (*tmp == ' ' || *tmp == '\t')
		{
			tmp++;
		}
		else
		{
			break;
		}
	}

	char* endtmp = (char*)strchr(tmp, ' ');
	if(endtmp != NULL)
	{
		*endtmp = 0;
	}
	else
	{
		char* endtmp = tmp + strlen(tmp) - 1;
		while (endtmp > tmp)
		{
			if (*endtmp == ' ' || *endtmp == '\t' || *endtmp == '\r' || *endtmp == '\n')
			{
				*endtmp = 0;
				endtmp--;
			}
			else
			{
				break;
			}
		}
	}

	val = atoi(tmp);

	return true;
}
#endif
};

//获取指定进程的进程信息,process 进程对象
shared_ptr<ProcessInfo> ProcessInfo::getProcessInfo(const shared_ptr<Process>& process,uint32_t timeout)
{
	shared_ptr<ProcessInfo> info(new BaseProcessInfo(process,timeout));
	return info;
}

//根据进程ID获取进程信息,pid为进程ID号
shared_ptr<ProcessInfo> ProcessInfo::getProcessInfo(int pid,uint32_t timeout)
{
	shared_ptr<ProcessInfo> info(new BaseProcessInfo(pid,timeout));
	return info;
}

//获取当前进程的进程信息
shared_ptr<ProcessInfo> ProcessInfo::getCurrProcessInfo(uint32_t timeout)
{
	return getProcessInfo(0,timeout);
}

} // namespace Base
} // namespace Public


