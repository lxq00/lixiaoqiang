#pragma once
namespace NS_DeviceStatusQueryResp
{
	IMPLEMENT_BASE_FUN(E_PT_DEVICE_STATUS_QUERY_RESP, "Response", "DeviceStatus")

	//����
	int Parse(string& strXml, void* pData);
	
	//����
	int Build(const void* pData, string& strXml);
};

