#pragma once
namespace NS_DeviceStatusQueryReq
{
	IMPLEMENT_BASE_FUN(E_PT_DEVICE_STATUS_QUERY_REQ, "Query", "DeviceStatus")

	//����
	int Parse(string& strXml, void* pData);
	
	//����
	int Build(const void* pData, string& strXml);
};

