#pragma once

#include <string.h>


namespace NS_VodCtrl
{
	IMPLEMENT_BASE_FUN(E_PT_VOD_CTRL_REQ, "","")

	//解析请求
	int Parse(string& strXml, void* pData);
	
	//构造请求
	int Build(const void* pData, string& strXml);
}

