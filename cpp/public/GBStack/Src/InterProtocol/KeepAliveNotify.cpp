#include "Public.h"
#include "KeepAliveNotify.h"

//解析请求
int NS_KeepAliveNotify::Parse(string& strXml, void* pData)
{
	TKeepAliveNotify* pParam = (TKeepAliveNotify*)pData;
	TiXmlDocument xml;
	int re =  MsgParse(xml, strXml, pParam->nSn, pParam->strDeviceId);
	if (re)
		return re;

	return MsgParseStatus(xml.RootElement()->FirstChildElement("Status"),
		pParam->bStatus);
}

//构造请求
int NS_KeepAliveNotify::Build(const void* pData, string& strXml)
{
	TKeepAliveNotify* pParam = (TKeepAliveNotify*)pData;
	
	ostringstream oss;
	//构造头
	oss	<<XML_HEAD
		<<"<Notify>\r\n"
		<<XML_CMD_TYPE("Keepalive")
		<<XML_SN(pParam->nSn)
		<<XML_DEVICE_ID(pParam->strDeviceId)
		<<XML_ELEMENT1(Status, RESULT_TO_STR(pParam->bStatus))
		<<"</Notify>\r\n";
	strXml = oss.str();

	return E_EC_OK;
}

