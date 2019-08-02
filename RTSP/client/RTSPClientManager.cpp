#include "RTSP/RTSPClient.h"
#include "../common/wwwAuthenticate.h"
#include "../common/udpPortAlloc.h"

namespace Public {
namespace RTSP {

struct RTSPClientManager::RTSPClientManagerInternal :public UDPPortAlloc
{
	shared_ptr<IOWorker> worker;
	std::string			useragent;

	RTSPClientManagerInternal(){}
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

	AllockUdpPortCallback alloccallback(&UDPPortAlloc::allockRTPPort, (UDPPortAlloc*)internal);

	//const std::shared_ptr<IOWorker>& work, const shared_ptr<RTSPClientHandler>& handler, const std::string& rtspUrl,const std::string& useragent);
	shared_ptr<RTSPClient> client = shared_ptr<RTSPClient>(new RTSPClient(internal->worker,handler, alloccallback ,pRtspUrl,internal->useragent));

	return client;
}
bool RTSPClientManager::initRTPOverUdpType(uint32_t startport, uint32_t stopport)
{
	internal->setUdpPortInfo(startport, stopport);

	return true;
}

}
}
