#ifndef __ONVIFPROTOCOL_PROFILES_H__GetAlarm
#define __ONVIFPROTOCOL_PROFILES_H__GetAlarm
#include "CmdObject.h"


class CMDGetAlarm :public CmdObject
{
public:
	CMDGetAlarm()
	{
		action = "http://www.onvif.org/ver10/events/wsdl/PullPointSubscription/PullMessagesRequest";
	}
	virtual ~CMDGetAlarm() {}

	virtual std::string build(const URL& URL)
	{
		stringstream stream;

		stream << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>"
			<< "<s:Envelope xmlns:s=\"http://www.w3.org/2003/05/soap-envelope\" xmlns:a=\"http://www.w3.org/2005/08/addressing\">"
			<< buildCreateHeader(URL)
			<< "<s:Body xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\" xmlns:xsd=\"http://www.w3.org/2001/XMLSchema\">"
			<< "<PullMessages xmlns=\"http://www.onvif.org/ver10/events/wsdl\">"
			<< "<Timeout>PT50S</Timeout>"
			<< "<MessageLimit>1024</MessageLimit>"
			<< "</PullMessages>"
			<< "</s:Body></s:Envelope>";

		return stream.str();
	}

	std::string strAlarmInfo;
	virtual bool parse(const XMLObject::Child& body)
	{
		const XMLObject::Child& response = body.getChild("tev:PullMessagesResponse");
		if (response.isEmpty()) return false;

		return false;
		/*XMLN * p_res = xml_node_soap_get(p_xml, "tev:PullMessagesResponse");
		if (NULL == p_res || recvbuffer == NULL)
		{
			return FALSE;
		}
		bool bMessageUpdate = false;
		const char *pKeyAlarmIn = strstr(recvbuffer, "AlarmIn");
		if (NULL != pKeyAlarmIn)
		{
			bMessageUpdate = true;
			strAlarmInfo = "_AlarmIn";
		}
		const char *pKeyDigitalInput = strstr(recvbuffer, "DigitalInput");
		if (NULL != pKeyDigitalInput)
		{
			bMessageUpdate = true;
			strAlarmInfo = "_AlarmIn";
		}
		const char *pKeyMaskAlarm = strstr(recvbuffer, "MaskAlarm");
		if (NULL != pKeyMaskAlarm)
		{
			bMessageUpdate = true;
			strAlarmInfo += "_MaskAlarm";
		}
		const char *pKeyTamperAlarm = strstr(recvbuffer, "Tamper");
		if (NULL != pKeyTamperAlarm)
		{
			bMessageUpdate = true;
			strAlarmInfo += "_MaskAlarm";
		}
		const char *pKeyMotionAlarm = strstr(recvbuffer, "MotionAlarm");
		if (NULL != pKeyMotionAlarm)
		{
			bMessageUpdate = true;
			strAlarmInfo += "_MotionAlarm";
		}

		return bMessageUpdate;*/
	}
};



#endif //__ONVIFPROTOCOL_H__
