
/**
 * 协议控制类实现文件
 *
 * 用来将文本解析成结构体
 * 或者把结构体转化成字符串
 *
 * 单个对象不是线程安全的
 * 自行保证线程安全，或每个线程分配一个对象
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

//函数结构体定义
struct TProtocolFun
{
	//获取协议类型
	EProtocolType (*fGetProType)();
	//获取功能名称
	const char* (*fGetFunName)();
	//获取命令类型名称
	const char* (*fGetCmdName)();
	//解析函数
	int (*fParse)(string&,void*);
	//构造函数
	int (*fBuild)(const void*,string&);
};

#define ADD_CLASS_TO_FUN_TABLE(class_name) \
	{class_name::GetProType,class_name::GetFunName,class_name::GetCmdName, \
		class_name::Parse,class_name::Build} 

//定义函数表
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
//定义函数表
TProtocolFun g_tExtProtocolFunTable[] = 
	{
		ADD_CLASS_TO_FUN_TABLE(NS_VodCtrl),
		ADD_CLASS_TO_FUN_TABLE(NS_VodCtrlRtsp),
		{NULL, NULL, NULL}
	};


//获取字符串的类型
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
	//	WRITE_ERROR("不是有效的xml文档\n");
	//	return E_EC_INVALID_DOC;
	}

	//获取功能名称
	TiXmlElement* pElement = xml.RootElement();
	if(pElement == NULL)
	{
		return E_EC_INVALID_DOC;
	}
	string strFunctionName = pElement->Value();

	//获取命令类型
	pElement = pElement->FirstChildElement("CmdType");
	if (pElement==NULL)
	{
	//	WRITE_ERROR("缺少CmdType标签\n");
		return E_EC_PARSE_XML_DOC;
	}
	const char* pTemp = pElement->GetText();
	if (pTemp==NULL)
	{
	//	WRITE_ERROR("读取CmdType数据出错\n");
		return E_EC_PARSE_XML_DOC;
	}
	string strCmdType = pTemp;

	//获取sn
	if (pSn!=NULL)
	{
		pElement = pElement->NextSiblingElement("SN");
		if (pElement==NULL)
		{
	//		WRITE_ERROR("缺少SN标签\n");
			return E_EC_PARSE_XML_DOC;
		}
		pTemp = pElement->GetText();
		if (pTemp==NULL)
		{
	//		WRITE_ERROR("读取SN数据出错\n");
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

	//WRITE_ERROR("无效类型%s\n", strCmdType.c_str());
	return E_EC_INVALID_CMD_TYPE;
}

//解析字符串
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
	WRITE_ERROR("无效类型%d\n", eType);
	return E_EC_INVALID_CMD_TYPE;
}

//构造字符串
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
	WRITE_ERROR("无效类型%d\n", eType);
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




