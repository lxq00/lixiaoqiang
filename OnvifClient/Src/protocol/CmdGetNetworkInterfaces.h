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

		network->name = networkinterface.getChild("tt:Info").getChild("tt:Name").data();
		network->macaddr = networkinterface.getChild("tt:Info").getChild("tt:HwAddress").data();

		const XMLObject::Child& manual = networkinterface.getChild("tt:IPv4").getChild("tt:Config").getChild("tt:Manual");
		if (manual)
		{
			if (!manual.data().empty()) network->dhcp = !manual.data().readBool();
			else
			{
				network->ipaddr = manual.getChild("tt:Address").data();
			}
		}

		const XMLObject::Child& dhcp = networkinterface.getChild("tt:IPv4").getChild("tt:Config").getChild("tt:DHCP");
		if (dhcp)
		{
			if (!dhcp.data().empty()) network->dhcp = dhcp.data().readBool();
			else
			{
				network->ipaddr = dhcp.getChild("tt:Address").data();
			}
		}

		return true;
	}
};



#endif //__ONVIFPROTOCOL_H__
