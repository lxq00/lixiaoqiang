#include "Public.h"
#include "DeviceInfoQueryResp.h"



//解析请求
int NS_DeviceInfoQueryResp::Parse(string& strXml, void* pData)
{
	TDeviceInfoQueryResp* pResp = (TDeviceInfoQueryResp*)pData;
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

	
	/*pItem = pRoot->FirstChildElement("DeviceType");
	if (pRoot!=NULL && (pValue=pItem->GetText())!=NULL)
		pResp->strDeviceType = pValue;
	else
		pResp->strDeviceType = "";*/
	
	pItem = pRoot->FirstChildElement("Manufacturer");
	if (pItem!=NULL && (pValue=pItem->GetText())!=NULL)
		pResp->strManufacturer = pValue;
	else
		pResp->strManufacturer = "";
	
	pItem = pRoot->FirstChildElement("Model");
	if (pItem!=NULL && (pValue=pItem->GetText())!=NULL)
		pResp->strModel = pValue;
	else
		pResp->strModel = "";
	
	pItem = pRoot->FirstChildElement("Firmware");
	if (pItem!=NULL && (pValue=pItem->GetText())!=NULL)
		pResp->strFirmware = pValue;
	else
		pResp->strFirmware = "";
	
	pItem = pRoot->FirstChildElement("MaxCamera");
	if (pItem!=NULL && (pValue=pItem->GetText())!=NULL)
		pResp->nMaxCamera = atoi(pValue);
	else
		pResp->nMaxCamera = -1;
	
	pItem = pRoot->FirstChildElement("MaxAlarm");
	if (pItem!=NULL && (pValue=pItem->GetText())!=NULL)
		pResp->nMaxAlarm = atoi(pValue);
	else
		pResp->nMaxAlarm = -1;

	return E_EC_OK;
}

//构造请求
int NS_DeviceInfoQueryResp::Build(const void* pData, string& strXml)
{
	TDeviceInfoQueryResp* pResp = (TDeviceInfoQueryResp*)pData;
	
	ostringstream oss;
	//构造头
	oss	<<XML_HEAD
		<<"<Response>\r\n"
		<<XML_CMD_TYPE("DeviceInfo")
		<<XML_SN(pResp->nSn)
		<<XML_DEVICE_ID(pResp->strDeviceId)
		<<XML_ELEMENT1(Result, RESULT_TO_STR(pResp->bResult));
//	if (!pResp->strDeviceType.empty())
//		oss<<XML_ELEMENT1(DeviceType, pResp->strDeviceType);
	if (!pResp->strManufacturer.empty())
		oss<<XML_ELEMENT1(Manufacturer, pResp->strManufacturer);
	if (!pResp->strModel.empty())
		oss<<XML_ELEMENT1(Model, pResp->strModel);
	if (!pResp->strFirmware.empty())
		oss<<XML_ELEMENT1(Firmware, pResp->strFirmware);
	if (pResp->nMaxCamera!=-1)
		oss<<XML_ELEMENT1(MaxCamera, pResp->nMaxCamera);
	if (pResp->nMaxAlarm!=-1)
		oss<<XML_ELEMENT1(MaxAlarm, pResp->nMaxAlarm);
		

	oss<<"</Response>\r\n";
	strXml = oss.str();

	return E_EC_OK;
}

