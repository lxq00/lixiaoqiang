#ifndef __ONVIFPROTOCOL_CmdGetStreamURL_H__SetPreset
#define __ONVIFPROTOCOL_CmdGetStreamURL_H__SetPreset
#include "CmdObject.h"


class CmdSetPreset :public CmdObject
{
public:
	CmdSetPreset()
	{
		action = "http://www.onvif.org/ver20/ptz/wsdl/SetPreset";
	}
	virtual ~CmdSetPreset() {}

	virtual std::string build(const URL& URL)
	{
		stringstream stream;

		stream << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>"
			<< "<s:Envelope " << onvif_xml_ns << ">"
			<< buildHeader(URL)
			<< "<s:Body>"
			<< "<SetPreset xmlns=\"http://www.onvif.org/ver20/ptz/wsdl\" />"
			<< "</s:Body></s:Envelope>";

		return stream.str();
	}
	virtual bool parse(const XMLObject::Child& body) = 0;
};



#endif //__ONVIFPROTOCOL_H__
