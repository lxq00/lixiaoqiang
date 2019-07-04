#ifndef __ONVIFPROTOCOL_CmdGetStreamURL_H__GetSystemDateAndTime
#define __ONVIFPROTOCOL_CmdGetStreamURL_H__GetSystemDateAndTime
#include "CmdObject.h"


class CmdGetSystemDateAndTime :public CmdObject
{
public:
	CmdGetSystemDateAndTime()
	{
		action = "http://www.onvif.org/ver10/device/wsdl/GetSystemDateAndTime";
	}
	virtual ~CmdGetSystemDateAndTime() {}

	virtual std::string build(const URL& URL)
	{
		stringstream stream;

		stream << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>"
			<< "<s:Envelope " << onvif_xml_ns << ">"
			<< buildHeader(URL)
			<< "<s:Body>"
			<< "<GetSystemDateAndTime xmlns=\"http://www.onvif.org/ver10/device/wsdl\" />"
			<< "</s:Body></s:Envelope>";

		return stream.str();
	}
	shared_ptr<Time> time;
	virtual bool parse(const XMLObject::Child& body)
	{
		time = make_shared<Time>();
		time->breakTime(Time::getCurrentTime().makeTime());

		const XMLObject::Child & resp = body.getChild("tds:GetSystemDateAndTimeResponse");
		if (!resp) return false;

		const XMLObject::Child& systemdate = resp.getChild("tds:SystemDateAndTime");
		if (!systemdate) return false;

		const XMLObject::Child& utcdate = systemdate.getChild("tds:UTCDateTime");
		if (!utcdate) return false;
		
		time->hour = utcdate.getChild("tt:Time").getChild("tt:Hour").getValue().readInt();
		time->minute = utcdate.getChild("tt:Time").getChild("tt:Minute").getValue().readInt();
		time->second = utcdate.getChild("tt:Time").getChild("tt:Second").getValue().readInt();

		time->year = utcdate.getChild("tt:Date").getChild("tt:Year").getValue().readInt();
		time->month = utcdate.getChild("tt:Date").getChild("tt:Month").getValue().readInt();
		time->day = utcdate.getChild("tt:Date").getChild("tt:Day").getValue().readInt();
		
		
		return true; 
	}
};



#endif //__ONVIFPROTOCOL_H__
