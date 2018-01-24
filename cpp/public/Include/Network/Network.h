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
	/// ��ӡ Base�� �汾��Ϣ
	void  printLibVersion();

	//��ʼ�������
	//��maxcpuCorePerTherad ��= 0 NETWORK�߳���ʹ�� maxcpuCorePerTherad*CPUCore 
	//��maxcpuCorePerTherad == 0  ��threadNum ��= 0 Networkʹ���߳��� threadNum
	//��maxcpuCorePerTherad == 0  ��threadNum == 0 ʱ��networkʹ���߳���Ĭ��Ϊ1
	static void init();

	//����ʼ�������
	static void uninit();
};

}
}



#endif //__XUNMEINETWORK_H__
