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
	virtual bool parse(XMLN * p_xml) 
	{
		time = make_shared<Time>();
		time->breakTime(Time::getCurrentTime().makeTime());

		XMLN * p_res = xml_node_soap_get(p_xml, "tds:GetSystemDateAndTimeResponse");
		if (NULL == p_res)
		{
			return false;
		}

		XMLN * p_datetime = xml_node_soap_get(p_res, "tds:SystemDateAndTime");
		if (p_datetime == NULL)
		{
			return false;
		}

		XMLN * p_utc = xml_node_soap_get(p_datetime, "tt:UTCDateTime");
		if (p_utc == NULL)
		{
			return false;
		}

		XMLN * p_time = xml_node_soap_get(p_utc, "tt:Time");
		if (p_utc == NULL)
		{
			return false;
		}

		XMLN * p_hour = xml_node_soap_get(p_time, "tt:Hour");
		if(p_hour && p_hour->data)
		{
			time->hour = atoi(p_hour->data) + time->timeZone/60;
		}
		XMLN * p_min = xml_node_soap_get(p_time, "tt:Minute");
		if (p_min && p_min->data)
		{
			time->minute = atoi(p_min->data);
		}
		XMLN * p_sec = xml_node_soap_get(p_time, "tt:Second");
		if (p_sec && p_sec->data)
		{
			time->second = atoi(p_sec->data);
		}
		
		XMLN * p_date = xml_node_soap_get(p_utc, "tt:Date");
		if (p_date == NULL)
		{
			return false;
		}

		XMLN * p_year = xml_node_soap_get(p_date, "tt:Year");
		if (p_year && p_year->data)
		{
			time->year = atoi(p_year->data);
		}
		XMLN * p_mon = xml_node_soap_get(p_date, "tt:Month");
		if (p_mon && p_mon->data)
		{
			time->month = atoi(p_mon->data);
		}
		XMLN * p_day = xml_node_soap_get(p_date, "tt:Day");
		if (p_day && p_day->data)
		{
			time->day = atoi(p_day->data);
		}
		return true; 
	}
};



#endif //__ONVIFPROTOCOL_H__
