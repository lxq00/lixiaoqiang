#pragma once
namespace NS_DeviceCatalogQueryResp
{
	IMPLEMENT_BASE_FUN(E_PT_DEVICE_CATALOG_QUERY_RESP, "Response", "Catalog")

	//Ω‚Œˆ
	int Parse(string& strXml, void* pData);
	
	//ππ‘Ï
	int Build(const void* pData, string& strXml);
};

