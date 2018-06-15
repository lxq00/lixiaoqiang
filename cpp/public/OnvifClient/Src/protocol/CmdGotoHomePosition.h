#ifndef __ONVIFPROTOCOL_CmdGetStreamUri_H__GotoHomePosition
#define __ONVIFPROTOCOL_CmdGetStreamUri_H__GotoHomePosition
#include "CmdObject.h"


class CmdGotoHomePosition :public CmdObject
{
public:
	CmdGotoHomePosition() {}
	virtual ~CmdGotoHomePosition() {}

	virtual std::string build(const URI& uri)
	{
		stringstream stream;

		stream << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>"
			<< "<s:Envelope " << onvif_xml_ns << ">"
			<< buildHeader(uri)
			<< "<s:Body>"
			<< "<GotoHomePosition xmlns=\"http://www.onvif.org/ver20/ptz/wsdl\" />"
			<< "</s:Body></s:Envelope>";

		return stream.str();
	}
	virtual bool parse(const std::string& data) = 0;
};



#endif //__ONVIFPROTOCOL_H__
