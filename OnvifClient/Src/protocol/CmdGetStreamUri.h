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
			<< "<trt:GetStreamURL>"
			<< "<trt:StreamSetup>"
			<< "<tt:Stream>RTP-Unicast</tt:Stream>"
			<< "<tt:Transport>"
			<< "<tt:Protocol>RTSP</tt:Protocol>"
			<< "</tt:Transport>"
			<< "</trt:StreamSetup>"
			<< "<trt:ProfileToken>" << token << "</trt:ProfileToken>"
			<< "</trt:GetStreamURL>"
			<<"</s:Body></s:Envelope>";

		return stream.str();
	}

	std::string streamurl;
	virtual bool parse(XMLN * p_xml)
	{
		XMLN * p_res = xml_node_soap_get(p_xml, "trt:GetStreamURLResponse");
		if (NULL == p_res)
		{
			return false;
		}

		XMLN * p_media_URL = xml_node_soap_get(p_res, "trt:MediaURL");
		if (p_media_URL)
		{
			XMLN * p_URL = xml_node_soap_get(p_media_URL, "trt:URL");
			if (p_URL && p_URL->data)
			{
				streamurl = p_URL->data;
			}
			else
			{
				return false;
			}
		}
		else
		{
			return false;
		}

		return true;
	}

private:
	std::string token;
};



#endif //__ONVIFPROTOCOL_H__
