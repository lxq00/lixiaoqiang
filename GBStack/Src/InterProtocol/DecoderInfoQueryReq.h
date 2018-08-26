#pragma once
namespace NS_DecoderInfoQueryReq
{
	IMPLEMENT_BASE_FUN(E_PT_DECODER_INFO_QUERY_REQ, "Query", "DecoderInfo")

	//Ω‚Œˆ
	int Parse(string& strXml, void* pData);

	//ππ‘Ï
	int Build(const void* pData, string& strXml);

};

