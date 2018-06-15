#ifndef __ONVIFPROTOCOL_CmdGetStreamUri_H__GetNetworkInterfaces
#define __ONVIFPROTOCOL_CmdGetStreamUri_H__GetNetworkInterfaces
#include "CmdObject.h"


class CmdGetNetworkInterfaces :public CmdObject
{
public:
	CmdGetNetworkInterfaces(){}
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
	virtual bool parse(const std::string& data) = 0;
};



#endif //__ONVIFPROTOCOL_H__
