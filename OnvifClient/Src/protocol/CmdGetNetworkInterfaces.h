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
	virtual bool parse(const XMLObject::Child& body)
	{
		network = make_shared<OnvifClientDefs::NetworkInterfaces>();

		const XMLObject::Child& resp = body.getChild("tds:GetNetworkInterfacesResponse");
		if (!resp) return false;

		const XMLObject::Child& networkinterface = resp.getChild("tds:NetworkInterfaces");
		if (!networkinterface) return false;

		network->name = networkinterface.getChild("tt:Info").getChild("tt:Name").getValue();
		network->macaddr = networkinterface.getChild("tt:Info").getChild("tt:HwAddress").getValue();

		const XMLObject::Child& manual = networkinterface.getChild("tt:IPv4").getChild("tt:Config").getChild("tt:Manual");
		if (manual)
		{
			if (manual.getValue()) network->dhcp = !manual.getValue().readBool();
			else
			{
				network->ipaddr = manual.getChild("tt:Address").getValue();
			}
		}

		const XMLObject::Child& dhcp = networkinterface.getChild("tt:IPv4").getChild("tt:Config").getChild("tt:DHCP");
		if (dhcp)
		{
			if (dhcp.getValue()) network->dhcp = dhcp.getValue().readBool();
			else
			{
				network->ipaddr = dhcp.getChild("tt:Address").getValue();
			}
		}

		return true;
	}
};



#endif //__ONVIFPROTOCOL_H__
