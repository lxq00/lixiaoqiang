#include "Public.h"

//����������Ϣ
int MsgParse(TiXmlDocument& xml, string& strXml,int &nSn, string& strDeviceId)
{
	xml.Parse(strXml.c_str(), 0, TIXML_ENCODING_LEGACY);
	if (xml.Error())
	{
	//	WRITE_ERROR("������Ч��xml�ĵ�\n");
	//	return E_EC_INVALID_DOC;
	}
	//��ȡSN
	TiXmlElement* pElement = xml.RootElement()->FirstChildElement("SN");
	if (pElement==NULL)
	{
		WRITE_ERROR("ȱ��SN��ǩ\n");
		return E_EC_PARSE_XML_DOC;
	}
	const char* pTemp = pElement->GetText();
	if (pTemp==NULL)
	{
		WRITE_ERROR("��ȡSN���ݳ���\n");
		return E_EC_PARSE_XML_DOC;
	}
	nSn = atoi(pTemp);

	//��ȡ�豸id
	pElement = xml.RootElement()->FirstChildElement("DeviceID");
	if (pElement==NULL)
	{
		WRITE_ERROR("ȱ��DeviceID��ǩ\n");
		return E_EC_PARSE_XML_DOC;
	}
	pTemp = pElement->GetText();
	if (pTemp==NULL)
	{
		WRITE_ERROR("��ȡDeviceID���ݳ���\n");
		return E_EC_PARSE_XML_DOC;
	}
	strDeviceId = pTemp;

	return E_EC_OK;
}


//������չ��Ϣ
void MsgBuildExtendInfo(const vector<string>& vExtendInfo, ostringstream& oss)
{
	assert(!vExtendInfo.empty());

	for(unsigned int i=0; i<vExtendInfo.size(); i++)
	{
		oss<<XML_ELEMENT1(Info, vExtendInfo[i]);
	}
}

//����������Ϣ
int MsgParse(TiXmlDocument& xml, string& strXml,int &nSn, string& strDeviceId, vector<string>* pExtendInfo)
{
	int ret = MsgParse(xml,strXml,nSn,strDeviceId);
	if(ret != E_EC_OK)
	{
		return ret;
	}
	
	xml.Parse(strXml.c_str(),0, TIXML_ENCODING_LEGACY);
	if (xml.Error())
	{
	//	WRITE_ERROR("������Ч��xml�ĵ�\n");
	//	return E_EC_INVALID_DOC;
	}

	//������չ��Ϣ
	if (pExtendInfo!=NULL)
	{
		pExtendInfo->clear();
		TiXmlElement* pElement = xml.RootElement()->FirstChildElement("Info");
		while(pElement!=NULL)
		{
			const char* pTemp = pElement->GetText();
			pExtendInfo->push_back(pTemp==NULL?"":pTemp);
			pElement = pElement->NextSiblingElement("Info");
		}
	}

	return E_EC_OK;
}
//������չ��Ϣ
void MsgBuildExtendInfo(const map<std::string,std::string>& vExtendInfo, ostringstream& oss)
{
	assert(!vExtendInfo.empty());

	if(vExtendInfo.size() > 0)
	{
		oss<< "<Info>\r\n";
	}

	map<std::string,std::string>::const_iterator iter;
	for(iter = vExtendInfo.begin();iter != vExtendInfo.end();iter ++)
	{
		oss<<"<"<<iter->first.c_str()<<">"<<iter->second.c_str()<<"</"<<iter->first.c_str()<<">\r\n";
	}
	
	if(vExtendInfo.size() > 0)
	{
		oss<< "</Info>\r\n";
	}
}

//����������Ϣ
int MsgParse(TiXmlDocument& xml, string& strXml,int &nSn, string& strDeviceId, std::map<string,string>* pExtendInfo)
{
	int ret = MsgParse(xml,strXml,nSn,strDeviceId);
	if(ret != E_EC_OK)
	{
		return ret;
	}
	
	xml.Parse(strXml.c_str());
	if (xml.Error())
	{
	//	WRITE_ERROR("������Ч��xml�ĵ�\n");
	//	return E_EC_INVALID_DOC;
	}
	//������չ��Ϣ
	if (pExtendInfo!=NULL)
	{
		pExtendInfo->clear();
		TiXmlElement* pElement = xml.RootElement()->FirstChildElement("Info");
		if(pElement != NULL)
		{
			TiXmlElement* chileElement = pElement->FirstChildElement();
			
			while(chileElement!=NULL)
			{
				const char* pTemp = chileElement->GetText();
				if(pTemp != NULL)
				{
					std::string value = chileElement->Value() == NULL ? "" : chileElement->Value();
					pExtendInfo->insert(pair<std::string,std::string>(pTemp,value));
				}
				
				chileElement = chileElement->NextSiblingElement();
			}
		}
		
	}

	return E_EC_OK;
}

//�������
int MsgParseResult(TiXmlElement* pElement, bool& bResult)
{
	if (pElement==NULL)
	{
		bResult = false;
	//	WRITE_ERROR("�޽����Ϣ\n");
		return E_EC_PARSE_XML_DOC;
	}

	const char* pTemp = pElement->GetText();
	if (pTemp==NULL)
	{
		bResult = false;
	//	WRITE_ERROR("�޽��ֵ��Ϣ\n");
		return E_EC_PARSE_XML_DOC;
	}

	//string strResult(pTemp);
/*	if (strResult=="OK" || strResult=="ON")
		bResult = 1;
	else if(strResult=="ERROR" || strResult=="OFF")
		bResult = 0;*/

#ifdef WIN32
	if ((_stricmp("OK", pTemp) == 0) || (_stricmp("ON", pTemp)==0) || (_stricmp("ONLINE", pTemp)==0))
		bResult = 1;
	else if ((_stricmp("ERROR", pTemp) == 0) || (_stricmp("OFF", pTemp)==0) || (_stricmp("OFFLINE", pTemp)==0))
		bResult = 0;
#else
	if ((strcasecmp("OK", pTemp) == 0) || (strcasecmp("ON", pTemp)==0) || (strcasecmp("ONLINE", pTemp)==0))
		bResult = 1;
	else if ((strcasecmp("ERROR", pTemp) == 0) || (strcasecmp("OFF", pTemp)==0) || (strcasecmp("OFFLINE", pTemp)==0))
		bResult = 0;
#endif
	else
	{
		bResult = false;
	//	WRITE_ERROR("��Ч�Ľ��ֵ��Ϣ\n");
		return E_EC_PARSE_XML_DOC;
	}
	return E_EC_OK;
}

//ʮ�������ַ�ת��������
	char HexChar2Num(char cHex)
	{
		if(cHex>='0' && cHex<='9')
			return cHex-'0';
		else if(cHex>='a' && cHex<='f')
			return cHex-'a'+10;
		else if(cHex>='A' && cHex<='F')
			return cHex-'A'+10;
		else
			return -1;
	}
	//����ת����ʮ�������ַ�
char Num2HexChar(char cNum)
{		if (cNum>=0 && cNum<=9)
			return cNum+'0';
		else if(cNum>=10 && cNum<=15)
			return cNum-10+'A';
		else
			return -1;
}
	
