#pragma once
namespace NS_KeepAliveNotify
{
	IMPLEMENT_BASE_FUN(E_PT_KEEP_ALIVE_NOTIFY, "Notify","Keepalive")

	//��������
	int Parse(string& strXml, void* pData);
	
	//��������
	int Build(const void* pData, string& strXml);
}

