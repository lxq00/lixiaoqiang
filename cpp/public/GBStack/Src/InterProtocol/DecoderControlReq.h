#pragma once
namespace NS_DecoderControlReq
{
	IMPLEMENT_BASE_FUN(E_PT_DECODER_CONTROL_REQ, "Control", "Decoder")

	//����
	int Parse(string& strXml, void* pData);

	//����
	int Build(const void* pData, string& strXml);

};

