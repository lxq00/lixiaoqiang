#include "Public.h"
#include "DecoderControlReq.h"

//����
int NS_DecoderControlReq::Parse(string& strXml, void* pData)
{
	TDecoderControlReq *req = (TDecoderControlReq*)pData;
	TiXmlDocument xml;

	xml.Parse(strXml.c_str());
	if (xml.Error())
	{
		WRITE_ERROR("������Ч��xml�ĵ�\n");
		return E_EC_INVALID_DOC;
	}
	TiXmlElement* root = xml.RootElement();

	TiXmlElement* element;
	const char* text;

	element = root->FirstChildElement("SN");
	if (element==NULL || (text=element->GetText())==NULL)
	{
		WRITE_ERROR("ȱ��SN��ǩ\n");
		return E_EC_PARSE_XML_DOC;
	}
	req->nSn = atoi(text);

	element = root->FirstChildElement("DecoderId");
	if (element==NULL || (text=element->GetText())==NULL)
	{
		WRITE_ERROR("ȱ��DecoderId��ǩ\n");
		return E_EC_PARSE_XML_DOC;
	}
	req->strDecoderId = text;

	element = root->FirstChildElement("Opt");
	if (element==NULL || (text=element->GetText())==NULL)
	{
		WRITE_ERROR("ȱ��Opt��ǩ\n");
		return E_EC_PARSE_XML_DOC;
	}
	if (strcmp(text, "Start")==0)
	{
		req->bStart = 1;
	}
	else if (strcmp(text, "Stop")==0)
	{
		req->bStart = 0;
	}
	else
	{
		WRITE_ERROR("��Ч�Ĳ�������\n");
		return E_EC_PARSE_XML_DOC;
	}

	if (req->bStart)
	{
		element = root->FirstChildElement("DeviceId");
		if (element==NULL || (text=element->GetText())==NULL)
		{
			WRITE_ERROR("ȱ��DeviceId��ǩ\n");
			return E_EC_PARSE_XML_DOC;
		}
		req->strDeviceId = text;
	}

	return E_EC_OK;
}

//����
int NS_DecoderControlReq::Build(const void* pData, string& strXml)
{
	TDecoderControlReq* req = (TDecoderControlReq*)pData;
	ostringstream oss;
	//����ͷ
	oss	<<XML_HEAD
		<<"<Control>\r\n"
		<<XML_CMD_TYPE("Decoder")
		<<XML_SN(req->nSn)
		<<XML_ELEMENT1(DecoderId, req->strDecoderId)
		<<XML_ELEMENT1(Opt, req->bStart?"Start":"Stop");
	if (req->bStart)
		oss <<XML_ELEMENT1(DeviceId, req->strDeviceId);
	oss <<"</Control>\r\n";

	strXml = oss.str();
	return E_EC_OK;
}
