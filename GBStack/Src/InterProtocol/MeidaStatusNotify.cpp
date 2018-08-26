#include "Public.h"
#include "MeidaStatusNotify.h"

//解析请求
int NS_MeidaStatusNotify::Parse(string& strXml, void* pData)
{
	MediaStatusNotify* pResp = (MediaStatusNotify*)pData;
	TiXmlDocument xml;
	int re = MsgParse(xml, strXml, pResp->nSn, pResp->strDeviceId);
	if (re)
		return re;
	
	//解析设备列表
	TiXmlElement* pElement = xml.RootElement()->FirstChildElement("NotifyType");
	const char* pTemp = pElement->GetText();
	if(pTemp != NULL)
	{
		pResp->nType = atoi(pTemp);
	}
	
	return E_EC_OK;
}

//构造请求
int NS_MeidaStatusNotify::Build(const void* pData, string& strXml)
{
	MediaStatusNotify* pReq = (MediaStatusNotify*)pData;
	
	ostringstream oss;
	//构造头
	oss	<<XML_HEAD
		<<"<Notify>\r\n"
		<<XML_CMD_TYPE("MediaStatus")
		<<XML_SN(pReq->nSn)
		<<XML_DEVICE_ID(pReq->strDeviceId)
		<<XML_ELEMENT1(NotifyType,pReq->nType)
		<<"</Notify>\r\n";
	strXml = oss.str();

	return E_EC_OK;
}

