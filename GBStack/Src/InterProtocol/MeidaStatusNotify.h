#ifndef __MEDIASTATUSNOTIFY_H__
#define __MEDIASTATUSNOTIFY_H__

#pragma once
namespace NS_MeidaStatusNotify
{
	IMPLEMENT_BASE_FUN(E_PT_MEDIASTATUS_NOTIFY, "Notify", "MediaStatus")

	//����
	int Parse(string& strXml, void* pData);
	
	//����
	int Build(const void* pData, string& strXml);
};


#endif //__MEDIASTATUSNOTIFY_H__

