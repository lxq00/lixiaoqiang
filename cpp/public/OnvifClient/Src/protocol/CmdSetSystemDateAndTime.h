#ifndef __ONVIFPROTOCOL_CmdGetStreamUri_H__SetSystemDateAndTime
#define __ONVIFPROTOCOL_CmdGetStreamUri_H__SetSystemDateAndTime
#include "CmdObject.h"


class CmdSetSystemDateAndTime :public CmdObject
{
public:
	CmdSetSystemDateAndTime(const Time& _time) :time(_time)
	{
		action = "http://www.onvif.org/ver10/device/wsdl/SetSystemDateAndTime";
	}
	virtual ~CmdSetSystemDateAndTime() {}

	virtual std::string build(const URI& uri)
	{
		stringstream stream;

		stream << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>"
			<< "<s:Envelope " << onvif_xml_ns << ">"
			<< buildHeader(uri)
			<< "<s:Body>"
			<< "<SetSystemDateAndTime xmlns=\"http://www.onvif.org/ver10/device/wsdl\">"
			<< "<DateTimeType>Manual</DateTimeType>"
			<< "<DaylightSavings>false</DaylightSavings>"
			<< "<UTCDateTime>"
			<< "<Time xmlns=\"http://www.onvif.org/ver10/schema\">"
			<< "<Hour>"<<time.hour<<"</Hour>"
			<< "<Minute>"<<time.minute<<"</Minute>"
			<< "<Second>"<<time.second<<"</Second>"
			<< "</Time>"
			<< "<Date xmlns=\"http://www.onvif.org/ver10/schema\">"
			<< "<Year>"<<time.year<<"</Year>"
			<< "<Month>"<<time.month<<"</Month>"
			<< "<Day>"<<time.day<<"</Day>"
			<< "</Date>"
			<< "</UTCDateTime>"
			<< "</SetSystemDateAndTime>"
			<< "</s:Body></s:Envelope>";

		return stream.str();
	}
	virtual bool parse(XMLN * p_xml) { return false; }
private:
	Time time;
};



#endif //__ONVIFPROTOCOL_H__
