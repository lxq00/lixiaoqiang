#pragma once
namespace NS_AlarmQueryReq
{
	IMPLEMENT_BASE_FUN(E_PT_ALARM_SUBSCRIBE_REQ, "Query","Alarm")

	//��������
	int Parse(string& strXml, void* pData);
	
	//��������
	int Build(const void* pData, string& strXml);

}

