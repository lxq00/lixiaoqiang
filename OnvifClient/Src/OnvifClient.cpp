#include "OnvifClient/OnvifClient.h"
#include "HTTP/HTTPClient.h"
#include "protocol/OnvifProtocol.h"

using namespace Public::HTTP;

namespace Public {
namespace Onvif {

#define DEFAULTONVIFRDEVICEURL	"/onvif/device_service"

#define ONVIFECONENTTYPE		"application/soap+xml"

struct OnvifClient::OnvifClientInternal
{
	URL url;
	shared_ptr<IOWorker> worker;
	std::string useragent;
public:
	bool sendOvifRequest(CmdObject* cmd, int timeout, const URL& _requrl = URL())
	{
		shared_ptr<HTTPClient> client = make_shared<HTTPClient>(worker, useragent);
		shared_ptr<HTTPRequest> req = make_shared<HTTPRequest>();

		URL requrl = _requrl;
		if (requrl.getHostname() == "") requrl = url;
		if (requrl.getPath() == "" || requrl.getPath() == "/")
		{
			requrl.setPath(DEFAULTONVIFRDEVICEURL);
		}

		requrl.setAuthen(url.getAuhen());

		{
			req->headers()["Content-Type"] = std::string(ONVIFECONENTTYPE) + "; charset=utf-8; action=\"" + cmd->action + "\"";
			req->headers()["Accept-Encoding"] = "gzip, deflate";
			if (useragent != "")
				req->headers()["User-Agent"] = useragent;
			req->headers()["Connection"] = "close";
		}

		req->content()->write(cmd->build(requrl));
		req->method() = "POST";
		req->timeout() = timeout;
		req->url() = requrl.getHost() + requrl.getPath() + requrl.getSearch();

		shared_ptr<HTTPResponse> response = client->request(req);

		if (response == NULL || response->statusCode() != 200)
		{
			return false;
		}

		Value contenttypeval = response->header("Content-Type");
		if (strstr(String::tolower(contenttypeval.readString()).c_str(), ONVIFECONENTTYPE) == 0)
		{
			return false;
		}

		std::string httpbody = response->content()->read();

		XMLObject xml;
		if (!xml.parseBuffer(httpbody)) return false;

		if (xml.getRoot().name() != "s:Envelope") return false;

		const XMLObject::Child& body = xml.getRoot().getChild("s:Body");
		if (body.isEmpty()) return false;

		cmd->recvbuffer = httpbody.c_str();
		bool parseret = cmd->parse(body);

		return parseret;
	}
};

OnvifClient::OnvifClient(const shared_ptr<IOWorker>& worker, const URL& url,const std::string& useragent)
{
	internal = new OnvifClientInternal;
	internal->url = url;
	internal->useragent = useragent;
	internal->worker = worker;
}
OnvifClient::~OnvifClient()
{
	SAFE_DELETE(internal);
}

shared_ptr<OnvifClientDefs::Info> OnvifClient::getInfo(int timeoutms)
{
	shared_ptr<CMDGetDeviceInformation> cmd = make_shared<CMDGetDeviceInformation>();

	if (!internal->sendOvifRequest(cmd.get(), timeoutms)) return shared_ptr<OnvifClientDefs::Info>();

	return cmd->devinfo;
}


shared_ptr<OnvifClientDefs::Capabilities> OnvifClient::getCapabities(int timeoutms)
{
	shared_ptr<CMDGetCapabilities> cmd = make_shared<CMDGetCapabilities>();

	if (!internal->sendOvifRequest(cmd.get(), timeoutms)) return shared_ptr<OnvifClientDefs::Capabilities>();

	return cmd->capabilities;
}

//shared_ptr<OnvifClientDefs::Scopes> OnvifClient::getScopes(int timeoutms)
//{
//	shared_ptr<CMDGetScopes> cmd = make_shared<CMDGetScopes>();
//
//	internal->sendOvifRequest(cmd.get(), timeoutms);
//
//	return cmd->scopes;
//}
shared_ptr<OnvifClientDefs::Profiles> OnvifClient::getProfiles(int timeoutms)
{
	shared_ptr<CMDGetProfiles> cmd = make_shared<CMDGetProfiles>();

	if (!internal->sendOvifRequest(cmd.get(), timeoutms)) return shared_ptr<OnvifClientDefs::Profiles>();

	return cmd->profileInfo;
}
shared_ptr<OnvifClientDefs::StreamUrl> OnvifClient::getStreamUrl(const OnvifClientDefs::ProfileInfo& info, int timeoutms)
{
	shared_ptr<CmdGetStreamURL> cmd = make_shared<CmdGetStreamURL>(info.token);

	if (!internal->sendOvifRequest(cmd.get(), timeoutms)) return shared_ptr<OnvifClientDefs::StreamUrl>();

	return cmd->streamurl;
}
shared_ptr<OnvifClientDefs::SnapUrl> OnvifClient::getSnapUrl(const OnvifClientDefs::ProfileInfo& info, int timeoutms)
{
	shared_ptr<CmdGetSnapURL> cmd = make_shared<CmdGetSnapURL>(info.token);

	if (!internal->sendOvifRequest(cmd.get(), timeoutms)) return shared_ptr<OnvifClientDefs::SnapUrl>();

	return cmd->snapurl;
}
shared_ptr<OnvifClientDefs::NetworkInterfaces> OnvifClient::getNetworkInterfaces(int timeoutms)
{
	shared_ptr<CmdGetNetworkInterfaces> cmd = make_shared<CmdGetNetworkInterfaces>();

	if (!internal->sendOvifRequest(cmd.get(), timeoutms)) return shared_ptr<OnvifClientDefs::NetworkInterfaces>();

	return cmd->network;
}
//shared_ptr<OnvifClientDefs::VideoEncoderConfigurations> OnvifClient::getVideoEncoderConfigurations(int timeoutms)
//{
//	shared_ptr<CmdGetVideoEncoderConfigurations> cmd = make_shared<CmdGetVideoEncoderConfigurations>();
//
//	internal->sendOvifRequest(cmd.get(), timeoutms);
//
//	return cmd->encoder;
//}
shared_ptr<OnvifClientDefs::ContinuousMove> OnvifClient::getContinuousMove(const OnvifClientDefs::ProfileInfo& info, int timeoutms)
{
	shared_ptr<CmdContinuousMove> cmd;// =  = make_shared<CmdContinuousMove>();

	if (!internal->sendOvifRequest(cmd.get(), timeoutms)) return shared_ptr<OnvifClientDefs::ContinuousMove>();

	return cmd->move;
}
shared_ptr<OnvifClientDefs::AbsoluteMove> OnvifClient::getAbsoluteMove(const OnvifClientDefs::ProfileInfo& info, int timeoutms)
{
	shared_ptr<CmdAbsoluteMove> cmd;// = make_shared<CmdAbsoluteMove>();

	if (!internal->sendOvifRequest(cmd.get(), timeoutms)) return shared_ptr<OnvifClientDefs::AbsoluteMove>();

	return cmd->move;
}
shared_ptr<OnvifClientDefs::PTZConfig> OnvifClient::getConfigurations(int timeoutms)
{
	shared_ptr<CmdGetConfigurations> cmd = make_shared<CmdGetConfigurations>();

	if (!internal->sendOvifRequest(cmd.get(), timeoutms)) return shared_ptr<OnvifClientDefs::PTZConfig>();

	return cmd->ptzcfg;
}
shared_ptr<OnvifClientDefs::ConfigurationOptions> OnvifClient::getConfigurationOptions(const shared_ptr<OnvifClientDefs::PTZConfig>& ptzcfg, int timeoutms)
{
	if (ptzcfg == NULL) return make_shared<OnvifClientDefs::ConfigurationOptions>();

	shared_ptr<CmdGetConfigurationOptions> cmd = make_shared<CmdGetConfigurationOptions>(ptzcfg->token);

	if (!internal->sendOvifRequest(cmd.get(), timeoutms)) return shared_ptr<OnvifClientDefs::ConfigurationOptions>();

	return cmd->options;
}
shared_ptr<Time> OnvifClient::GetSystemDatetime(int timeoutms)
{
	shared_ptr<CmdGetSystemDateAndTime> cmd = make_shared<CmdGetSystemDateAndTime>();

	if (!internal->sendOvifRequest(cmd.get(), timeoutms)) return shared_ptr<Time>();

	return cmd->time;
}
bool OnvifClient::SetSystemDatetime(const Time& time, int timeoutms)
{
	shared_ptr<CmdSetSystemDateAndTime> cmd = make_shared<CmdSetSystemDateAndTime>(time);

	return internal->sendOvifRequest(cmd.get(), timeoutms);
}
bool OnvifClient::SystemReboot(int timeoutms)
{
	shared_ptr<CMDSystemReboot> cmd = make_shared<CMDSystemReboot>();

	return internal->sendOvifRequest(cmd.get(), timeoutms);
}
shared_ptr<OnvifClientDefs::StartRecvAlarm>  OnvifClient::startRecvAlarm(const shared_ptr<OnvifClientDefs::Capabilities>& capabilities,int timeoutms)
{
	if (capabilities == NULL || !capabilities->Events.Support)
	{
		return shared_ptr<OnvifClientDefs::StartRecvAlarm>();
	}

	shared_ptr<CMDStartRecvAlarm> cmd = make_shared<CMDStartRecvAlarm>();

	if (!internal->sendOvifRequest(cmd.get(), timeoutms, capabilities->Events.xaddr))
	{
		return shared_ptr<OnvifClientDefs::StartRecvAlarm>();
	}

	return cmd->startrecvalarm;
}
bool OnvifClient::recvAlarm(const shared_ptr<OnvifClientDefs::StartRecvAlarm>& alarminfo, int timeoutms)
{
	if (alarminfo == NULL) return false;

	shared_ptr<CMDGetAlarm> cmd = make_shared<CMDGetAlarm>();

	return internal->sendOvifRequest(cmd.get(), timeoutms,alarminfo->xaddr);
}
bool OnvifClient::stopRecvAlarm()
{
	return false;
}

}
}

