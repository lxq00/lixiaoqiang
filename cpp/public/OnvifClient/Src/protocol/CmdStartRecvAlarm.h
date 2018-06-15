#ifndef __ONVIFPROTOCOL_PROFILES_H__StartRecvAlarm
#define __ONVIFPROTOCOL_PROFILES_H__StartRecvAlarm
#include "CmdObject.h"


class CMDStartRecvAlarm :public CmdObject
{
public:
	CMDStartRecvAlarm() {}
	virtual ~CMDStartRecvAlarm() {}

	virtual std::string build(const URI& uri)
	{
		stringstream stream;

		stream << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>"
			<< "<s:Envelope xmlns:s=\"http://www.w3.org/2003/05/soap-envelope\" xmlns:a=\"http://www.w3.org/2005/08/addressing\">"
			<< buildCreateHeader(uri)
			<< "<s:Body xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\" xmlns:xsd=\"http://www.w3.org/2001/XMLSchema\">"
			<< "<CreatePullPointSubscription xmlns=\"http://www.onvif.org/ver10/events/wsdl\">"
			<< "<InitialTerminationTime>PT60S</InitialTerminationTime>"
			<< "</CreatePullPointSubscription>"
			<< "</s:Body></s:Envelope>";

		return stream.str();
	}

	OnvifClientDefs::XAddr xaddr;
	virtual bool parse(XMLN * p_xml)
	{
		XMLN * p_res = xml_node_soap_get(p_xml, "tev:CreatePullPointSubscriptionResponse");
		if (NULL == p_res)
		{
			return false;
		}

		XMLN * p_media_uri = xml_node_soap_get(p_res, "tev:SubscriptionReference");
		if (p_media_uri)
		{
			XMLN * p_uri = xml_node_soap_get(p_media_uri, "wsa5:Address");
			if (p_uri && p_uri->data)
			{
				onvif_parse_xaddr(p_uri->data, xaddr);
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
};



#endif //__ONVIFPROTOCOL_H__
