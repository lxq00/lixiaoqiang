#pragma once
namespace NS_DeviceCatalogQueryResp
{
	IMPLEMENT_BASE_FUN(E_PT_DEVICE_CATALOG_QUERY_RESP, "Response", "Catalog")

	//����
	int Parse(string& strXml, void* pData);
	
	//����
	int Build(const void* pData, string& strXml);
};

