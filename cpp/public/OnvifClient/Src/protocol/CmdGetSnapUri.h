#ifndef __ONVIFPROTOCOL_H__GetSnapUri
#define __ONVIFPROTOCOL_H__GetSnapUri
#include "CmdObject.h"


class CmdGetSnapUri :public CmdObject
{
public:
	CmdGetSnapUri(const std::string& _token):token(_token) {}
	virtual ~CmdGetSnapUri() {}

	virtual std::string build(const URI& uri)
	{
		stringstream stream;

		stream << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>"
			<< "<s:Envelope " << onvif_xml_ns << ">"
			<< buildHeader(uri)
			<< "<s:Body xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\" xmlns:xsd=\"http://www.w3.org/2001/XMLSchema\">"
			<< "<GetSnapshotUri xmlns=\"http://www.onvif.org/ver10/media/wsdl\">"
			<< "<trt:ProfileToken>" << token << "</trt:ProfileToken>"
			<< "</GetSnapshotUri>"
			<< "</s:Body></s:Envelope>";

		return stream.str();
	}
	std::string snapurl;
	virtual bool parse(XMLN * p_xml)
	{
		XMLN * p_res = xml_node_soap_get(p_xml, "trt:GetSnapshotUriResponse");
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
				snapurl = p_uri->data;
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
