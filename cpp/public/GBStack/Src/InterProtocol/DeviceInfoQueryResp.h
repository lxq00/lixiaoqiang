#pragma once
namespace NS_DeviceInfoQueryResp
{
	IMPLEMENT_BASE_FUN(E_PT_DEVICE_INFO_QUERY_RESP, "Response", "DeviceInfo")

	//����
	int Parse(string& strXml, void* pData);
	
	//����
	int Build(const void* pData, string& strXml);
};

