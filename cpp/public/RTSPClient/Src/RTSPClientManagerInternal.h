#ifndef __RTSPClientManager_H__
#define __RTSPClientManager_H__
#include "RTSPClient/RTSPClient.h"

using namespace Xunmei::RTSPClient;

struct RTSPClientManager::RTSPClientManagerInternal
{
	shared_ptr<IOWorker> ioworker;
	std::string			 userContext;

	uint32_t				m_udpStartPort;
	uint32_t				m_udpStopPort;
};

#endif //__RTSPClientManager_H__
