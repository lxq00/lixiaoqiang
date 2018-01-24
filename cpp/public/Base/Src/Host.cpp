#ifdef WIN32
#include <windows.h>
#include <IPHlpApi.h>
#endif
#include "Base/Host.h"
#include "Base/Guard.h"
#include "Base/PrintLog.h"
#include "../version.inl"


namespace Public{
namespace Base {

uint16_t Host::getAvailablePort(uint16_t startPort,SocketType type)
{
	static Mutex	getMutex;
	static uint16_t  getPortNum = 0;


	Guard locker(getMutex);

	uint16_t  activeport = 0;
	while(1)
	{
		activeport = startPort + getPortNum ++;

		if(checkPortIsNotUsed(activeport,type))
		{
			break;
		}
	}

	return activeport;
}
#ifdef WIN32

bool Host::checkPortIsNotUsed(uint16_t port,SocketType type)
{
	bool haveFind = false;

	if(type == SocketType_TCP)
	{
		PMIB_TCPTABLE pTcpTable = (MIB_TCPTABLE*) malloc(sizeof(MIB_TCPTABLE));
		DWORD dwSize = 0;
		DWORD dwRetVal = 0;

		if (GetTcpTable(pTcpTable, &dwSize, TRUE) == ERROR_INSUFFICIENT_BUFFER)
		{
			//����һ��Ҫ��ΪpTcpTable����һ�οռ䣬��Ϊԭ������Ŀռ��Ѿ������ˣ����»�ô�С����
			free(pTcpTable);
			pTcpTable = (MIB_TCPTABLE*) malloc ((UINT) dwSize);
		}
		if ((dwRetVal = GetTcpTable(pTcpTable, &dwSize, TRUE)) == NO_ERROR)
		{
			for (int i = 0; i < (int) pTcpTable->dwNumEntries; i++)
			{
				u_short cwPort = ntohs((u_short)pTcpTable->table[i].dwLocalPort);
				if(cwPort == port)
				{
					haveFind = true;
					break;
				}
			}
		}
		free(pTcpTable);
	}
	else
	{
		PMIB_UDPTABLE pTcpTable = (MIB_UDPTABLE*) malloc(sizeof(MIB_UDPTABLE));
		DWORD dwSize = 0;
		DWORD dwRetVal = 0;

		if (GetUdpTable(pTcpTable, &dwSize, TRUE) == ERROR_INSUFFICIENT_BUFFER)
		{
			//����һ��Ҫ��ΪpTcpTable����һ�οռ䣬��Ϊԭ������Ŀռ��Ѿ������ˣ����»�ô�С����
			free(pTcpTable);
			pTcpTable = (MIB_UDPTABLE*) malloc ((UINT) dwSize);
		}
		if ((dwRetVal = GetUdpTable(pTcpTable, &dwSize, TRUE)) == NO_ERROR)
		{
			for (int i = 0; i < (int) pTcpTable->dwNumEntries; i++)
			{
				uint32_t cwPort = ntohs((u_short)pTcpTable->table[i].dwLocalPort);
				if(cwPort == port)
				{
					haveFind = true;
					break;
				}
			}
		}
		free(pTcpTable);
	}

	return !haveFind;
}

#else
///���˿��Ƿ�ռ��
bool Host::checkPortIsNotUsed(uint16_t port,SocketType type)
{
	char buffer[256];

	if(type == SocketType_TCP)
	{	snprintf(buffer,255,"netstat -tnl");
	}
	else
	{
		snprintf(buffer,255,"netstat -unl");
	}
	FILE* fd = popen(buffer,"r");

	if(fd == NULL)
	{
		return true;
	}

	char flag[64];
	snprintf(flag,64,":%d",port);

	bool haveFind = false;
	const char* tmp = NULL;
	while((tmp = fgets(buffer,255,fd)) != NULL)
	{
		const char* ftmp = strstr(tmp,flag);
		if(ftmp == NULL)
		{
			continue;
		}
		const char* pftmpend = ftmp;
		while(*pftmpend == ':' || (*pftmpend >= '0' && *pftmpend <= '9')) pftmpend++;

		std::string flagstr(ftmp,pftmpend-ftmp);

		if(strcmp(flagstr.c_str(),flag) == 0)
		{
			haveFind = true;
			break;
		}
	}
	pclose(fd);


	return !haveFind;
}
#endif

#ifdef WIN32
#pragma once
//#ifndef PSAPI_VERSION
//#define PSAPI_VERSION 1 //����kernel32.dll�Ľӿڲ�����ȱ��
//#endif
//#include <psapi.h>
//#include <iphlpapi.h>
//#pragma comment(lib,"iphlpapi.lib")

#define SystemBasicInformation 0
#define SystemPerformanceInformation 2
#define SystemTimeInformation 3
typedef LONG (WINAPI *PROCNTQSI)(UINT, PVOID, ULONG, PULONG);

#define Li2Double(x) ((double)((x).HighPart) * 4.294967296E9 + (double)((x).LowPart))

typedef struct
{
	DWORD dwUnknown1;
	ULONG uKeMaximumIncrement;
	ULONG uPageSize;
	ULONG uMmNumberOfPhysicalPages;
	ULONG uMmLowestPhysicalPage;
	ULONG uMmHighestPhysicalPage;
	ULONG uAllocationGranularity;
	PVOID pLowestUserAddress;
	PVOID pMmHighestUserAddress;
	ULONG uKeActiveProcessors;
	BYTE bKeNumberProcessors;
	BYTE bUnknown2;
	WORD wUnknown3;
} SYSTEM_BASIC_INFORMATION;

typedef struct
{
	LARGE_INTEGER liIdleTime;
	DWORD dwSpare[76];
} SYSTEM_PERFORMANCE_INFORMATION;

typedef struct
{
	LARGE_INTEGER liKeBootTime;
	LARGE_INTEGER liKeSystemTime;
	LARGE_INTEGER liExpTimeZoneBias;
	ULONG uCurrentTimeZoneId;
	DWORD dwReserved;
} SYSTEM_TIME_INFORMATION;


//
//
//class WinGetCpuUsage
//{
//public:
//	WinGetCpuUsage(void)
//	{
//		processnum = Host::getProcessorNum();
//		NtQuerySystemInformation = (PROCNTQSI)GetProcAddress(GetModuleHandle("ntdll.dll"),"NtQuerySystemInformation");
//	}
//	~WinGetCpuUsage(void){}
//
//	int GetUsage(void)
//	{
//		SYSTEM_PERFORMANCE_INFORMATION SysPerfInfo;
//		SYSTEM_TIME_INFORMATION SysTimeInfo;
//		double dbIdleTime;
//		double dbSystemTime;
//		long status;
//		LARGE_INTEGER liOldIdleTime;
//		LARGE_INTEGER liOldSystemTime;
//
//		if (!NtQuerySystemInformation)
//		{
//			return 0;
//		}
//
//		status = NtQuerySystemInformation(SystemTimeInformation,&SysTimeInfo,sizeof(SysTimeInfo),0);
//		if (status!=NO_ERROR)
//		{
//			return 0;
//		}
//		status = NtQuerySystemInformation(SystemPerformanceInformation,&SysPerfInfo,sizeof(SysPerfInfo),NULL);
//		if (status != NO_ERROR)
//		{
//			return 0;
//		}
//		liOldIdleTime = SysPerfInfo.liIdleTime;
//		liOldSystemTime = SysTimeInfo.liKeSystemTime;
//
//		Thread::sleep(100);
//
//		status = NtQuerySystemInformation(SystemTimeInformation,&SysTimeInfo,sizeof(SysTimeInfo),0);
//		if (status!=NO_ERROR)
//		{
//			return 0;
//		}
//		status = NtQuerySystemInformation(SystemPerformanceInformation,&SysPerfInfo,sizeof(SysPerfInfo),NULL);
//		if (status != NO_ERROR)
//		{
//			return 0;
//		}
//
//		dbIdleTime = Li2Double(SysPerfInfo.liIdleTime) - Li2Double(liOldIdleTime);
//		dbSystemTime = Li2Double(SysTimeInfo.liKeSystemTime) - Li2Double(liOldSystemTime);
//
//		dbIdleTime = dbIdleTime / dbSystemTime;
//
//		dbIdleTime = 100.0 - dbIdleTime * 100.0 / (double)processnum + 0.5;
//		
//
//		int m_iUsage = (int)dbIdleTime;
//
//		return -1 < m_iUsage ? m_iUsage : 0;
//	}
//private:
//	double processnum;
//	PROCNTQSI NtQuerySystemInformation;
//};
//
//
//class WinGetNetWorkInfo:public Thread
//{
//public:
//	WinGetNetWorkInfo():Thread("WinGetNetWorkInfo")
//	{
//		dwSize = 0;
//		netloadin = 0;
//		netloadout = 0;
//		pIfTalble = NULL;
//		
//		/*	DWORD dwRet = GetIfTable(pIfTalble, &dwSize, TRUE);
//
//		if(dwRet != ERROR_INSUFFICIENT_BUFFER)
//		{
//		return;
//		}
//		pIfTalble = (MIB_IFTABLE *) new char[dwSize];
//		if(pIfTalble == NULL)
//		{
//		return;
//		}*/
//	
//	///	createThread();
//	}
//	~WinGetNetWorkInfo()
//	{
//		destroyThread();
//		SAFE_DELETE(pIfTalble);
//	}
//	bool getNetWorkLoad(uint64_t& in,uint64_t& out)
//	{
//		in = netloadin;
//		out = netloadout;
//
//		return (in == 0 || out == 0) ? false : true;
//	}
//	static WinGetNetWorkInfo* instance()
//	{
//		static WinGetNetWorkInfo netinfo;
//
//		return &netinfo;
//	}
//private:
//	void threadProc()
//	{
//		DWORD preInCount = 0, curInCount = 0;
//		DWORD preOutCount = 0, curOutCount = 0;
//
//		while(looping())
//		{
//			ULONG tablesize = 0;
//			if (GetIfTable(pIfTalble, &tablesize, TRUE) != NO_ERROR)
//			{
//				break;
//			}
//	
//			for (uint32_t i=0; i<pIfTalble->dwNumEntries;i++)
//			{
//				PMIB_IFROW ifRow = (PMIB_IFROW) &pIfTalble->table[i];
//				if (ifRow->dwType == IF_TYPE_ETHERNET_CSMACD && ifRow->dwOperStatus == IF_OPER_STATUS_OPERATIONAL)
//				{
//					curInCount += ifRow->dwInOctets;
//					curOutCount += ifRow->dwOutOctets;
//				}
//			}
//			if(preInCount != 0 && preOutCount != 0)
//			{
//				netloadin = curInCount - preInCount;
//				netloadout = curOutCount - preOutCount;
//			}
//			
//			preInCount = curInCount;
//			preOutCount = curOutCount;
//
//			SAFE_DELETE(pIfTalble);
//			Thread::sleep(2000);
//		}
//	}
//private:
//	MIB_IFTABLE *pIfTalble;
//	ULONG 		dwSize;
//	uint64_t 	netloadin;
//	uint64_t 	netloadout;
//};
//

int Host::getProcessorNum()
{
	SYSTEM_INFO si;

	GetSystemInfo(&si);

	return si.dwNumberOfProcessors;
}
//
//bool Host::getMemoryUsage(uint64_t& totalPhys,uint64_t& totalVirual,int& physusage,int& virualUsage)
//{
//	MEMORYSTATUSEX mem_info;
//	mem_info.dwLength = sizeof(MEMORYSTATUSEX);
//
//	if(!GlobalMemoryStatusEx(&mem_info))
//	{
//		return false;
//	}
//
//	totalPhys = mem_info.ullTotalPhys/(1024*1024);
//	totalVirual = mem_info.ullTotalVirtual/(1024*1024);
//	physusage = (int)((100 * (mem_info.ullTotalPhys - mem_info.ullAvailPhys)) / mem_info.ullTotalPhys);
//	virualUsage = (int)((100 * (mem_info.ullTotalVirtual - mem_info.ullAvailVirtual)) / mem_info.ullTotalVirtual);
//
//
//	return true;
//}
//
//uint16_t Host::getCPUUsage()
//{
//	WinGetCpuUsage cpuusage;
//
//	return cpuusage.GetUsage();
//}
bool Host::getDiskInfos(std::vector<DiskInfo>& infos)
{
	char buffer[256];

	DWORD len = GetLogicalDriveStringsA(255,buffer);
	for(unsigned i = 0; i < len ;i += 4)
	{
		DiskInfo info = {"","",DiskInfo::DiskType_Disk,0,0};

		{
			char diskid[32];
			sprintf(diskid,"%c",buffer[i]);
			info.Id = diskid;
			info.Name = info.Id;
		}		

		std::string pathname = std::string(info.Id + ":");

		{
			UINT type = GetDriveTypeA(pathname.c_str());
			if(type == DRIVE_FIXED)
			{
				info.Type = DiskInfo::DiskType_Disk;
			}
			else if(type == DRIVE_CDROM)
			{
				info.Type = DiskInfo::DiskType_CDRom;
			}
			else if(type == DRIVE_REMOVABLE)
			{
				info.Type = DiskInfo::DiskType_Remove;
			}
			else if(type == DRIVE_REMOTE)
			{
				info.Type = DiskInfo::DiskType_Network;
			}
		}
		{
			ULARGE_INTEGER freeavilable,totalnum,totalfreenum;

			BOOL flag = GetDiskFreeSpaceEx(pathname.c_str(),&freeavilable,&totalnum,&totalfreenum);
			
			if(flag && totalnum.QuadPart > 0)
			{
				info.TotalSize = totalnum.QuadPart;
				info.FreeSize = totalfreenum.QuadPart;
			}
			else
			{
				int error = GetLastError();
				logerror("GetDiskFreeSpaceEx %s error %d\r\n",pathname.c_str(),error);
			}
		}

		infos.push_back(info);
	}

	return true;
}
bool Host::getDiskInfo(int& num,uint64_t& totalSize,uint64_t& freeSize)
{
	num = 0;
	totalSize = 0;
	freeSize = 0;


	for(char name = 'A' ; name <= 'Z' ;name ++)
	{
		char diskname[32];
		sprintf(diskname,"%c:",name);

		ULARGE_INTEGER freeavilable,totalnum,totalfreenum;
		
		bool flag = GetDiskFreeSpaceEx(diskname,&freeavilable,&totalnum,&totalfreenum) ? true : false;
		if(!flag || totalnum.QuadPart <= 0)
		{
			continue;
		}

		totalSize += totalnum.QuadPart / (1024*1024);
		freeSize += totalfreenum.QuadPart / (1024*1024);

		num ++;
	}

	return true;
}
//
//bool Host::getNetwordLoad(uint64_t& inbps,uint64_t& outbps)
//{
//	return WinGetNetWorkInfo::instance()->getNetWorkLoad(inbps,outbps);
//}
#else
#include <sys/vfs.h>

//class LinuxGetCputUsage:public Thread
//{
//public:
//	LinuxGetCputUsage():Thread("LinuxGetCputUsage")
//	{
//		cpuusage = 0;
//	//	createThread();
//	}
//	~LinuxGetCputUsage()
//	{
//		destroyThread();
//	}
//	
//	uint16_t getCPUUsage()
//	{
//		return cpuusage;
//	}
//
//private:
//	void getCputState(uint64_t& idlecpu,uint64_t& totalcpu)
//	{
//		idlecpu = 0;
//		totalcpu = 0;
//
//		FILE* fp = fopen("/proc/stat","r");
//		if(fp == NULL)
//		{
//			return;
//		}
//
//		long long unsigned param[7] = {0};
//		fscanf(fp,"cpu %llu %llu %llu %llu %llu %llu %llu\n",&param[0],
//			&param[1],&param[2],&param[3],&param[4],&param[5],&param[6]);
//
//		fclose(fp);
//
//		idlecpu = param[3];
//		totalcpu = param[1] + param[2] + param[3] + param[4] + param[5] + param[6];
//	}
//	void threadProc()
//	{
//		uint64_t preidlecpu = 0,pretotalcpu  = 0;
//		uint64_t curridlecpu  = 0,currtotalcpu  = 0;
//		
//		while(looping())
//		{
//			getCputState(curridlecpu,currtotalcpu);
//
//			if(preidlecpu != 0 && pretotalcpu != 0)
//			{
//				cpuusage = (100 * ((currtotalcpu - pretotalcpu) - (curridlecpu - preidlecpu)) / (currtotalcpu - pretotalcpu)) ;
//			}
//			preidlecpu = curridlecpu;
//			pretotalcpu = currtotalcpu;
//
//			Thread::sleep(2000);
//		}
//	}
//	
//	uint16_t	cpuusage;
//};
//
//static LinuxGetCputUsage LinuxCpuUsage;
//
//class LinuxNetWorkUsage
//{
//public:
//	static bool getNetUsage(uint64_t& in,uint64_t& out)
//	{
//		FILE* fd = fopen("/proc/net/dev","r");
//		if(fd == NULL)
//		{
//			return false;
//		}
//
//		uint64_t totalinbps = 0;
//		uint64_t totaloutbps = 0;
//
//		Ifinfo ifc;
//		do{
//			if(fscanf(fd," %6[^:]:%u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u\r",
//				ifc.name,&ifc.r_bytes,&ifc.r_pkt,&ifc.r_err,&ifc.r_drop,&ifc.r_fifo,
//				&ifc.r_frame,&ifc.r_compr,&ifc.r_mcast,&ifc.x_bytes,&ifc.x_pkt,&ifc.x_err,
//				&ifc.x_drop,&ifc.x_fifo,&ifc.x_coll,&ifc.x_carrier,&ifc.x_compr) == 16)
//			{
//				totalinbps += ifc.r_bytes;
//				totaloutbps += ifc.x_bytes;
//			}
//		}while(!isend(fd));
//
//		fclose(fd);
//
//		in = totalinbps * 8;
//		out = totaloutbps * 8;
//
//
//		return true;
//	}
//private:
//	static bool isend(FILE*fp)
//	{
//		int ch = getc(fp);
//		
//		return (ch == EOF);
//	}
//
//	struct Ifinfo
//	{
//		char name[32];
//		uint32_t r_bytes,r_pkt,r_err,r_drop,r_fifo,r_frame;
//		uint32_t r_compr,r_mcast;
//		uint32_t x_bytes,x_pkt,x_err,x_drop,x_fifo,x_coll;
//		uint32_t x_carrier,x_compr;
//	};
//};

int Host::getProcessorNum()
{
	int processnum = sysconf(_SC_NPROCESSORS_CONF);

	return processnum;
}

//bool Host::getMemoryUsage(uint64_t& totalPhys,uint64_t& totalVirual,int& physusage,int& virualUsage)
//{
//	File file;
//	
//	if(!file.open("/proc/meminfo",File::modeRead))
//	{
//		return false;
//	}
//
//	int filesize = file.getLength();
//	char* buffer = new(std::nothrow) char[filesize + 1];
//
//	file.read(buffer,filesize);
//	buffer[filesize] = 0;
//
//	char* memtotalstart = strstr(buffer,"MemTotal:");
//	if(memtotalstart == NULL)
//	{
//		SAFE_DELETE(buffer);
//		return false;
//	}
//	char* memtotalend = strchr(memtotalstart,'\n');
//	if(memtotalend == NULL)
//	{
//		SAFE_DELETE(buffer);
//		return false;
//	}
//	*memtotalend = 0;
//
//	char* memfreestart = strstr(buffer,"MemFree:");
//	if(memfreestart == NULL)
//	{
//		SAFE_DELETE(buffer);
//		return false;
//	}
//	char* memfreeend = strchr(memfreestart,'\n');
//	if(memfreeend == NULL)
//	{
//		SAFE_DELETE(buffer);
//		return false;
//	}
//	*memfreeend = 0;
//
//	char*vmallocTotalstart = strstr(buffer,"VmallocTotal:");
//	if(vmallocTotalstart == NULL)
//	{
//		SAFE_DELETE(buffer);
//		return false;
//	}
//	char* vmallocTotalend = strchr(vmallocTotalstart,'\n');
//	if(vmallocTotalend == NULL)
//	{
//		SAFE_DELETE(buffer);
//		return false;
//	}
//	*vmallocTotalend = 0;
//
//	char*vmallocUsedstart = strstr(buffer,"VmallocUsed:");
//	if(vmallocUsedstart == NULL)
//	{
//		SAFE_DELETE(buffer);
//		return false;
//	}
//	char* vmallocUsedend = strchr(vmallocUsedstart,'\n');
//	if(vmallocUsedend == NULL)
//	{
//		SAFE_DELETE(buffer);
//		return false;
//	}
//	*vmallocUsedend = 0;
//
//	long long unsigned memtotal = 0;
//	long long unsigned memfree = 0;
//	long long unsigned vmalloctotal = 0;
//	long long unsigned vmallocused = 0;
//	
//	sscanf(memtotalstart,"MemTotal: %llu KB",&memtotal);
//	sscanf(memtotalstart,"MemFree: %llu KB",&memfree);
//	sscanf(memtotalstart,"VmallocTotal: %llu KB",&vmalloctotal);
//	sscanf(memtotalstart,"VmallocUsed: %llu KB",&vmallocused);
//
//	SAFE_DELETE(buffer);
//
//	totalPhys = memtotal / 1024;
//	totalVirual = vmalloctotal / 1024;
//	physusage = (100*(memtotal - memfree))/memtotal;
//	virualUsage = (100*vmallocused)/vmalloctotal;
//
//	return true;
//}
//
//uint16_t Host::getCPUUsage()
//{
//	return LinuxCpuUsage.getCPUUsage();
//}

bool Host::getDiskInfo(int& num,uint64_t& totalSize,uint64_t& freeSize)
{
	struct statfs stat;

	if(statfs("/",&stat) != 0)
	{
		return false;
	}

	num = 1;
	totalSize = stat.f_blocks * (stat.f_bsize / 1024) / 1024;
	freeSize = stat.f_bavail * (stat.f_bsize / 1024) / 1024;

	return true;
}

//bool Host::getNetwordLoad(uint64_t& inbps,uint64_t& outbps)
//{
//	return LinuxNetWorkUsage::getNetUsage(inbps,outbps);
//}

#endif
};
};
