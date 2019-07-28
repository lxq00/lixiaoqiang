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
