
/**
 * Э�������ʵ���ļ�
 *
 * �������ı������ɽṹ��
 * ���߰ѽṹ��ת�����ַ���
 *
 * �����������̰߳�ȫ��
 * ���б�֤�̰߳�ȫ����ÿ���̷߳���һ������
 */

#include "Public.h"
#include "InterProtocol/InterProtocol.h"
#include "DeviceControlReq.h"
#include "DeviceControlResp.h"
#include "DeviceCatalogQueryReq.h"
#include "DeviceCatalogQueryResp.h"
#include "DeviceInfoQueryReq.h"
#include "DeviceInfoQueryResp.h"
#include "DeviceStatusQueryReq.h"
#include "DeviceStatusQueryResp.h"
#include "RecordInfoQueryReq.h"
#include "RecordInfoQueryResp.h"
#include "AlarmNotifyReq.h"
#include "AlarmNotifyResp.h"
#include "KeepAliveNotify.h"
#include "MeidaStatusNotify.h"
#include "Rtsp.h"
#include "DecoderControlReq.h"
#include "DecoderInfoQueryReq.h"
#include "DecoderInfoQueryResp.h"
#include "AlarmQueryReq.h"
#include "RtspResp.h"
using namespace NS_VodCtrlRtsp;

//�����ṹ�嶨��
struct TProtocolFun
{
	//��ȡЭ������
	EProtocolType (*fGetProType)();
	//��ȡ��������
	const char* (*fGetFunName)();
	//��ȡ������������
	const char* (*fGetCmdName)();
	//��������
	int (*fParse)(string&,void*);
	//���캯��
	int (*fBuild)(const void*,string&);
};

#define ADD_CLASS_TO_FUN_TABLE(class_name) \
	{class_name::GetProType,class_name::GetFunName,class_name::GetCmdName, \
		class_name::Parse,class_name::Build} 

//���庯����
TProtocolFun g_tProtocolFunTable[] = 
	{
		ADD_CLASS_TO_FUN_TABLE(NS_DeviceControlReq),
		ADD_CLASS_TO_FUN_TABLE(NS_DeviceControlResp),
		ADD_CLASS_TO_FUN_TABLE(NS_DeviceCatalogQueryReq),
		ADD_CLASS_TO_FUN_TABLE(NS_DeviceCatalogQueryResp),
		ADD_CLASS_TO_FUN_TABLE(NS_DeviceInfoQueryReq),
		ADD_CLASS_TO_FUN_TABLE(NS_DeviceInfoQueryResp),
		ADD_CLASS_TO_FUN_TABLE(NS_DeviceStatusQueryReq),
		ADD_CLASS_TO_FUN_TABLE(NS_DeviceStatusQueryResp),
		ADD_CLASS_TO_FUN_TABLE(NS_RecordInfoQueryReq),
		ADD_CLASS_TO_FUN_TABLE(NS_RecordInfoQueryResp),
		ADD_CLASS_TO_FUN_TABLE(NS_AlarmNotifyReq),
		ADD_CLASS_TO_FUN_TABLE(NS_AlarmNotifyResp),
		ADD_CLASS_TO_FUN_TABLE(NS_AlarmQueryReq),
		ADD_CLASS_TO_FUN_TABLE(NS_KeepAliveNotify),
		ADD_CLASS_TO_FUN_TABLE(NS_MeidaStatusNotify),
		ADD_CLASS_TO_FUN_TABLE(NS_DecoderControlReq),
		ADD_CLASS_TO_FUN_TABLE(NS_DecoderInfoQueryReq),
		ADD_CLASS_TO_FUN_TABLE(NS_DecoderInfoQueryResp),
		ADD_CLASS_TO_FUN_TABLE(NS_MeidaStatusNotify),
		{NULL, NULL, NULL}
	};
//���庯����
TProtocolFun g_tExtProtocolFunTable[] = 
	{
		ADD_CLASS_TO_FUN_TABLE(NS_VodCtrl),
		ADD_CLASS_TO_FUN_TABLE(NS_VodCtrlRtsp),
		{NULL, NULL, NULL}
	};


//��ȡ�ַ���������
int GetMsgType(string strXml, EProtocolType& eType, int* pSn)
{
	TiXmlDocument xml;
	xml.Parse(strXml.c_str());
	if (xml.Error())
	{
		TProtocolFun* pFun = g_tExtProtocolFunTable;
		while (pFun->fGetProType!=NULL)
		{
			if (pFun->fParse(strXml, NULL) == E_EC_OK)
			{
				eType = pFun->fGetProType();
				return E_EC_OK;
			}
			pFun++;
		}
		return E_EC_INVALID_DOC;
	//	WRITE_ERROR("������Ч��xml�ĵ�\n");
	//	return E_EC_INVALID_DOC;
	}

	//��ȡ��������
	TiXmlElement* pElement = xml.RootElement();
	if(pElement == NULL)
	{
		return E_EC_INVALID_DOC;
	}
	string strFunctionName = pElement->Value();

	//��ȡ��������
	pElement = pElement->FirstChildElement("CmdType");
	if (pElement==NULL)
	{
	//	WRITE_ERROR("ȱ��CmdType��ǩ\n");
		return E_EC_PARSE_XML_DOC;
	}
	const char* pTemp = pElement->GetText();
	if (pTemp==NULL)
	{
	//	WRITE_ERROR("��ȡCmdType���ݳ���\n");
		return E_EC_PARSE_XML_DOC;
	}
	string strCmdType = pTemp;

	//��ȡsn
	if (pSn!=NULL)
	{
		pElement = pElement->NextSiblingElement("SN");
		if (pElement==NULL)
		{
	//		WRITE_ERROR("ȱ��SN��ǩ\n");
			return E_EC_PARSE_XML_DOC;
		}
		pTemp = pElement->GetText();
		if (pTemp==NULL)
		{
	//		WRITE_ERROR("��ȡSN���ݳ���\n");
			return E_EC_PARSE_XML_DOC;
		}
		*pSn = atoi(pTemp);
	}

	TProtocolFun* pFun = g_tProtocolFunTable;
	while (pFun->fGetProType!=NULL)
	{
		if (strCmdType==pFun->fGetCmdName() && 
			strFunctionName==pFun->fGetFunName())
		{
			eType = pFun->fGetProType();
			return E_EC_OK;
		}
		pFun++;
	}

	//WRITE_ERROR("��Ч����%s\n", strCmdType.c_str());
	return E_EC_INVALID_CMD_TYPE;
}

//�����ַ���
int ParseMsg(string strXml, EProtocolType eType, void* pData)
{
	TProtocolFun* pFun = g_tExtProtocolFunTable;
	while (pFun->fGetProType!=NULL)
	{
		if (pFun->fGetProType()==eType)
			return pFun->fParse(strXml, pData);
		pFun++;
	}

	
	pFun = g_tProtocolFunTable;
	while (pFun->fGetProType!=NULL)
	{
		if (pFun->fGetProType()==eType)
			return pFun->fParse(strXml, pData);
		pFun++;
	}
	WRITE_ERROR("��Ч����%d\n", eType);
	return E_EC_INVALID_CMD_TYPE;
}

//�����ַ���
int BuildMsg(EProtocolType eType, const void* pData, string& strXml)
{
	TProtocolFun* pFun = g_tExtProtocolFunTable;
	while (pFun->fGetProType!=NULL)
	{
		if (pFun->fGetProType()==eType)
			return pFun->fBuild(pData, strXml);
		pFun++;
	}
	
	pFun = g_tProtocolFunTable;
	while (pFun->fGetProType!=NULL)
	{
		if (pFun->fGetProType()==eType)
			return pFun->fBuild(pData, strXml);
		pFun++;
	}
	WRITE_ERROR("��Ч����%d\n", eType);
	return E_EC_INVALID_CMD_TYPE;
}

unsigned long long SipTimeToSec(const string& timefromt)
{
	struct tm Tm;
	if(sscanf(timefromt.c_str(),"%4d-%02d-%02dT%02d:%02d:%02d",&Tm.tm_year, &Tm.tm_mon, &Tm.tm_mday, &Tm.tm_hour, \
		&Tm.tm_min, &Tm.tm_sec) == 6)
	{
		Tm.tm_year -= 1900;
		Tm.tm_mon -= 1;
		time_t sertime = mktime(&Tm);
		return sertime;
	}
	
	return 0;	
}

string SecToSipTime(unsigned long long sectime)
{
	time_t timet = sectime;
	struct tm* Tm = localtime(&timet);

	if(Tm == NULL)
	{
		return "";
	}

	static char timestr[64];
	Tm->tm_year += 1900;
	Tm->tm_mon += 1;
	
	sprintf(timestr,"%4d-%02d-%02dT%02d:%02d:%02d",Tm->tm_year, Tm->tm_mon, Tm->tm_mday, Tm->tm_hour, Tm->tm_min, Tm->tm_sec);

	return timestr;
}

string SecToSipRegTime(unsigned long long sectime)
{
	time_t timet = sectime;
	struct tm* Tm = localtime(&timet);

	if(Tm == NULL)
	{
		return "";
	}

	static char timestr[128];
	Tm->tm_year += 1900;
	Tm->tm_mon += 1;

	sprintf(timestr,"%4d-%02d-%02dT%02d:%02d:%02d.%03d",Tm->tm_year, Tm->tm_mon, Tm->tm_mday, Tm->tm_hour, Tm->tm_min, Tm->tm_sec,rand()%1000);

	return timestr;
}




