#pragma once

namespace NS_VodCtrlRtsp
{
	IMPLEMENT_BASE_FUN(E_PT_VOD_CTRL_RESP, "","")

	//��������
	int Parse(string& strXml, void* pData);
	
	//��������
	int Build(const void* pData, string& strXml);
}

