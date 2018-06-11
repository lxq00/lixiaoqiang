#pragma once
namespace NS_RecordInfoQueryResp
{
	IMPLEMENT_BASE_FUN(E_PT_RECORD_INFO_QUERY_RESP, "Response", "RecordInfo")

	//Ω‚Œˆ
	int Parse(string& strXml, void* pData);
	
	//ππ‘Ï
	int Build(const void* pData, string& strXml);
};

