#pragma once
namespace NS_DeviceControlResp
{
	IMPLEMENT_BASE_FUN(E_PT_DEVICE_CONTROL_RESP, "Response","DeviceControl")

	//��������
	int Parse(string& strXml, void* pData);
	
	//��������
	int Build(const void* pData, string& strXml);

}

