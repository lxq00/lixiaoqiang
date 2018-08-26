#include "Public.h"
#include "DecoderInfoQueryReq.h"

//����
int NS_DecoderInfoQueryReq::Parse(string& strXml, void* pData)
{
	TDecoderInfoQueryReq *req = (TDecoderInfoQueryReq*)pData;
	TiXmlDocument xml;

	xml.Parse(strXml.c_str());
	if (xml.Error())
	{
		WRITE_ERROR("������Ч��xml�ĵ�\n");
		return E_EC_INVALID_DOC;
	}

	TiXmlElement* element;
	const char* text;

	element = xml.RootElement()->FirstChildElement("SN");
	if (element==NULL || (text=element->GetText())==NULL)
	{
		WRITE_ERROR("ȱ��SN��ǩ\n");
		return E_EC_PARSE_XML_DOC;
	}
	req->nSn = atoi(text);

	element = xml.RootElement()->FirstChildElement("DecoderId");
	if (element==NULL || (text=element->GetText())==NULL)
	{
		WRITE_ERROR("ȱ��DecoderId��ǩ\n");
		return E_EC_PARSE_XML_DOC;
	}
	req->strDecoderId = text;

	return E_EC_OK;
}

//����
int NS_DecoderInfoQueryReq::Build(const void* pData, string& strXml)
{
	TDecoderInfoQueryReq* req = (TDecoderInfoQueryReq*)pData;
	ostringstream oss;
	//����ͷ
	oss	<<XML_HEAD
		<<"<Query>\r\n"
		<<XML_CMD_TYPE("DecoderInfo")
		<<XML_SN(req->nSn)
		<<XML_ELEMENT1(DecoderId, req->strDecoderId)
		<<"</Query>\r\n";

	strXml = oss.str();
	return E_EC_OK;
}
