#pragma once
namespace NS_KeepAliveNotify
{
	IMPLEMENT_BASE_FUN(E_PT_KEEP_ALIVE_NOTIFY, "Notify","Keepalive")

	//解析请求
	int Parse(string& strXml, void* pData);
	
	//构造请求
	int Build(const void* pData, string& strXml);
}

