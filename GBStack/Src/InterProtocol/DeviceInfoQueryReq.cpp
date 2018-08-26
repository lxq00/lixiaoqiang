#include "Public.h"
#include "DeviceInfoQueryReq.h"



//解析请求
int NS_DeviceInfoQueryReq::Parse(string& strXml, void* pData)
{
	TDeviceInfoQueryReq* pResp = (TDeviceInfoQueryReq*)pData;
	TiXmlDocument xml;
	return MsgParse(xml, strXml, pResp->nSn, pResp->strDeviceId);
}

//构造请求
int NS_DeviceInfoQueryReq::Build(const void* pData, string& strXml)
{
	TDeviceInfoQueryReq* pReq = (TDeviceInfoQueryReq*)pData;
	
	ostringstream oss;
	//构造头
	oss	<<XML_HEAD
		<<"<Query>\r\n"
		<<XML_CMD_TYPE("DeviceInfo")
		<<XML_SN(pReq->nSn)
		<<XML_DEVICE_ID(pReq->strDeviceId)
		<<"</Query>\r\n";
	strXml = oss.str();

	return E_EC_OK;
}

