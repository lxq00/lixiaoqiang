#pragma once
namespace NS_AlarmNotifyResp
{
	IMPLEMENT_BASE_FUN(E_PT_ALARM_NOTIFY_RESP, "Response","Alarm")

	//��������
	int Parse(string& strXml, void* pData);
	
	//��������
	int Build(const void* pData, string& strXml);

}

