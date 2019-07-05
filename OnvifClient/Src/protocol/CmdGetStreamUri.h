#ifndef __ONVIFPROTOCOL_CmdGetStreamURL_H__
#define __ONVIFPROTOCOL_CmdGetStreamURL_H__
#include "CmdObject.h"


class CmdGetStreamURL :public CmdObject
{
public:
	CmdGetStreamURL(const std::string& _token) :token(_token)
	{
		action = "http://www.onvif.org/ver10/media/wsdl/GetStreamURL";
	}
	virtual ~CmdGetStreamURL() {}

	virtual std::string build(const URL& URL)
	{
		stringstream stream;

		stream << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>"
			<< "<s:Envelope " << onvif_xml_ns << ">"
			<< buildHeader(URL)
			<< "<s:Body>"
			<< "<trt:GetStreamUri>"
			<< "<trt:StreamSetup>"
			<< "<tt:Stream>RTP-Unicast</tt:Stream>"
			<< "<tt:Transport>"
			<< "<tt:Protocol>RTSP</tt:Protocol>"
			<< "</tt:Transport>"
			<< "</trt:StreamSetup>"
			<< "<trt:ProfileToken>" << token << "</trt:ProfileToken>"
			<< "</trt:GetStreamUri>"
			<<"</s:Body></s:Envelope>";

		return stream.str();
	}

	shared_ptr<OnvifClientDefs::StreamUrl> streamurl;
	virtual bool parse(const XMLObject::Child& body)
	{
		streamurl = make_shared<OnvifClientDefs::StreamUrl>();

		const XMLObject::Child& resp = body.getChild("trt:GetStreamUriResponse");
		if (!resp) return false;

		streamurl->url = resp.getChild("trt:MediaUri").getChild("tt:Uri").data();

		if (streamurl->url == "") return false;

		return true;
	}

private:
	std::string token;
};



#endif //__ONVIFPROTOCOL_H__
