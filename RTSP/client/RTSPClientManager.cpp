#include "RTSP/RTSPClient.h"
#include "../common/wwwAuthenticate.h"

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


shared_ptr<RTSPClient> RTSPClientManager::create(const shared_ptr<RTSPClientHandler>& handler, const RTSPUrl& pRtspUrl)
{
	if (handler == NULL) return shared_ptr<RTSPClient>();

	//const std::shared_ptr<IOWorker>& work, const shared_ptr<RTSPClientHandler>& handler, const std::string& rtspUrl,const std::string& useragent);
	shared_ptr<RTSPClient> client = shared_ptr<RTSPClient>(new RTSPClient(internal->worker,handler,
		RTSPClient::AllockUdpPortCallback(&RTSPClientManagerInternal::allockRTPPort, internal),pRtspUrl,internal->useragent));

	return client;
}
bool RTSPClientManager::initRTPOverUdpType(uint32_t startport, uint32_t stopport)
{
	internal->udpstartport = internal->nowudpport = min(startport,stopport);
	internal->udpstopport = max(startport, stopport);

	return true;
}

}
}
