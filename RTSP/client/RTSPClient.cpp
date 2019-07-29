#include "RTSP/RTSPClient.h"
#include "../common/rtspProtocol.h"
#include "RTSPClientInternal.h"

namespace Public {
namespace RTSP {

RTSPClient::RTSPClient(const std::shared_ptr<IOWorker>& work, const shared_ptr<RTSPClientHandler>& handler, const AllockUdpPortCallback& allockport, const RTSPUrl& rtspUrl, const std::string& useragent)
{
	internal = new RTSPClientInternal(handler,allockport, work, rtspUrl, useragent);
}
RTSPClient::~RTSPClient()
{
	stop();
	SAFE_DELETE(internal);
}
bool RTSPClient::initRTPOverTcpType()
{
	internal->rtspmedia.videoTransport.transport = TRANSPORT_INFO::TRANSPORT_RTP_TCP;
	internal->rtspmedia.videoTransport.rtp.t.videointerleaved = 0;
	internal->rtspmedia.videoTransport.rtp.t.audiointerleaved = 1;
	internal->rtspmedia.audioTransport = internal->rtspmedia.videoTransport;


	return true;
}
bool RTSPClient::initRTPOverUdpType()
{
	internal->rtspmedia.videoTransport.transport = TRANSPORT_INFO::TRANSPORT_RTP_UDP;
	internal->rtspmedia.audioTransport.transport = TRANSPORT_INFO::TRANSPORT_RTP_UDP;


	return true;
}
bool RTSPClient::start(uint32_t timeout)
{
	return internal->start(timeout);
}

bool RTSPClient::stop()
{
	return internal->stop();
}

shared_ptr<RTSPCommandInfo> RTSPClient::sendPlayRequest(const RANGE_INFO& range)
{
	return internal->sendPlayRequest(range)->cmd;
}

bool RTSPClient::sendPlayRequest(const RANGE_INFO& range, uint32_t timeout)
{
	shared_ptr<RTSPClientInternal::CommandInfo> cmdinfo = internal->sendPlayRequest(range, timeout);
	if (!cmdinfo->waitsem->pend(timeout) || cmdinfo->responseheader == NULL)
	{
		return false;
	}

	return cmdinfo->responseheader->statuscode == 200;
}

shared_ptr<RTSPCommandInfo> RTSPClient::sendPauseRequest()
{
	return internal->sendPauseRequest()->cmd;
}
bool RTSPClient::sendPauseRequest(uint32_t timeout)
{
	shared_ptr<RTSPClientInternal::CommandInfo> cmdinfo = internal->sendPauseRequest(timeout);
	if (!cmdinfo->waitsem->pend(timeout) || cmdinfo->responseheader == NULL)
	{
		return false;
	}

	return cmdinfo->responseheader->statuscode == 200;
}

shared_ptr<RTSPCommandInfo> RTSPClient::sendGetparameterRequest(const std::string& body)
{
	return internal->sendGetparameterRequest(body)->cmd;
}
bool RTSPClient::sendGetparameterRequest(const std::string& body, uint32_t timeout)
{
	shared_ptr<RTSPClientInternal::CommandInfo> cmdinfo = internal->sendGetparameterRequest(body, timeout);
	if (!cmdinfo->waitsem->pend(timeout) || cmdinfo->responseheader == NULL)
	{
		return false;
	}

	return cmdinfo->responseheader->statuscode == 200;
}
shared_ptr<RTSPCommandInfo> RTSPClient::sendTeradownRequest()
{
	return internal->sendTeradownRequest()->cmd;
}
bool RTSPClient::sendTeradownRequest(uint32_t timeout)
{
	shared_ptr<RTSPClientInternal::CommandInfo> cmdinfo = internal->sendTeradownRequest(timeout);
	if (!cmdinfo->waitsem->pend(timeout) || cmdinfo->responseheader == NULL)
	{
		return false;
	}

	return cmdinfo->responseheader->statuscode == 200;
}

}
}