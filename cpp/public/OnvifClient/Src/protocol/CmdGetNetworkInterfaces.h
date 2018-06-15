#ifndef __ONVIFPROTOCOL_CmdGetStreamUri_H__GetNetworkInterfaces
#define __ONVIFPROTOCOL_CmdGetStreamUri_H__GetNetworkInterfaces
#include "CmdObject.h"


class CmdGetNetworkInterfaces :public CmdObject
{
public:
	CmdGetNetworkInterfaces()
	{
		action = "http://www.onvif.org/ver10/device/wsdl/GetNetworkInterfaces";
	}
	virtual ~CmdGetNetworkInterfaces() {}

	virtual std::string build(const URI& uri)
	{
		stringstream stream;

		stream << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>"
			<< "<s:Envelope " << onvif_xml_ns << ">"
			<< buildHeader(uri)
			<< "<s:Body>"
			<< "<tds:GetNetworkInterfaces></tds:GetNetworkInterfaces>"
			<<  "</s:Body></s:Envelope>";

		return stream.str();
	}

	shared_ptr<OnvifClientDefs::NetworkInterfaces> network;
	virtual bool parse(XMLN * p_xml) { return false; }
};



#endif //__ONVIFPROTOCOL_H__
