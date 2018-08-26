#pragma once
namespace NS_DeviceStatusQueryResp
{
	IMPLEMENT_BASE_FUN(E_PT_DEVICE_STATUS_QUERY_RESP, "Response", "DeviceStatus")

	//Ω‚Œˆ
	int Parse(string& strXml, void* pData);
	
	//ππ‘Ï
	int Build(const void* pData, string& strXml);
};

