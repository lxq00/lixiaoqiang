#pragma once
namespace NS_RecordInfoQueryReq
{
	IMPLEMENT_BASE_FUN(E_PT_RECORD_INFO_QUERY_REQ, "Query", "RecordInfo")

	//����
	int Parse(string& strXml, void* pData);
	
	//����
	int Build(const void* pData, string& strXml);
};

