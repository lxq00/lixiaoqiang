#pragma once
namespace NS_AlarmQueryReq
{
	IMPLEMENT_BASE_FUN(E_PT_ALARM_SUBSCRIBE_REQ, "Query","Alarm")

	//解析请求
	int Parse(string& strXml, void* pData);
	
	//构造请求
	int Build(const void* pData, string& strXml);

}

