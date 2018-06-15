#ifndef __ONVIFPROTOCOL_CmdGetStreamUri_H__
#define __ONVIFPROTOCOL_CmdGetStreamUri_H__
#include "CmdObject.h"


class CmdGetStreamUri :public CmdObject
{
public:
	CmdGetStreamUri(const std::string& _token):token(_token) {}
	virtual ~CmdGetStreamUri() {}

	virtual std::string build(const URI& uri)
	{
		stringstream stream;

		stream << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>"
			<< "<s:Envelope " << onvif_xml_ns << ">"
			<< buildHeader(uri)
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

	std::string streamurl;
	virtual bool parse(XMLN * p_xml)
	{
		XMLN * p_res = xml_node_soap_get(p_xml, "trt:GetStreamUriResponse");
		if (NULL == p_res)
		{
			return false;
		}

		XMLN * p_media_uri = xml_node_soap_get(p_res, "trt:MediaUri");
		if (p_media_uri)
		{
			XMLN * p_uri = xml_node_soap_get(p_media_uri, "trt:Uri");
			if (p_uri && p_uri->data)
			{
				streamurl = p_uri->data;
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
