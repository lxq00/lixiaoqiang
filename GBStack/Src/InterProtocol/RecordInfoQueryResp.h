#pragma once
namespace NS_RecordInfoQueryResp
{
	IMPLEMENT_BASE_FUN(E_PT_RECORD_INFO_QUERY_RESP, "Response", "RecordInfo")

	//����
	int Parse(string& strXml, void* pData);
	
	//����
	int Build(const void* pData, string& strXml);
};

