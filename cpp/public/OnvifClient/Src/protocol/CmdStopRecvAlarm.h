#ifndef __ONVIFPROTOCOL_CmdGetStreamUri_H__StopRecvAlarm
#define __ONVIFPROTOCOL_CmdGetStreamUri_H__StopRecvAlarm
#include "CmdObject.h"


class CmdStopRecvAlarm :public CmdObject
{
public:
	CmdStopRecvAlarm(){}
	virtual ~CmdStopRecvAlarm() {}

	virtual std::string build(const URI& uri)
	{
		stringstream stream;

		stream << "<?xml version=\"1.0\" encoding=\"utf-8\"?>"
			<< "<s:Envelope "<< onvif_xml_ns <<">"
			<< buildHeader(uri)
			<< "<s:Body>"
			<< "<tds:SystemReboot/>"
			<< "</s:Body></s:Envelope>";

		return stream.str();
	}
	virtual bool parse(const std::string& data) = 0;
};



#endif //__ONVIFPROTOCOL_H__
