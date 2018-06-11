#include "Public.h"
#include "DeviceControlResp.h"

//解析
int NS_DeviceControlResp::Parse(string& strXml, void* pData)
{
	TDeviceControlResp* pResp = (TDeviceControlResp*)pData;
	TiXmlDocument xml;
	int re = MsgParse(xml, strXml, pResp->nSn, pResp->strDeviceId);
	if (re)
		return re;
	return MsgParseResult(xml.RootElement()->FirstChildElement("Result"), pResp->bResult);
}

//构造
int NS_DeviceControlResp::Build(const void* pData, string& strXml)
{
	const TDeviceControlResp* pResp = (TDeviceControlResp*)pData;

	ostringstream oss;
	
	//构造头
	oss	<<XML_HEAD
		<<"<Response>\r\n"
		<<XML_CMD_TYPE("DeviceControl")
		<<XML_SN(pResp->nSn)
		<<XML_DEVICE_ID(pResp->strDeviceId)
		<<XML_ELEMENT1(Result, RESULT_TO_STR(pResp->bResult))
		<<"</Response>\r\n";
	strXml = oss.str();

	return E_EC_OK;
}

