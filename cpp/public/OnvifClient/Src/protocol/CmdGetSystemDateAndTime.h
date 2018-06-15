#ifndef __ONVIFPROTOCOL_CmdGetStreamUri_H__GetSystemDateAndTime
#define __ONVIFPROTOCOL_CmdGetStreamUri_H__GetSystemDateAndTime
#include "CmdObject.h"


class CmdGetSystemDateAndTime :public CmdObject
{
public:
	CmdGetSystemDateAndTime(){}
	virtual ~CmdGetSystemDateAndTime() {}

	virtual std::string build(const URI& uri)
	{
		stringstream stream;

		stream << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>"
			<< "<s:Envelope " << onvif_xml_ns << ">"
			<< buildHeader(uri)
			<< "<s:Body>"
			<< "<GetSystemDateAndTime xmlns=\"http://www.onvif.org/ver10/device/wsdl\" />"
			<< "</s:Body></s:Envelope>";

		return stream.str();
	}
	virtual bool parse(const std::string& data) = 0;
};



#endif //__ONVIFPROTOCOL_H__
