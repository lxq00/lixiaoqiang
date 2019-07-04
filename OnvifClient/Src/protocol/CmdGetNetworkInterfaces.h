#ifndef __ONVIFPROTOCOL_CmdGetStreamURL_H__GetNetworkInterfaces
#define __ONVIFPROTOCOL_CmdGetStreamURL_H__GetNetworkInterfaces
#include "CmdObject.h"


class CmdGetNetworkInterfaces :public CmdObject
{
public:
	CmdGetNetworkInterfaces()
	{
		action = "http://www.onvif.org/ver10/device/wsdl/GetNetworkInterfaces";
	}
	virtual ~CmdGetNetworkInterfaces() {}

	virtual std::string build(const URL& URL)
	{
		stringstream stream;

		stream << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>"
			<< "<s:Envelope " << onvif_xml_ns << ">"
			<< buildHeader(URL)
			<< "<s:Body>"
			<< "<tds:GetNetworkInterfaces></tds:GetNetworkInterfaces>"
			<<  "</s:Body></s:Envelope>";

		return stream.str();
	}

	shared_ptr<OnvifClientDefs::NetworkInterfaces> network;
	virtual bool parse(XMLN * p_xml) 
	{
		network = make_shared<OnvifClientDefs::NetworkInterfaces>();

		XMLN * p_res = xml_node_soap_get(p_xml, "tds:GetNetworkInterfacesResponse");
		if (NULL == p_res)
		{
			return false;
		}

		XMLN * p_network = xml_node_soap_get(p_res, "tds:NetworkInterfaces");
		if (NULL == p_network)
		{
			return false;
		}

		XMLN * p_info = xml_node_soap_get(p_network, "tt:Info");
		if (p_info)
		{
			XMLN * p_name = xml_node_soap_get(p_info, "tt:Name");
			if (p_name && p_name->data)
			{
				network->name = p_name->data;
			}
			XMLN * p_mac = xml_node_soap_get(p_info, "tt:HwAddress");
			if (p_mac && p_mac->data)
			{
				network->macaddr = p_mac->data;
			}
		}

		XMLN * p_ipv4 = xml_node_soap_get(p_network, "tt:IPv4");
		if (p_ipv4)
		{
			XMLN * p_config = xml_node_soap_get(p_ipv4, "tt:Config");
			if (p_config)
			{
				XMLN * p_manual = xml_node_soap_get(p_config, "tt:Manual");
				if (p_manual)
				{
					if (p_manual->data) network->dhcp = !atoi(p_manual->data);
					else
					{
						XMLN * p_addr = xml_node_soap_get(p_manual, "tt:Address");
						if (p_addr && p_addr->data)
						{
							network->ipaddr = p_addr->data;
						}
					}
				}
				XMLN * p_dhcp = xml_node_soap_get(p_config, "tt:DHCP");
				if (p_dhcp)
				{
					if (p_dhcp->data) network->dhcp = atoi(p_dhcp->data);
					else
					{
						XMLN * p_addr = xml_node_soap_get(p_dhcp, "tt:Address");
						if (p_addr && p_addr->data)
						{
							network->ipaddr = p_addr->data;
						}
					}
				}
			}
		}

		return true;
	}
};



#endif //__ONVIFPROTOCOL_H__
