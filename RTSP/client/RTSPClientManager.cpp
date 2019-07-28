#include "RTSP/RTSPClient.h"
#include "../common/rtspProtocol.h"
#include "../common/sdpParse.h"
#include "../common/rtspTransport.h"
#include "../common/rtspRange.h"
#include "../common/rtspCMD.h"

namespace Public {
namespace RTSP {

struct RTSPClientManager::RTSPClientManagerInternal
{
	shared_ptr<IOWorker> worker;
	std::string			useragent;

	uint32_t					udpstartport;
	uint32_t					udpstopport;
	uint32_t					nowudpport;


	RTSPClientManagerInternal():udpstartport(40000),udpstopport(4000),nowudpport(udpstartport){}

	uint32_t allockRTPPort()
	{
		uint32_t udpport = nowudpport;
		uint32_t allocktimes = 0;

		while (allocktimes < udpstopport - udpstartport)
		{
			if (Host::checkPortIsNotUsed(udpport, Host::SocketType_UDP)
				&& Host::checkPortIsNotUsed(udpport + 1, Host::SocketType_UDP)
				&& Host::checkPortIsNotUsed(udpport + 2, Host::SocketType_UDP)
				&& Host::checkPortIsNotUsed(udpport + 3, Host::SocketType_UDP))
			{
				nowudpport = udpport + 1;

				return nowudpport;
			}

			udpport++;
			allocktimes++;
			if (udpport > udpstopport - 4) udpport = udpstartport;
		}
		
		return udpstartport;
	}
};

RTSPClientManager::RTSPClientManager(const shared_ptr<IOWorker>& iowrker, const std::string& useragent)
{
	internal = new RTSPClientManagerInternal;
	internal->worker = iowrker;
	internal->useragent = useragent;

	if (internal->worker == NULL) internal->worker = IOWorker::defaultWorker();
}
RTSPClientManager::~RTSPClientManager()
{
	SAFE_DELETE(internal);
}


shared_ptr<RTSPClient> RTSPClientManager::create(const shared_ptr<RTSPClientHandler>& handler, const std::string& pRtspUrl)
{
	if (!CheckUrl(pRtspUrl) || handler == NULL) return make_shared<RTSPClient>();

	//const std::shared_ptr<IOWorker>& work, const shared_ptr<RTSPClientHandler>& handler, const std::string& rtspUrl,const std::string& useragent);
	shared_ptr<RTSPClient> client = make_shared<RTSPClient>(internal->worker,handler,pRtspUrl,internal->useragent);

	return client;
}

bool RTSPClientManager::CheckUrl(const std::string& pRtspUrltmp)
{
	RTSPUrlInfo info;

	return info.parse(pRtspUrltmp);
}

bool RTSPClientManager::GetUserInfo(const std::string& pRtspUrl, std::string& pUserName, std::string& pPassWord)
{
	RTSPUrlInfo info;
	if (!info.parse(pRtspUrl))
	{
		return false;
	}
	pUserName = info.m_szUserName;
	pPassWord = info.m_szPassWord;

	return true;
}
}
}
