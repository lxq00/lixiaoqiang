#pragma once

#include <string.h>


namespace NS_VodCtrl
{
	IMPLEMENT_BASE_FUN(E_PT_VOD_CTRL_REQ, "","")

	//��������
	int Parse(string& strXml, void* pData);
	
	//��������
	int Build(const void* pData, string& strXml);
}

