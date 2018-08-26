#include "Public.h"
#include "AlarmNotifyResp.h"

//����
int NS_AlarmNotifyResp::Parse(string& strXml, void* pData)
{
	TAlarmNotifyResp* pResp = (TAlarmNotifyResp*)pData;
	TiXmlDocument xml;
	int re = MsgParse(xml, strXml, pResp->nSn, pResp->strDeviceId);
	if (re)
		return re;
	return MsgParseResult(xml.RootElement()->FirstChildElement("Result"), pResp->bResult);
}

//����
int NS_AlarmNotifyResp::Build(const void* pData, string& strXml)
{
	const TAlarmNotifyResp* pResp = (TAlarmNotifyResp*)pData;

	ostringstream oss;
	
	//����ͷ
	oss	<<XML_HEAD
		<<"<Response>\r\n"
		<<XML_CMD_TYPE("Alarm")
		<<XML_SN(pResp->nSn)
		<<XML_DEVICE_ID(pResp->strDeviceId)
		<<XML_ELEMENT1(Result, RESULT_TO_STR(pResp->bResult))
		<<"</Response>\r\n";
	strXml = oss.str();

	return E_EC_OK;
}

