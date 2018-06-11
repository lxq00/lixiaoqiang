#include "Public.h"
#include "AlarmNotifyReq.h"

//解析
int NS_AlarmNotifyReq::Parse(string& strXml, void* pData)
{
	TAlarmNotifyReq* pReq = (TAlarmNotifyReq*)pData;
	TiXmlDocument xml;
	int re = MsgParse(xml, strXml, pReq->nSn, pReq->strDeviceId, &pReq->vExtendInfo);
	if (re)
		return re;

	TiXmlElement* pRoot = xml.RootElement();
	const char* pValue;
	string strTemp;

	TiXmlElement* pItem = pRoot->FirstChildElement("AlarmPriority");
	if (pItem!=NULL && (pValue=pItem->GetText())!=NULL)
		pReq->nAlarmPriority = atoi(pValue);
	else
	{
		WRITE_ERROR("解析AlarmPriority失败\n");
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

	pItem = pRoot->FirstChildElement("AlarmTime");
	if (pItem!=NULL && (pValue=pItem->GetText())!=NULL)
		pReq->strAlarmTime = pValue;
	else
		pReq->strAlarmTime = "";

	pItem = pRoot->FirstChildElement("AlarmDescription");
	if (pItem!=NULL && (pValue=pItem->GetText())!=NULL)
		pReq->strAlarmDescription = pValue;
	else
		pReq->strAlarmDescription = "";


	pItem = pRoot->FirstChildElement("Longitude");
	if (pItem!=NULL && (pValue=pItem->GetText())!=NULL)
		pReq->dLongitude = atof(pValue);
	else
		pReq->dLongitude = -1;

	pItem = pRoot->FirstChildElement("Latitude");
	if (pItem!=NULL && (pValue=pItem->GetText())!=NULL)
		pReq->dLatitude = atof(pValue);
	else
		pReq->dLatitude = -1;
	return E_EC_OK;
}

//构造
int NS_AlarmNotifyReq::Build(const void* pData, string& strXml)
{
	const TAlarmNotifyReq* pReq = (TAlarmNotifyReq*)pData;

	ostringstream oss;
	
	//构造头
	oss	<<XML_HEAD
		<<"<Notify>\r\n"
		<<XML_CMD_TYPE("Alarm")
		<<XML_SN(pReq->nSn)
		<<XML_DEVICE_ID(pReq->strDeviceId)
		<<XML_ELEMENT1(AlarmPriority, pReq->nAlarmPriority)
		<<XML_ELEMENT1(AlarmMethod, pReq->nAlarmMethod);
	if (!pReq->strAlarmTime.empty())
		oss << XML_ELEMENT1(AlarmTime, pReq->strAlarmTime);
	if (!pReq->strAlarmDescription.empty())
		oss << XML_ELEMENT1(AlarmDescription, pReq->strAlarmDescription);
	if (pReq->dLongitude!=-1)
		oss << XML_ELEMENT1(Longitude, pReq->dLongitude);
	if (pReq->dLatitude!=-1)
		oss << XML_ELEMENT1(Latitude, pReq->dLatitude);
	MsgBuildExtendInfo(pReq->vExtendInfo,oss);
	oss << "</Notify>\r\n";
	strXml = oss.str();
	return E_EC_OK;
}
