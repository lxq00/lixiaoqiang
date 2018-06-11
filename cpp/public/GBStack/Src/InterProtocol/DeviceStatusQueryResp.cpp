#include "Public.h"
#include "DeviceStatusQueryResp.h"



//解析请求
int NS_DeviceStatusQueryResp::Parse(string& strXml, void* pData)
{
	TDeviceStatusQueryResp* pResp = (TDeviceStatusQueryResp*)pData;
	TiXmlDocument xml;
	int re =  MsgParse(xml, strXml, pResp->nSn, pResp->strDeviceId, &pResp->vExtendInfo);
	if (re)
		return re;
	TiXmlElement* pRoot = xml.RootElement();
	re = MsgParseResult(pRoot->FirstChildElement("Result"), pResp->bResult);
	if (re)
		return re;

	TiXmlElement* pItem;
	const char* pValue;

	pItem = pRoot->FirstChildElement("Online");
	if (pItem!=NULL && (pValue=pItem->GetText())!=NULL)
	{
		if (strcmp(pValue, "ONLINE")==0)
			pResp->bOnline = 1;
		else if (strcmp(pValue, "OFFONLINE")==0)
			pResp->bOnline = 0;
		else
		{
			pResp->bOnline = 0;	
		}
	}
	else
	{
		WRITE_ERROR("解析ONLINE元素失败\n");
		return E_EC_PARSE_XML_DOC;
	}

	re = MsgParseStatus(pRoot->FirstChildElement("Status"), pResp->bStatus);
	if (re)
		return re;

	pItem = pRoot->FirstChildElement("Reason");
	if (pItem!=NULL && (pValue=pItem->GetText())!=NULL)
		pResp->strReason = pValue;
	else
		pResp->strReason = "";


	MsgParseStatus(pRoot->FirstChildElement("Encode"), pResp->bEncode);
	MsgParseStatus(pRoot->FirstChildElement("Record"), pResp->bRecord);


	pItem = pRoot->FirstChildElement("DeviceTime");
	if (pItem!=NULL && (pValue=pItem->GetText())!=NULL)
		pResp->strDeviceTime = pValue;
	else
		pResp->strDeviceTime = "";
	
	pItem = pRoot->FirstChildElement("Alarmstatus");
	pResp->vTAlarmStatus.clear();
	if (pItem!=NULL)
	{
		TAlarmStatus AlarmStatus;
		pItem = pItem->FirstChildElement("Item");
		TiXmlElement* pSubItem;
		while (pItem!=NULL)
		{
			pSubItem = pItem->FirstChildElement("DeviceID");
			if (pSubItem==NULL || (pValue=pSubItem->GetText())==NULL)
			{
				pItem = pItem->NextSiblingElement("Item");
				continue;
			}
			AlarmStatus.strDeviceId = pValue;
			pSubItem = pItem->FirstChildElement("DutyStatus");
			if (pSubItem==NULL || (pValue=pSubItem->GetText())==NULL)
			{
				pItem = pItem->NextSiblingElement("Item");
				continue;
			}
			if (strcmp(pValue, "ONDUTY")==0)
				AlarmStatus.eAlarmStatus = E_AS_ONDUTY;
			else if(strcmp(pValue, "OFFDUTY")==0)
				AlarmStatus.eAlarmStatus = E_AS_OFFDUTY;
			else if(strcmp(pValue, "ALARM")==0)
				AlarmStatus.eAlarmStatus = E_AS_ALARM;
			else
			{
				pItem = pItem->NextSiblingElement("Item");
				continue;
			}

			pResp->vTAlarmStatus.push_back(AlarmStatus);
			pItem = pItem->NextSiblingElement("Item");
		}
	}

	return E_EC_OK;
}

//构造请求
int NS_DeviceStatusQueryResp::Build(const void* pData, string& strXml)
{
	TDeviceStatusQueryResp* pResp = (TDeviceStatusQueryResp*)pData;
	
	ostringstream oss;
	//构造头
	oss	<<XML_HEAD
		<<"<Response>\r\n"
		<<XML_CMD_TYPE("DeviceStatus")
		<<XML_SN(pResp->nSn)
		<<XML_DEVICE_ID(pResp->strDeviceId)
		<<XML_ELEMENT1(Result, RESULT_TO_STR(pResp->bResult))
		<<XML_ELEMENT1(Online, (pResp->bOnline?"ONLINE":"OFFLINE"))
		<<XML_ELEMENT1(Status, STATUS_TO_STR(pResp->bStatus));

	if (!pResp->strReason.empty())
		oss<<XML_ELEMENT1(Reason, pResp->strReason);
	//if (pResp->bEncode!=-1)
		oss<<XML_ELEMENT1(Encode, STATUS_TO_STR(pResp->bEncode));
	//if (pResp->bRecord!=-1)
		oss<<XML_ELEMENT1(Record, STATUS_TO_STR(pResp->bRecord));
	if (!pResp->strDeviceTime.empty())
		oss<<XML_ELEMENT1(DeviceTime, pResp->strDeviceTime);
	if (!pResp->vTAlarmStatus.empty())
	{
		oss<<"<Alarmstatus Num=\""<<pResp->vTAlarmStatus.size()<<"\">\r\n";
		for (unsigned int i=0; i<pResp->vTAlarmStatus.size(); i++)
		{	
			oss<< "<Item>\r\n"
				<< XML_ELEMENT2(DeviceID,pResp->vTAlarmStatus[i].strDeviceId);
			switch(pResp->vTAlarmStatus[i].eAlarmStatus)
			{
			case  E_AS_ONDUTY:
				oss<<XML_ELEMENT2(DutyStatus, "ONDUTY");
				break;
			case E_AS_OFFDUTY:
				oss<<XML_ELEMENT2(DutyStatus, "OFFDUTY");
				break;
			case E_AS_ALARM:
				oss<<XML_ELEMENT2(DutyStatus, "ALARM");
				break;
			default:
				break;
			}
			oss << "</Item>\r\n";
		}
		oss<<"</Alarmstatus>\r\n";
	}


	oss<<"</Response>\r\n";
	strXml = oss.str();

	return E_EC_OK;
}

