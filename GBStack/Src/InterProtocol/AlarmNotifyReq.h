#pragma once
namespace NS_AlarmNotifyReq
{
	IMPLEMENT_BASE_FUN(E_PT_ALARM_NOTIFY_REQ, "Notify","Alarm")

	//��������
	int Parse(string& strXml, void* pData);
	
	//��������
	int Build(const void* pData, string& strXml);

}

