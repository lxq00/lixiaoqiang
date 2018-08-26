#pragma once

namespace NS_VodCtrlRtsp
{
	IMPLEMENT_BASE_FUN(E_PT_VOD_CTRL_RESP, "","")

	//解析请求
	int Parse(string& strXml, void* pData);
	
	//构造请求
	int Build(const void* pData, string& strXml);
}

