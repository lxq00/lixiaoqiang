#pragma once
namespace NS_DecoderControlReq
{
	IMPLEMENT_BASE_FUN(E_PT_DECODER_CONTROL_REQ, "Control", "Decoder")

	//Ω‚Œˆ
	int Parse(string& strXml, void* pData);

	//ππ‘Ï
	int Build(const void* pData, string& strXml);

};

