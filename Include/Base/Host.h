//
//  Copyright (c)1998-2012,  Public Technology
//  All Rights Reserved.
//
//	Description:
//	$Id: host.h 216 2015-12-15 11:33:55Z  $
//
#ifndef __NETWORKHOST_H__
#define __NETWORKHOST_H__
#include "Defs.h"
#include "Base/IntTypes.h"

namespace Public{
namespace Base {

///主机相关的类
class BASE_API Host
{
public:
	struct DiskInfo
	{
		std::string		Id;
		std::string		Name;
		std::string		Alias; //别名
		std::string		SerialNumber;//序列号
		enum{
			DiskType_Usb = 0,
			DiskType_CDRom,
			DiskType_Remove,
			DiskType_Disk,
			DiskType_Network,
		}Type;
		enum {
			FormatType_Unkown = 0,
			FormatType_FAT,
			FormatType_NTFS,
			FormatType_ext4,
		}FormatType;
		uint64_t		TotalSize;
		uint64_t		FreeSize;
	};
	typedef enum {
		SocketType_TCP,
		SocketType_UDP,
	}SocketType;

	struct IPInfo
	{
		std::string Ip;
		std::string Netmask;
		std::string Gateway;
	};

	struct NetworkInfo :public IPInfo
	{
		std::string AdapterName;
		std::string Description;
		std::string Mac;
	};
public:
	///获取一个可用的端口号 10000开始 失败返回0
	///type 在某协议下判断端口是否可用
	static uint16_t getAvailablePort(uint16_t startPort = 10000,SocketType type = SocketType_TCP);

	static bool getNowUsedPortMap(std::set<uint16_t>& portmap,SocketType type = SocketType_TCP);

	///检测端口是否被占用
	///type 在某协议下判断端口是否可用
	static bool checkPortIsNotUsed(uint16_t port,SocketType type = SocketType_TCP);

	///获取CPU 核数
	static int getProcessorNum();

	///获取进程内存使用率、返回内存使用大小
//	static bool getMemoryUsage(uint64_t& totalPhys,uint64_t& totalVirual,int& physusage,int& virualUsage);

	///获取cpu使用率 0 ~ 100 
//	static uint16_t getCPUUsage();

	///获取磁盘信息
	/// num 磁盘总数
	/// totalSize 磁盘总大小 M为单位
	/// freeSize  剩余大小
	static bool getDiskInfo(int& num,uint64_t& totalSize,uint64_t& freeSize);

	static bool getDiskInfos(std::vector<DiskInfo>& infos);

	///获取网络负载
//	static bool getNetwordLoad(uint64_t& inbps,uint64_t& outbps);

	static bool getNetworkInfos(std::map<std::string, NetworkInfo>& infos,std::string& defaultMac);

	static bool setIPInfo(const NetworkInfo& info,const std::string& adapterName = "");

	static std::string	guessMyIpaddr(const std::string& destip = "");
	static bool guessMyIpInfo(NetworkInfo& info);

	//获取系统位数
	static uint32_t getSystemBits();
};

};
};


#endif //__BASEHOST_H__

