#pragma once
namespace NS_AlarmNotifyResp
{
	IMPLEMENT_BASE_FUN(E_PT_ALARM_NOTIFY_RESP, "Response","Alarm")

	//解析请求
	int Parse(string& strXml, void* pData);
	
	//构造请求
	int Build(const void* pData, string& strXml);

}

