#include "Public.h"
#include "MeidaStatusNotify.h"

//��������
int NS_MeidaStatusNotify::Parse(string& strXml, void* pData)
{
	MediaStatusNotify* pResp = (MediaStatusNotify*)pData;
	TiXmlDocument xml;
	int re = MsgParse(xml, strXml, pResp->nSn, pResp->strDeviceId);
	if (re)
		return re;
	
	//�����豸�б�
	TiXmlElement* pElement = xml.RootElement()->FirstChildElement("NotifyType");
	const char* pTemp = pElement->GetText();
	if(pTemp != NULL)
	{
		pResp->nType = atoi(pTemp);
	}
	
	return E_EC_OK;
}

//��������
int NS_MeidaStatusNotify::Build(const void* pData, string& strXml)
{
	MediaStatusNotify* pReq = (MediaStatusNotify*)pData;
	
	ostringstream oss;
	//����ͷ
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

