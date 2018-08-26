#include "Public.h"
#include "DeviceCatalogQueryReq.h"



//��������
int NS_DeviceCatalogQueryReq::Parse(string& strXml, void* pData)
{
	TDeviceCatalogQueryReq* pResp = (TDeviceCatalogQueryReq*)pData;
	TiXmlDocument xml;
	int ret = MsgParse(xml, strXml, pResp->nSn, pResp->strDeviceId);

	TiXmlElement* pRoot = xml.RootElement();
	const char* pValue;
	string strTemp;

	TiXmlElement* pItem = pRoot->FirstChildElement("StartTime");
	if (pItem!=NULL && (pValue=pItem->GetText())!=NULL)
		pResp->strStartTime = pItem->GetText();

	pItem = pRoot->FirstChildElement("EndTime");
	if (pItem!=NULL && (pValue=pItem->GetText())!=NULL)
		pResp->strEndTime = pItem->GetText();
	
	pResp->bSubcribe = (pResp->strStartTime != "" && pResp->strEndTime != "");

	return ret;
}

//��������
int NS_DeviceCatalogQueryReq::Build(const void* pData, string& strXml)
{
	TDeviceCatalogQueryReq* pReq = (TDeviceCatalogQueryReq*)pData;
	
	ostringstream oss;
	//����ͷ
	oss	<<XML_HEAD
		<<"<Query>\r\n"
		<<XML_CMD_TYPE("Catalog")
		<<XML_SN(pReq->nSn)
		<<XML_DEVICE_ID(pReq->strDeviceId);
		
	if(pReq->bSubcribe)
	{
		oss << XML_ELEMENT1(StartTime,pReq->strStartTime) 
			<< XML_ELEMENT1(EndTime,pReq->strEndTime);
	}
	oss << "</Query>\r\n";
	strXml = oss.str();

	return E_EC_OK;
}

