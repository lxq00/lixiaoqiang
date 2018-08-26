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

	bool sendOvifRequest(CmdObject* cmd,int timeout)
	{
		shared_ptr<HTTPClient> client = make_shared<HTTPClient>(worker, useragent);
		shared_ptr<HTTPRequest> req = make_shared<HTTPRequest>();

		{
			{
				stringstream stream;
				stream << ONVIFECONENTTYPE << "; charset=utf-8; action=\""
					<< cmd->action
					<< "\"";

				req->headers()["Content-Type"] = stream.str();
			}
			req->headers()["Accept-Encoding"] = "gzip, deflate";
			if(useragent != "")
				req->headers()["User-Agent"] = useragent;
			req->headers()["Connection"] = "close";
		}

		req->content()->write(cmd->build(url));
		req->method() = "POST";
		req->timeout() = timeout;
		req->url() = url;

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
		XMLN * p_node = xxx_hxml_parse((char*)httpbody.c_str(), httpbody.length());
		
		if (p_node == NULL) return false;
		if (p_node->name == NULL || soap_strcmp(p_node->name, "s:Envelope") != 0)
		{
			xml_node_del(p_node);
			return false;
		}
		XMLN * p_body = xml_node_soap_get(p_node, "s:Body");
		if (p_body == NULL)
		{
			xml_node_del(p_node);
			return false;
		}
		cmd->recvbuffer = httpbody.c_str();
		bool parseret = cmd->parse(p_body);
		xml_node_del(p_node);

		return parseret;
	}
};

OnvifClient::OnvifClient(const URL& url, const shared_ptr<IOWorker>& worker, const std::string& useragent)
{
	internal = new OnvifClientInternal;
	internal->url = url;
	internal->useragent = useragent;
	internal->worker = worker;

	if (internal->url.getPath() == "")
	{
		internal->url.setPath(DEFAULTONVIFRDEVICEURL);
	}
}
OnvifClient::~OnvifClient()
{
	SAFE_DELETE(internal);
}

shared_ptr<OnvifClientDefs::Info> OnvifClient::getInfo(int timeoutms)
{
	shared_ptr<CMDGetDeviceInformation> cmd = make_shared<CMDGetDeviceInformation>();

	internal->sendOvifRequest(cmd.get(), timeoutms);

	return cmd->devinfo;
}


shared_ptr<OnvifClientDefs::Capabilities> OnvifClient::getCapabities(int timeoutms)
{
	shared_ptr<CMDGetCapabilities> cmd = make_shared<CMDGetCapabilities>();

	internal->sendOvifRequest(cmd.get(), timeoutms);

	return cmd->capabilities;
}

shared_ptr<OnvifClientDefs::Scopes> OnvifClient::getScopes(int timeoutms)
{
	shared_ptr<CMDGetScopes> cmd = make_shared<CMDGetScopes>();

	internal->sendOvifRequest(cmd.get(), timeoutms);

	return cmd->scopes;
}
shared_ptr<OnvifClientDefs::Profiles> OnvifClient::getProfiles(int timeoutms)
{
	shared_ptr<CMDGetProfiles> cmd = make_shared<CMDGetProfiles>();

	internal->sendOvifRequest(cmd.get(), timeoutms);

	return cmd->profileInfo;
}
std::string OnvifClient::getStreamUrl(const std::string& streamtoken, int timeoutms)
{
	shared_ptr<CmdGetStreamURL> cmd = make_shared<CmdGetStreamURL>(streamtoken);

	internal->sendOvifRequest(cmd.get(), timeoutms);

	return cmd->streamurl;
}
std::string OnvifClient::getSnapUrl(const std::string& snaptoken, int timeoutms)
{
	shared_ptr<CmdGetSnapURL> cmd = make_shared<CmdGetSnapURL>(snaptoken);

	internal->sendOvifRequest(cmd.get(), timeoutms);

	return cmd->snapurl;
}
shared_ptr<OnvifClientDefs::NetworkInterfaces> OnvifClient::getNetworkInterfaces(int timeoutms)
{
	shared_ptr<CmdGetNetworkInterfaces> cmd = make_shared<CmdGetNetworkInterfaces>();

	internal->sendOvifRequest(cmd.get(), timeoutms);

	return cmd->network;
}
shared_ptr<OnvifClientDefs::VideoEncoderConfigurations> OnvifClient::getVideoEncoderConfigurations(int timeoutms)
{
	shared_ptr<CmdGetVideoEncoderConfigurations> cmd = make_shared<CmdGetVideoEncoderConfigurations>();

	internal->sendOvifRequest(cmd.get(), timeoutms);

	return cmd->encoder;
}
shared_ptr<OnvifClientDefs::ContinuousMove> OnvifClient::getContinuousMove(int timeoutms)
{
	shared_ptr<CmdContinuousMove> cmd;// = make_shared<CmdContinuousMove>();

	internal->sendOvifRequest(cmd.get(), timeoutms);

	return cmd->move;
}
shared_ptr<OnvifClientDefs::AbsoluteMove> OnvifClient::getAbsoluteMove(int timeoutms)
{
	shared_ptr<CmdAbsoluteMove> cmd;// = make_shared<CmdAbsoluteMove>();

	internal->sendOvifRequest(cmd.get(), timeoutms);

	return cmd->move;
}
shared_ptr<OnvifClientDefs::_PTZConfig> OnvifClient::getConfigurations(int timeoutms)
{
	shared_ptr<CmdGetConfigurations> cmd = make_shared<CmdGetConfigurations>();

	internal->sendOvifRequest(cmd.get(), timeoutms);

	return cmd->ptzcfg;
}
shared_ptr<OnvifClientDefs::ConfigurationOptions> OnvifClient::getConfigurationOptions(int timeoutms)
{
	shared_ptr<CmdGetConfigurationOptions> cmd;// = make_shared<CmdGetConfigurationOptions>();

	internal->sendOvifRequest(cmd.get(), timeoutms);

	return cmd->options;
}
shared_ptr<Time> OnvifClient::GetSystemDatetime(int timeoutms)
{
	shared_ptr<CmdGetSystemDateAndTime> cmd = make_shared<CmdGetSystemDateAndTime>();

	internal->sendOvifRequest(cmd.get(), timeoutms);

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
bool OnvifClient::startRecvAlarm()
{
	return false;
}
bool OnvifClient::stopRecvAlarm()
{
	return false;
}

struct OnvifClientManager::OnvifClientManagerInternal
{
	std::string useragent;
	shared_ptr<IOWorker> worker;
};


OnvifClientManager::OnvifClientManager(const std::string& userContent, const IOWorker::ThreadNum& threadNum)
	:OnvifClientManager(userContent, make_shared<IOWorker>(threadNum))
{
}

OnvifClientManager::OnvifClientManager(const std::string& userContent, const shared_ptr<IOWorker>& worker)
{
	internal->useragent = userContent;
	internal->worker = worker;
	if (internal->worker == NULL)
	{
		internal->worker = make_shared<IOWorker>(2);
	}
}
OnvifClientManager::~OnvifClientManager()
{
	SAFE_DELETE(internal);
}

shared_ptr<OnvifClient> OnvifClientManager::create(const URL& url)
{
	return shared_ptr<OnvifClient>(new OnvifClient(url, internal->worker, internal->useragent));
}

}
}

