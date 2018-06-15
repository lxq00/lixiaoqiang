#ifndef __ONVIFPROTOCOL_PROFILES_H__SystemReboot
#define __ONVIFPROTOCOL_PROFILES_H__SystemReboot
#include "CmdObject.h"


class CMDSystemReboot :public CmdObject
{
public:
	CMDSystemReboot()
	{
		action = "http://www.onvif.org/ver10/device/wsdl/SystemReboot";
	}
	virtual ~CMDSystemReboot() {}

	virtual std::string build(const URI& uri)
	{
		stringstream stream;

		stream << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>"
			<< "<s:Envelope " << onvif_xml_ns << ">"
			<< buildHeader(uri)
			<< "<s:Body>"
			<< "<tds:SystemReboot/>"
			<< "</s:Body></s:Envelope>";

		return stream.str();
	}
	virtual bool parse(const std::string& data) = 0;
};



#endif //__ONVIFPROTOCOL_H__
