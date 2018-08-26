#include "Public.h"
#include "RecordInfoQueryReq.h"
#include "InterProtocol/InterProtocol.h"

//类型表
const char* g_pRecordInfoType[] = 
{"", "all", "time","alarm","manual"};

//解析请求
int NS_RecordInfoQueryReq::Parse(string& strXml, void* pData)
{
	TRecordInfoQueryReq* pReq = (TRecordInfoQueryReq*)pData;
	TiXmlDocument xml;
	int re = MsgParse(xml, strXml, pReq->nSn, pReq->strDeviceId);
	if (re)
		return re;
	TiXmlElement* pRoot = xml.RootElement();
	TiXmlElement* pItem;
	const char* pValue;

	pItem = pRoot->FirstChildElement("StartTime");
	if (pItem!=NULL && (pValue=pItem->GetText())!=NULL)
		pReq->strStartTime = pValue;
	else
		pReq->strStartTime = "";

	pItem = pRoot->FirstChildElement("EndTime");
	if (pItem!=NULL && (pValue=pItem->GetText())!=NULL)
		pReq->strEndTime = pValue;
	else
		pReq->strEndTime = "";

	pItem = pRoot->FirstChildElement("FilePath");
	if (pItem!=NULL && (pValue=pItem->GetText())!=NULL)
		pReq->strFilePath = pValue;
	else
		pReq->strFilePath = "";

	pItem = pRoot->FirstChildElement("Address");
	if (pItem!=NULL && (pValue=pItem->GetText())!=NULL)
		pReq->strAddress = pValue;
	else
		pReq->strAddress = "";

	pItem = pRoot->FirstChildElement("Secrecy");
	if (pItem!=NULL && (pValue=pItem->GetText())!=NULL)
		pReq->nSecrecy = atoi(pValue);
	else
		pReq->nSecrecy = 0;

	pItem = pRoot->FirstChildElement("Type");
	if (pItem!=NULL && (pValue=pItem->GetText())!=NULL)
	{
		pReq->eType = E_FQT_NONE;
		for(int i=E_FQT_ALL; i<=E_FQT_MANUAL; i++)
		{
			if (strcmp(g_pRecordInfoType[i], pValue)==0)
			{
				pReq->eType = (ERecordInfoType)i;
				break;
			}
		}
	}
	else
		pReq->eType = E_FQT_NONE;

	pItem = pRoot->FirstChildElement("RecorderID");
	if (pItem!=NULL && (pValue=pItem->GetText())!=NULL)
		pReq->strRecorderID = pValue;
	else
		pReq->strRecorderID = "";

	if(pReq->strEndTime == "")
	{
		pReq->strEndTime = SecToSipTime(time(NULL));
	}

	if(pReq->strStartTime == "")
	{
		pReq->strStartTime = SecToSipTime(SipTimeToSec(pReq->strEndTime) - 60*60*4);
	}

	return E_EC_OK;
}

//构造请求
int NS_RecordInfoQueryReq::Build(const void* pData, string& strXml)
{
	TRecordInfoQueryReq* pReq = (TRecordInfoQueryReq*)pData;
	
	ostringstream oss;
	//构造头
	oss	<<XML_HEAD
		<<"<Query>\r\n"
		<<XML_CMD_TYPE("RecordInfo")
		<<XML_SN(pReq->nSn)
		<<XML_DEVICE_ID(pReq->strDeviceId);
	if (!pReq->strStartTime.empty())
		oss<<XML_ELEMENT1(StartTime, pReq->strStartTime);
	if (!pReq->strEndTime.empty())
		oss<<XML_ELEMENT1(EndTime, pReq->strEndTime);
	if (!pReq->strFilePath.empty())
		oss<<XML_ELEMENT1(FilePath, pReq->strFilePath);
	if (!pReq->strAddress.empty())
		oss<<XML_ELEMENT1(Address, pReq->strAddress);
	oss<<XML_ELEMENT1(Secrecy, pReq->nSecrecy);
	if (pReq->eType>E_FQT_NONE && pReq->eType<=E_FQT_MANUAL)
		oss<<XML_ELEMENT1(Type, g_pRecordInfoType[pReq->eType]);
	else
		oss<<XML_ELEMENT1(Type, g_pRecordInfoType[E_FQT_ALL]);
	if (!pReq->strRecorderID.empty())
		oss<<XML_ELEMENT1(RecorderID, pReq->strRecorderID);
	oss<<"</Query>\r\n";
	strXml = oss.str();

	return E_EC_OK;
}

