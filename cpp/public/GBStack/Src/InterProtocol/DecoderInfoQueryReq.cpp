#include "Public.h"
#include "DecoderInfoQueryReq.h"

//解析
int NS_DecoderInfoQueryReq::Parse(string& strXml, void* pData)
{
	TDecoderInfoQueryReq *req = (TDecoderInfoQueryReq*)pData;
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

	element = xml.RootElement()->FirstChildElement("DecoderId");
	if (element==NULL || (text=element->GetText())==NULL)
	{
		WRITE_ERROR("缺少DecoderId标签\n");
		return E_EC_PARSE_XML_DOC;
	}
	req->strDecoderId = text;

	return E_EC_OK;
}

//构造
int NS_DecoderInfoQueryReq::Build(const void* pData, string& strXml)
{
	TDecoderInfoQueryReq* req = (TDecoderInfoQueryReq*)pData;
	ostringstream oss;
	//构造头
	oss	<<XML_HEAD
		<<"<Query>\r\n"
		<<XML_CMD_TYPE("DecoderInfo")
		<<XML_SN(req->nSn)
		<<XML_ELEMENT1(DecoderId, req->strDecoderId)
		<<"</Query>\r\n";

	strXml = oss.str();
	return E_EC_OK;
}
