#ifndef __ONVIFPROTOCOL_CmdGetStreamUri_H__GotoPreset
#define __ONVIFPROTOCOL_CmdGetStreamUri_H__GotoPreset
#include "CmdObject.h"


class CmdGotoPreset :public CmdObject
{
public:
	CmdGotoPreset() {}
	virtual ~CmdGotoPreset() {}

	virtual std::string build(const URI& uri)
	{
		stringstream stream;

		stream << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>"
			<< "<s:Envelope " << onvif_xml_ns << ">"
			<< buildHeader(uri)
			<< "<s:Body>"
			<< "<GotoPreset xmlns=\"http://www.onvif.org/ver20/ptz/wsdl\" />"
			<< "</s:Body></s:Envelope>";

		return stream.str();
	}
	virtual bool parse(const std::string& data) = 0;
};



#endif //__ONVIFPROTOCOL_H__
