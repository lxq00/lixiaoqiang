#include "RTSPClient/RTSPClient.h"
#include "RTSPClientInternal.h"
#include "RTSPClientManagerInternal.h"


using namespace Public::RTSPClient;

RTSPClientManager::RTSPClientManager(const std::string& userContent, const IOWorker::ThreadNum& threadNum)
	:RTSPClientManager(userContent, make_shared<IOWorker>(threadNum))
{
}

RTSPClientManager::RTSPClientManager(const std::string& userContent, const shared_ptr<IOWorker>& _worker)
{
	shared_ptr<IOWorker> worker = _worker;
	if (worker == NULL)
	{
		worker = make_shared<IOWorker>(IOWorker::ThreadNum(2));
	}

	internal = new RTSPClientManagerInternal();
	internal->ioworker = worker;
	internal->userContext = userContent;

	initUDPRecvTypeStartPort();
}

RTSPClientManager::~RTSPClientManager()
{
	SAFE_DELETE(internal);
}

bool RTSPClientManager::initUDPRecvTypeStartPort(uint32_t startport, uint32_t stopport)
{
	internal->m_udpStartPort = startport;
	internal->m_udpStopPort = stopport;

	return true;
}

shared_ptr<RTSPClient> RTSPClientManager::create(const std::string& pRtspUrl)
{
	RTSPUrlInfo info;
	if (!info.parse(pRtspUrl))
	{
		return shared_ptr<RTSPClient>();
	}

	shared_ptr<RTSPClient> client(new RTSPClient(this, pRtspUrl,internal->userContext));

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


RTSPClient::RTSPClient(RTSPClientManager* manager, const std::string& rtspUrl, const std::string& usercontext)
{
	internal = new RTSPClientInternal(rtspUrl);
	internal->m_manager = manager;
	internal->m_rtspInfo->m_userAgent = usercontext;
}

RTSPClient::~RTSPClient()
{
	SAFE_DELETE(internal);
}

bool RTSPClient::initCallback(const RTSPClient_DataCallBack& dataCallback, const RTSPClient_StatusCallback& statusCallback, const RTSPClient_MediaCallback& mediacallback, const RTSPClient_ProtocolCallback& protocolCallback, void* userData)
{
	internal->m_dataCallback = dataCallback;
	internal->m_statusCallback = statusCallback;
	internal->m_mediaCallback = mediacallback;
	internal->m_protocolCallback = protocolCallback;
	internal->m_userData = userData;

	return true;
}

bool RTSPClient::initCallback(const RTSPClient_RTPDataCallBack& dataCallback, const RTSPClient_StatusCallback& statusCallback, const RTSPClient_MediaCallback& mediacallback, const RTSPClient_ProtocolCallback& protocolCallback, void* userData)
{
	internal->m_rtpDataCallback = dataCallback;
	internal->m_statusCallback = statusCallback;
	internal->m_mediaCallback = mediacallback;
	internal->m_protocolCallback = protocolCallback;
	internal->m_userData = userData;

	return true;
}

bool RTSPClient::initRecvType(RTP_RECV_Type nRecvType)
{
	internal->m_nRecvType = nRecvType;;

	return true;
}

bool RTSPClient::start(uint32_t timeout)
{
	uint32_t udpstartport = 0;

	if (internal->m_nRecvType == RTP_RECV_TYPE_UDP)
	{
		udpstartport = internal->m_manager->internal->m_udpStartPort;
		while (udpstartport < internal->m_manager->internal->m_udpStopPort)
		{
			if (Host::checkPortIsNotUsed(udpstartport, Host::SocketType_UDP)
				&& Host::checkPortIsNotUsed(udpstartport + 1, Host::SocketType_UDP)
				&& Host::checkPortIsNotUsed(udpstartport + 2, Host::SocketType_UDP)
				&& Host::checkPortIsNotUsed(udpstartport + 3, Host::SocketType_UDP))
			{
				break;
			}

			udpstartport++;
		}
	}
	
	return internal->start(internal->m_manager->internal->ioworker, timeout, udpstartport);
}


bool RTSPClient::stop()
{
	return internal->stop();
}

bool RTSPClient::getMediaInfo(LPMEDIA_PARAMETER lpMdeiaParam)
{
	if (lpMdeiaParam == NULL)
	{
		return false;
	}
	internal->converRTSPCode(lpMdeiaParam);

	return true;
}
bool RTSPClient::initUserInfo(const std::string& username, const std::string& passwd)
{
	internal->m_rtspInfo->m_szUserName = username;
	internal->m_rtspInfo->m_szPassWord = passwd;

	return true;
}
bool RTSPClient::initPlayInfo(const std::string& type, const std::string& starttime, const std::string& stoptime)
{
	internal->play_type = type;
	internal->play_startTime = starttime;
	internal->play_stopTime = stoptime;

	return true;
}