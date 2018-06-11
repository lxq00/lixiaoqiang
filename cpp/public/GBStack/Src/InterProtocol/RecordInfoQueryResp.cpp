#include "Public.h"
#include "RecordInfoQueryResp.h"

//类型表
extern const char* g_pRecordInfoType[];


//解析请求
int NS_RecordInfoQueryResp::Parse(string& strXml, void* pData)
{
	TRecordInfoQueryResp* pResp = (TRecordInfoQueryResp*)pData;
	TiXmlDocument xml;
	int re =  MsgParse(xml, strXml, pResp->nSn, pResp->strDeviceId);
	if (re)
		return re;
	TiXmlElement* pRoot = xml.RootElement();
	TiXmlElement* pItem;
	const char* pValue;

	pItem = pRoot->FirstChildElement("Name");
	if (pItem!=NULL && (pValue=pItem->GetText())!=NULL)
		pResp->strName = pValue;
	else
	{
		//WRITE_ERROR("不是有效的xml文档\n");
		//return E_EC_INVALID_DOC;
		pResp->strName = "";
	}

	pItem = pRoot->FirstChildElement("SumNum");
	if (pItem!=NULL && (pValue=pItem->GetText())!=NULL)
		pResp->nSumNum = atoi(pValue);
	else
	{
		pResp->nSumNum = -1;
	}

	pItem = pRoot->FirstChildElement("EndofFile");
	if (pItem!=NULL && (pValue=pItem->GetText())!=NULL)
		pResp->bEndofFile = atoi(pValue) != 0 ? true : false;
	else
	{
		pResp->bEndofFile = false;
	}


	pItem = pRoot->FirstChildElement("RecordList");
	if (pItem!=NULL)
	{
		TiXmlElement* pSubItem;
		pItem = pItem->FirstChildElement("Item");
		TRecordItem RecordItem;
		while (pItem!=NULL)
		{
			pSubItem = pItem->FirstChildElement("DeviceID");
			if (pSubItem!=NULL && (pValue=pSubItem->GetText())!=NULL)
				RecordItem.strDeviceId = pValue;
			else
			{
				pItem = pItem->NextSiblingElement("Item");
				continue;
			}

			pSubItem = pItem->FirstChildElement("Name");
			if (pSubItem!=NULL && (pValue=pSubItem->GetText())!=NULL)
				RecordItem.strName = pValue;
			else
			{
				RecordItem.strName = "";
			}

			pSubItem = pItem->FirstChildElement("FilePath");
			if (pSubItem!=NULL && (pValue=pSubItem->GetText())!=NULL)
				RecordItem.strFilePath = pValue;
			else
				RecordItem.strFilePath = "";

			pSubItem = pItem->FirstChildElement("Address");
			if (pSubItem!=NULL && (pValue=pSubItem->GetText())!=NULL)
				RecordItem.strAddress = pValue;
			else
				RecordItem.strAddress = "";

			pSubItem = pItem->FirstChildElement("StartTime");
			if (pSubItem!=NULL && (pValue=pSubItem->GetText())!=NULL)
				RecordItem.strStartTime = pValue;
			else
				RecordItem.strStartTime = "";

			pSubItem = pItem->FirstChildElement("EndTime");
			if (pSubItem!=NULL && (pValue=pSubItem->GetText())!=NULL)
				RecordItem.strEndTime = pValue;
			else
				RecordItem.strEndTime = "";


			pSubItem = pItem->FirstChildElement("Secrecy");
			if (pSubItem!=NULL && (pValue=pSubItem->GetText())!=NULL)
				RecordItem.nSecrecy = atoi(pValue);
			else
				RecordItem.nSecrecy = 0;

			pSubItem = pItem->FirstChildElement("FileSize");
			if (pSubItem!=NULL && (pValue=pSubItem->GetText())!=NULL)
			{
				sscanf(pValue,"%llu",&RecordItem.nFileSize);
			}
			else
				RecordItem.nFileSize = 0;

			pSubItem = pItem->FirstChildElement("Type");
			if (pSubItem!=NULL && (pValue=pSubItem->GetText())!=NULL)
			{
				RecordItem.eType = E_FQT_NONE;
				for(int i=E_FQT_ALL; i<=E_FQT_MANUAL; i++)
				{
					if (strcmp(g_pRecordInfoType[i], pValue)==0)
					{
						RecordItem.eType = (ERecordInfoType)i;
						break;
					}
				}
			}
			else
				RecordItem.eType = E_FQT_NONE;

			pSubItem = pItem->FirstChildElement("RecorderID");
			if (pSubItem!=NULL && (pValue=pSubItem->GetText())!=NULL)
				RecordItem.strRecorderID = pValue;
			else
				RecordItem.strRecorderID = "";

			pResp->vRecordList.push_back(RecordItem);
			pItem = pItem->NextSiblingElement("Item");

		}
	
	}

	return E_EC_OK;
}

//构造请求
int NS_RecordInfoQueryResp::Build(const void* pData, string& strXml)
{
	TRecordInfoQueryResp* pResp = (TRecordInfoQueryResp*)pData;
	
	ostringstream oss;
	//构造头
	oss	<<XML_HEAD
		<<"<Response>\r\n"
		<<XML_CMD_TYPE("RecordInfo")
		<<XML_SN(pResp->nSn)
		<<XML_DEVICE_ID(pResp->strDeviceId)
		<<XML_ELEMENT1(Name, pResp->strName)
		<<XML_ELEMENT1(SumNum, pResp->nSumNum)
		<<XML_ELEMENT1(EndofFile,pResp->bEndofFile);
	if (!pResp->vRecordList.empty())
	{
		oss<<"<RecordList Num=\"" << pResp->vRecordList.size() << "\">\r\n";
		for (unsigned int i=0; i<pResp->vRecordList.size(); i++)
		{
			char filesizestr[64];
			sprintf(filesizestr,"%llu",pResp->vRecordList[i].nFileSize);

			oss<<"<Item>\r\n"<<XML_ELEMENT2(DeviceID, pResp->vRecordList[i].strDeviceId)
				<<XML_ELEMENT2(Name, pResp->vRecordList[i].strName);
			if (!pResp->vRecordList[i].strFilePath.empty())
				oss<<XML_ELEMENT2(FilePath, pResp->vRecordList[i].strFilePath);
			if (!pResp->vRecordList[i].strAddress.empty())
				oss<<XML_ELEMENT2(Address, pResp->vRecordList[i].strAddress);
			if (!pResp->vRecordList[i].strStartTime.empty())
				oss<<XML_ELEMENT2(StartTime, pResp->vRecordList[i].strStartTime);
			if (!pResp->vRecordList[i].strEndTime.empty())
				oss<<XML_ELEMENT2(EndTime, pResp->vRecordList[i].strEndTime);
		//	if (pResp->vRecordList[i].nSecrecy)
				oss<<XML_ELEMENT2(Secrecy, pResp->vRecordList[i].nSecrecy);
				oss<<XML_ELEMENT2(FileSize, filesizestr);
			if (pResp->vRecordList[i].eType>E_FQT_NONE && pResp->vRecordList[i].eType<=E_FQT_MANUAL)
				oss<<XML_ELEMENT1(Type, g_pRecordInfoType[pResp->vRecordList[i].eType]);
			if (!pResp->vRecordList[i].strRecorderID.empty())
				oss<<XML_ELEMENT1(RecorderID, pResp->vRecordList[i].strRecorderID);
			oss<<"</Item>\r\n";
		}
		oss<<"</RecordList>\r\n";
	}

	oss<<"</Response>\r\n";
	strXml = oss.str();

	return E_EC_OK;
}

