#ifndef __RTSPClientManager_H__
#define __RTSPClientManager_H__
#include "RTSPClient/RTSPClient.h"

using namespace Public::RTSPClient;

struct RTSPClientManager::RTSPClientManagerInternal
{
	shared_ptr<AsyncIOWorker> ioworker;
	std::string			 userContext;

	uint32_t				m_udpStartPort;
	uint32_t				m_udpStopPort;
};

#endif //__RTSPClientManager_H__
