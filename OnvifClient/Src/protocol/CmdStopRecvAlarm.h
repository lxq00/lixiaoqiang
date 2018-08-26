#ifndef __ONVIFPROTOCOL_CmdGetStreamURL_H__StopRecvAlarm
#define __ONVIFPROTOCOL_CmdGetStreamURL_H__StopRecvAlarm
#include "CmdObject.h"


class CmdStopRecvAlarm :public CmdObject
{
public:
	CmdStopRecvAlarm()
	{
		action = "http://www.onvif.org/ver10/events/wsdl/PullPointSubscription/PullMessagesRequest";
	}
	virtual ~CmdStopRecvAlarm() {}

	virtual std::string build(const URL& URL)
	{
		stringstream stream;

		stream << "<?xml version=\"1.0\" encoding=\"utf-8\"?>"
			<< "<s:Envelope "<< onvif_xml_ns <<">"
			<< buildHeader(URL)
			<< "<s:Body>"
			<< "<tds:SystemReboot/>"
			<< "</s:Body></s:Envelope>";

		return stream.str();
	}
	virtual bool parse(const std::string& data) = 0;
};



#endif //__ONVIFPROTOCOL_H__
