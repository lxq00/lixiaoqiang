#include "Public.h"
#include "DecoderInfoQueryResp.h"

//解析
int NS_DecoderInfoQueryResp::Parse(string& strXml, void* pData)
{
	TDecoderInfoQueryResp* req = (TDecoderInfoQueryResp*)pData;
	TiXmlDocument xml;

	xml.Parse(strXml.c_str());
	if (xml.Error())
	{
		WRITE_ERROR("不是有效的xml文档\n");
		return E_EC_INVALID_DOC;
	}

	TiXmlElement* element;
	const char* text;

	element = xml.RootElement()->FirstChildElement("SN");
	if (element==NULL || (text=element->GetText())==NULL)
	{
		WRITE_ERROR("缺少SN标签\n");
		return E_EC_PARSE_XML_DOC;
	}
	req->nSn = atoi(text);

	element = xml.RootElement()->FirstChildElement("DeviceId");

	if (element==NULL || (text=element->GetText())==NULL)
		req->strDeviceId = "";
	else
		req->strDeviceId = text;

	element = xml.RootElement()->FirstChildElement("Info");
	if (element==NULL)
		return E_EC_OK;
	element = element->FirstChildElement("Item");
	while (element!=NULL)
	{
		TDecoderInfo info;
		TiXmlElement* subelement = element->FirstChildElement("DecoderId");
		if (subelement==NULL || (text=subelement->GetText())==NULL)
		{
			element = element->NextSiblingElement("Item");
			continue;
		}
		info.strDecoderId = text;
		subelement = element->FirstChildElement("DeviceId");
		if (subelement==NULL || (text=subelement->GetText())==NULL)
		{
			element = element->NextSiblingElement("Item");
			continue;
		}
		info.strDeviceId = text;
		req->vDecoderInfo.push_back(info);
	}
	return E_EC_OK;
}

//构造
int NS_DecoderInfoQueryResp::Build(const void* pData, string& strXml)
{
	TDecoderInfoQueryResp* req = (TDecoderInfoQueryResp*)pData;
	ostringstream oss;
	//构造头
	oss	<<XML_HEAD
		<<"<Response>\r\n"
		<<XML_CMD_TYPE("DecoderInfo")
		<<XML_SN(req->nSn)
		<<XML_ELEMENT1(DeviceId, req->strDeviceId)
		<<"<List>\r\n";
	for (unsigned int i=0; i<req->vDecoderInfo.size(); i++)
	{
		oss << "<Item>\r\n"
			<< XML_ELEMENT3(DecoderId, req->vDecoderInfo[i].strDecoderId)
			<< XML_ELEMENT3(DeviceId, req->vDecoderInfo[i].strDeviceId)
			<< "</Item>\r\n";
	}
	oss <<"</List>\r\n</Response>\r\n";

	strXml = oss.str();
	return E_EC_OK;
}
