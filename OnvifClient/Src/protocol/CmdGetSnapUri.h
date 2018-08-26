#ifndef __ONVIFPROTOCOL_H__GetSnapURL
#define __ONVIFPROTOCOL_H__GetSnapURL
#include "CmdObject.h"


class CmdGetSnapURL :public CmdObject
{
public:
	CmdGetSnapURL(const std::string& _token) :token(_token)
	{
		action = "http://www.onvif.org/ver10/media/wsdl/GetSnapshotURL";
	}
	virtual ~CmdGetSnapURL() {}

	virtual std::string build(const URL& URL)
	{
		stringstream stream;

		stream << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>"
			<< "<s:Envelope " << onvif_xml_ns << ">"
			<< buildHeader(URL)
			<< "<s:Body xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\" xmlns:xsd=\"http://www.w3.org/2001/XMLSchema\">"
			<< "<GetSnapshotURL xmlns=\"http://www.onvif.org/ver10/media/wsdl\">"
			<< "<trt:ProfileToken>" << token << "</trt:ProfileToken>"
			<< "</GetSnapshotURL>"
			<< "</s:Body></s:Envelope>";

		return stream.str();
	}
	std::string snapurl;
	virtual bool parse(XMLN * p_xml)
	{
		XMLN * p_res = xml_node_soap_get(p_xml, "trt:GetSnapshotURLResponse");
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
				snapurl = p_URL->data;
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
