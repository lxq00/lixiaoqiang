#include "Public.h"
#include "AlarmQueryReq.h"

//解析
int NS_AlarmQueryReq::Parse(string& strXml, void* pData)
{
	TAlarmQueryReq* pReq = (TAlarmQueryReq*)pData;
	TiXmlDocument xml;
	int re = MsgParse(xml, strXml, pReq->nSn, pReq->strDeviceId);
	if (re)
		return re;

	TiXmlElement* pRoot = xml.RootElement();
	const char* pValue;
	string strTemp;

	TiXmlElement* pItem = pRoot->FirstChildElement("StartAlarmPriority");
	if (pItem!=NULL && (pValue=pItem->GetText())!=NULL)
		pReq->nStartAlarmPriority = atoi(pValue);
	else
	{
		WRITE_ERROR("解析StartAlarmPriority失败\n");
		return E_EC_INVALID_DOC;
	}

	pItem = pRoot->FirstChildElement("EndAlarmPriority");
	if (pItem!=NULL && (pValue=pItem->GetText())!=NULL)
		pReq->nEndAlarmPriority = atoi(pValue);
	else
	{
		WRITE_ERROR("解析EndAlarmPriority失败\n");
		return E_EC_INVALID_DOC;
	}

	pItem = pRoot->FirstChildElement("AlarmMethod");
	if (pItem!=NULL && (pValue=pItem->GetText())!=NULL)
		pReq->nAlarmMethod = atoi(pValue);
	else
	{
		WRITE_ERROR("解析AlarmMethod失败\n");
		return E_EC_INVALID_DOC;
	}

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

	return E_EC_OK;
}

//构造
int NS_AlarmQueryReq::Build(const void* pData, string& strXml)
{
	const TAlarmQueryReq* pReq = (TAlarmQueryReq*)pData;

	ostringstream oss;
	
	//构造头
	oss	<<XML_HEAD
		<<"<Query>\r\n"
		<<XML_CMD_TYPE("Alarm")
		<<XML_SN(pReq->nSn)
		<<XML_DEVICE_ID(pReq->strDeviceId)
		<<XML_ELEMENT1(StartAlarmPriority, pReq->nStartAlarmPriority)
		<<XML_ELEMENT1(EndAlarmPriority, pReq->nEndAlarmPriority)
		<<XML_ELEMENT1(AlarmMethod, pReq->nAlarmMethod)
		<<XML_ELEMENT1(StartTime, pReq->strStartTime)
		<<XML_ELEMENT1(EndTime, pReq->strEndTime);
	oss << "</Query>\r\n";
	strXml = oss.str();
	return E_EC_OK;
}
