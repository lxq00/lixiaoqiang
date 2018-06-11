#pragma once
namespace NS_DeviceControlResp
{
	IMPLEMENT_BASE_FUN(E_PT_DEVICE_CONTROL_RESP, "Response","DeviceControl")

	//解析请求
	int Parse(string& strXml, void* pData);
	
	//构造请求
	int Build(const void* pData, string& strXml);

}

