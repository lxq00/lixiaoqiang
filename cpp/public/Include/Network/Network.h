//
//  Copyright (c)1998-2012, Chongqing Xunmei Technology
//  All Rights Reserved.
//
//	Description:
//	$Id: network.h 216 2015-12-15 11:33:55Z lixiaoqiang $
//
#ifndef __XUNMEINETWORK_H__
#define __XUNMEINETWORK_H__
#include "Network/Defs.h"
#include "Network/Host.h"
#include "Network/NetAddr.h"
#include "Network/Socket.h"
#include "Network/TcpClient.h"
#include "Network/TcpServer.h"
#include "Network/Udp.h"
#include "Network/Guid.h"

namespace Xunmei{
namespace Network{

class NETWORK_API NetworkSystem
{
public:
	/// 打印 Base库 版本信息
	void  printLibVersion();

	//初始化网络库
	//当maxcpuCorePerTherad ！= 0 NETWORK线程数使用 maxcpuCorePerTherad*CPUCore 
	//当maxcpuCorePerTherad == 0  ，threadNum ！= 0 Network使用线程数 threadNum
	//当maxcpuCorePerTherad == 0  ，threadNum == 0 时，network使用线程数默认为1
	static void init();

	//反初始化网络库
	static void uninit();
};

}
}



#endif //__XUNMEINETWORK_H__
