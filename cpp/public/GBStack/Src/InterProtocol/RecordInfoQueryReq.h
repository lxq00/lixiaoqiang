#pragma once
namespace NS_RecordInfoQueryReq
{
	IMPLEMENT_BASE_FUN(E_PT_RECORD_INFO_QUERY_REQ, "Query", "RecordInfo")

	//Ω‚Œˆ
	int Parse(string& strXml, void* pData);
	
	//ππ‘Ï
	int Build(const void* pData, string& strXml);
};

