#include "Public.h"
#include "DecoderControlReq.h"

//解析
int NS_DecoderControlReq::Parse(string& strXml, void* pData)
{
	TDecoderControlReq *req = (TDecoderControlReq*)pData;
	TiXmlDocument xml;

	xml.Parse(strXml.c_str());
	if (xml.Error())
	{
		WRITE_ERROR("不是有效的xml文档\n");
		return E_EC_INVALID_DOC;
	}
	TiXmlElement* root = xml.RootElement();

	TiXmlElement* element;
	const char* text;

	element = root->FirstChildElement("SN");
	if (element==NULL || (text=element->GetText())==NULL)
	{
		WRITE_ERROR("缺少SN标签\n");
		return E_EC_PARSE_XML_DOC;
	}
	req->nSn = atoi(text);

	element = root->FirstChildElement("DecoderId");
	if (element==NULL || (text=element->GetText())==NULL)
	{
		WRITE_ERROR("缺少DecoderId标签\n");
		return E_EC_PARSE_XML_DOC;
	}
	req->strDecoderId = text;

	element = root->FirstChildElement("Opt");
	if (element==NULL || (text=element->GetText())==NULL)
	{
		WRITE_ERROR("缺少Opt标签\n");
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
		WRITE_ERROR("无效的操作命令\n");
		return E_EC_PARSE_XML_DOC;
	}

	if (req->bStart)
	{
		element = root->FirstChildElement("DeviceId");
		if (element==NULL || (text=element->GetText())==NULL)
		{
			WRITE_ERROR("缺少DeviceId标签\n");
			return E_EC_PARSE_XML_DOC;
		}
		req->strDeviceId = text;
	}

	return E_EC_OK;
}

//构造
int NS_DecoderControlReq::Build(const void* pData, string& strXml)
{
	TDecoderControlReq* req = (TDecoderControlReq*)pData;
	ostringstream oss;
	//构造头
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
