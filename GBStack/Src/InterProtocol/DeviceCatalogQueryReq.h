#pragma once
namespace NS_DeviceCatalogQueryReq
{
	IMPLEMENT_BASE_FUN(E_PT_DEVICE_CATALOG_QUERY_REQ, "Query", "Catalog")

	//Ω‚Œˆ
	int Parse(string& strXml, void* pData);
	
	//ππ‘Ï
	int Build(const void* pData, string& strXml);
};

