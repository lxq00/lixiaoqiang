#include "Base/Base.h"
#include "Public.h"
#include "DeviceCatalogQueryResp.h"

//解析
int NS_DeviceCatalogQueryResp::Parse(string& strXml, void* pData)
{
	TDeviceCatalogQueryResp* pResp = (TDeviceCatalogQueryResp*)pData;
	TiXmlDocument xml;
	int re = MsgParse(xml, strXml, pResp->nSn, pResp->strDeviceId, &pResp->vExtendInfo);
	if (re)
		return re;
	pResp->bSubcribe = false;
	do 
	{
		//获取总数
		TiXmlElement* pElement = xml.RootElement()->FirstChildElement("Result");
		if (pElement==NULL)
		{
			break;
		}
		const char* pTemp = pElement->GetText();
		if(pTemp == NULL)
		{
			break;
		}
		pResp->bResult = strcasecmp(pTemp,"OK") == 0;
		pResp->bSubcribe = true;
	} while (0);
	
	//解析设备列表
	int devsumnum = 0;
	do{
		//获取总数
		TiXmlElement* pElement = xml.RootElement()->FirstChildElement("SumNum");
		if (pElement==NULL)
		{
			break;
		}
		const char* pTemp = pElement->GetText();
		if (pTemp==NULL)
		{
			break;
		}
		devsumnum = atoi(pTemp);
	}while(0);
	
	const char* pValue;
	TiXmlElement* pItem = xml.RootElement()->FirstChildElement("EndofFile");
	if (pItem!=NULL && (pValue=pItem->GetText())!=NULL)
		pResp->bEndofFile = atoi(pValue) != 0 ? true : false;
	else
	{
		pResp->bEndofFile = false;
	}

	TiXmlElement* pElement = xml.RootElement()->FirstChildElement("DeviceList");
	int nDeviceNum = 0;
	if(pElement != NULL)
	{
		pElement->QueryIntAttribute("Num", &nDeviceNum);
	}

	pResp->nSumNum = devsumnum == 0 ? nDeviceNum : devsumnum;

	pResp->vDeviceItem.clear();
	if(pElement != NULL)
	{
		pElement = pElement->FirstChildElement("Item");	
	}
	while (pElement!=NULL)
	{
		TDeviceItem oItem;

		pItem = pElement->FirstChildElement("DeviceID");
		if (pItem==NULL || (pValue=pItem->GetText())==NULL)
		{
			//WRITE_ERROR("解析设备列表中的DeviceId出错\n");
			//return E_EC_PARSE_XML_DOC;
			pElement = pElement->NextSiblingElement("Item");
			continue;
		}
		oItem.strDeviceId = pValue;

		pItem = pElement->FirstChildElement("Name");
		if (pItem==NULL || (pValue=pItem->GetText())==NULL)
		{
			//WRITE_ERROR("解析设备列表中的Name出错\n");
			//return E_EC_PARSE_XML_DOC;
			oItem.strName = "";			
		}
		else
			oItem.strName = pValue;

		bool status = false;
		re = MsgParseStatus(pElement->FirstChildElement("Status"), status);
		if (re == E_EC_OK)
			oItem.bStatus = status ? 1 : 0;

		pItem = pElement->FirstChildElement("Manufacturer");
		if (pItem!=NULL)
			oItem.strManufacturer = ((pValue=pItem->GetText())!=NULL)?pValue:"";
		else
			oItem.strManufacturer = "";

		pItem = pElement->FirstChildElement("Model");
		if (pItem!=NULL)
			oItem.strModel = ((pValue=pItem->GetText())!=NULL)?pValue:"";
		else
			oItem.strModel = "";

		
		pItem = pElement->FirstChildElement("Owner");
		if (pItem!=NULL)
			oItem.strOwner = ((pValue=pItem->GetText())!=NULL)?pValue:"";
		else
			oItem.strOwner = "";


		pItem = pElement->FirstChildElement("CivilCode");
		if (pItem!=NULL)
			oItem.strCivilCode = ((pValue=pItem->GetText())!=NULL)?pValue:"";
		else
			oItem.strCivilCode = "";


		pItem = pElement->FirstChildElement("Address");
		if (pItem!=NULL)
			oItem.strAddress = ((pValue=pItem->GetText())!=NULL)?pValue:"";
		else
			oItem.strAddress = "";


		pItem = pElement->FirstChildElement("ParentID");
		if (pItem!=NULL)
			oItem.strParentId = ((pValue=pItem->GetText())!=NULL)?pValue:"";
		else
			oItem.strParentId = "";


		pItem = pElement->FirstChildElement("Parental");
		if (pItem!=NULL)
			oItem.nParental = ((pValue=pItem->GetText())!=NULL)?atoi(pValue):0;
		else
			oItem.nParental = 0;


		pItem = pElement->FirstChildElement("RegisterWay");
		if (pItem!=NULL)
			oItem.nRegisterWay = ((pValue=pItem->GetText())!=NULL)?atoi(pValue):0;
		else
			oItem.nRegisterWay = 0;


		pItem = pElement->FirstChildElement("Secrecy");
		if (pItem!=NULL)
			oItem.nSecrecy = ((pValue=pItem->GetText())!=NULL)?atoi(pValue):0;
		else
			oItem.nSecrecy = 0;


		pItem = pElement->FirstChildElement("Block");
		if (pItem!=NULL)
			oItem.strBlock = ((pValue=pItem->GetText())!=NULL)?pValue:"";
		else
			oItem.strBlock = "";

		pItem = pElement->FirstChildElement("IPAddress");
		if (pItem!=NULL)
			oItem.strIPAddress = ((pValue=pItem->GetText())!=NULL)?pValue:"";
		else
			oItem.strIPAddress = "";

		pItem = pElement->FirstChildElement("Port");
		if (pItem!=NULL)
			oItem.nPort = ((pValue=pItem->GetText())!=NULL)?atoi(pValue):0;
		else
			oItem.nPort = 0;

		pItem = pElement->FirstChildElement("Password");
		if (pItem!=NULL)
			oItem.strPasswd = ((pValue=pItem->GetText())!=NULL)?pValue:"";
		else
			oItem.strPasswd = "";

		pItem = pElement->FirstChildElement("Longitude");
		if (pItem!=NULL)
			oItem.fLongitude = (float)(((pValue=pItem->GetText())!=NULL)?atof(pValue):0);
		else
			oItem.fLongitude = 0;

		pItem = pElement->FirstChildElement("Latitude");
		if (pItem!=NULL)
			oItem.fLatitude = (float)(((pValue=pItem->GetText())!=NULL)?atof(pValue):0);
		else
			oItem.fLatitude = 0;
		
		pItem = pElement->FirstChildElement("Code");
		if(pItem!=NULL)
			oItem.nCode = ((pValue=pItem->GetText())!=NULL)?atoi(pValue):0;
		else
			oItem.nCode = 0;

		pResp->vDeviceItem.push_back(oItem);
		pElement = pElement->NextSiblingElement("Item");
	}
	return E_EC_OK;
}
//构造
int NS_DeviceCatalogQueryResp::Build(const void* pData, string& strXml)
{
	const TDeviceCatalogQueryResp* pResp = (TDeviceCatalogQueryResp*)pData;

	ostringstream oss;
	
	//构造头
	oss	<<XML_HEAD
		<<"<Response>\r\n"
		<<XML_CMD_TYPE("Catalog")
		<<XML_SN(pResp->nSn)
		<<XML_DEVICE_ID(pResp->strDeviceId.c_str());

	if(pResp->bSubcribe)
	{
		oss << XML_ELEMENT1(Result,STATUS_TO_STR(pResp->bResult));
	}
	else
	{
		oss <<XML_ELEMENT1(SumNum,pResp->nSumNum);
		oss <<XML_ELEMENT1(bEndOfFile,pResp->bEndofFile);

		//构造设备列表
		oss << "<DeviceList Num=\"" << pResp->vDeviceItem.size() << "\">\r\n";
		for(unsigned int i=0; i<pResp->vDeviceItem.size(); i++)
		{
			oss	<<"<Item>\r\n"
				<<XML_ELEMENT3(DeviceID, pResp->vDeviceItem[i].strDeviceId.c_str())
				<<XML_ELEMENT3(Name, pResp->vDeviceItem[i].strName.c_str());

			if(pResp->vDeviceItem[i].strManufacturer != "")
				oss <<XML_ELEMENT3(Manufacturer, pResp->vDeviceItem[i].strManufacturer.c_str());

			if(pResp->vDeviceItem[i].strModel != "")
				oss <<XML_ELEMENT3(Model, pResp->vDeviceItem[i].strModel.c_str());

			if(pResp->vDeviceItem[i].strOwner != "")
				oss <<XML_ELEMENT3(Owner, pResp->vDeviceItem[i].strOwner.c_str());

			if(pResp->vDeviceItem[i].strCivilCode != "")
				oss <<XML_ELEMENT3(CivilCode, pResp->vDeviceItem[i].strCivilCode.c_str());

			if(pResp->vDeviceItem[i].strAddress != "")
				oss<<XML_ELEMENT3(Address, pResp->vDeviceItem[i].strAddress.c_str());

			if(pResp->vDeviceItem[i].nParental != -1)
				oss<<XML_ELEMENT3(Parental, pResp->vDeviceItem[i].nParental);

			if(pResp->vDeviceItem[i].strParentId != "")
				oss <<XML_ELEMENT3(ParentID, pResp->vDeviceItem[i].strParentId.c_str());

			if(pResp->vDeviceItem[i].nRegisterWay != -1)
				oss<<XML_ELEMENT3(RegisterWay, pResp->vDeviceItem[i].nRegisterWay);

			if(pResp->vDeviceItem[i].nSecrecy != -1)
				oss<<XML_ELEMENT3(Secrecy, pResp->vDeviceItem[i].nSecrecy);

			//if(pResp->vDeviceItem[i].bStatus != -1)
				oss<<XML_ELEMENT3(Status, STATUS_TO_STR(pResp->vDeviceItem[i].bStatus == 1));

			if(pResp->vDeviceItem[i].strBlock != "")
				oss<<XML_ELEMENT3(Block, pResp->vDeviceItem[i].strBlock.c_str());

			if(pResp->vDeviceItem[i].strIPAddress != "")
				oss<<XML_ELEMENT3(IPAddress, pResp->vDeviceItem[i].strIPAddress.c_str());

			if(pResp->vDeviceItem[i].nPort != -1)
				oss<<XML_ELEMENT3(Port, pResp->vDeviceItem[i].nPort);

			if(pResp->vDeviceItem[i].strPasswd != "")
				oss<<XML_ELEMENT3(Password, pResp->vDeviceItem[i].strPasswd.c_str());

			if(pResp->vDeviceItem[i].fLongitude != -1)
				oss<<XML_ELEMENT3(Longitude, pResp->vDeviceItem[i].fLongitude);

			if(pResp->vDeviceItem[i].fLatitude != -1)
				oss<<XML_ELEMENT3(Latitude, pResp->vDeviceItem[i].fLatitude);
			
			if(pResp->vDeviceItem[i].nCode != -1)
				oss<<XML_ELEMENT3(Code, pResp->vDeviceItem[i].nCode);
			oss <<"</Item>\r\n";
		}
		oss << "</DeviceList>\r\n";

		//扩展信息
		if (!pResp->vExtendInfo.empty())
			MsgBuildExtendInfo(pResp->vExtendInfo, oss);
	}
		

	oss << "</Response>\r\n";
	strXml = oss.str();

	return E_EC_OK;
}

