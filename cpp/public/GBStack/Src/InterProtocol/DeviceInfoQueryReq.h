#pragma once
namespace NS_DeviceInfoQueryReq
{
	IMPLEMENT_BASE_FUN(E_PT_DEVICE_INFO_QUERY_REQ, "Query", "DeviceInfo")

	//Ω‚Œˆ
	int Parse(string& strXml, void* pData);
	
	//ππ‘Ï
	int Build(const void* pData, string& strXml);
};

