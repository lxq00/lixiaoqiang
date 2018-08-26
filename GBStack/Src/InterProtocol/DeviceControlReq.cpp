#include "Public.h"
#include "DeviceControlReq.h"

bool NS_DeviceControlReq::tranPtzInfo(const char* in, char* out)
{
	if (strlen(in)!=16)
	{
		WRITE_ERROR("控制码的长度不是16, %s", in);
		return false;
	}
	if (memcmp(in, "A50F", 4)!=0)
	{
		WRITE_ERROR("控制码前四位不是A50f, %s", in);
		return false;
	}

	char h,l;
	for (int i=0; i<8; i++)
	{
		h = HexChar2Num(in[2*i]);
		l = HexChar2Num(in[2*i+1]);
		if (h==-1 || l==-1)
		{
			WRITE_ERROR("控制码中出现非数字字符, %s", in);
			return false;
		}
		out[i] = (h<<4)|l;
	}

	//看检验码对不对
	char check = 0;
	for (int i=0; i<7; i++)
		check += out[i];

	if (check!=out[7])
	{
		WRITE_ERROR("控制码中校验码不正确, %s", in);
		return false;
	}
	return true;
}

void NS_DeviceControlReq::BuildFi(int ptz, int p, ostringstream& oss)
{
	char sTempPtz[17];//8*2+1

	//固定信息 A5H
	sTempPtz[0] = 'A';
	sTempPtz[1] = '5';

	//版本信息
	sTempPtz[2] = '0';
	sTempPtz[3] = 'F'; //0+5+A

	//地址的低4位
	sTempPtz[4] = '0';
	sTempPtz[5] = '0';

	//指令位
	sTempPtz[6] = '4';

	int tmpPtz7 = 0;
	if (ptz==PtzII)
		//sTempPtz[7] = Num2HexChar(4);
		tmpPtz7 = 4;	
	else if(ptz==PtzIO)
		//sTempPtz[7] = Num2HexChar(8);
		tmpPtz7 = 8;	
	else if(ptz==PtzFI)
		//sTempPtz[7] = Num2HexChar(1);
		tmpPtz7 = 1;	
	else if(ptz==PtzFO)
		//sTempPtz[7] = Num2HexChar(2);
		tmpPtz7 = 2;	

	sTempPtz[7] = Num2HexChar(tmpPtz7);

	if (tmpPtz7&3)
	{
		sTempPtz[8] = Num2HexChar(p>>4);
		sTempPtz[9] = Num2HexChar(p&0xF);
	}
	else
	{
		sTempPtz[8] = '0';
		sTempPtz[9] = '0';
	}

	if (tmpPtz7&12)
	{
		sTempPtz[10] = Num2HexChar(p>>4);
		sTempPtz[11] = Num2HexChar(p&0xF);
	}
	else
	{
		sTempPtz[10] = '0';
		sTempPtz[11] = '0';
	}

	//变倍速度、
	sTempPtz[12] = '0';
	//地址高四位
	sTempPtz[13] = Num2HexChar(0>>8);


	//校验码
	int nCheck = 0;
	for(int i=0; i<7; i++)
		nCheck += HexChar2Num(sTempPtz[2*i])<<4 | HexChar2Num(sTempPtz[2*i+1]);
	sTempPtz[14] = Num2HexChar((nCheck>>4)&0xf);
	sTempPtz[15] = Num2HexChar(nCheck&0xF);

	sTempPtz[16] = '\0';

	oss<<XML_ELEMENT1(PTZCmd, sTempPtz);
}

bool NS_DeviceControlReq::ParseFi(char* info, int& ptz, int& speed)
{
	if ((info[3]&0xF0)!=0x40)
		return false;

	int temp = info[3]&0xf;
	if (temp==4)
	{
		speed = info[5];
		ptz = PtzII;
		return true;
	}
	else if (temp==8)
	{
		speed = info[5];
		ptz = PtzIO;
		return true;
	}
	else if (temp==1)
	{
		speed = info[4];
		ptz = PtzFI;
		return true;
	}
	else if (temp==2)
	{
		speed = info[4];
		ptz = PtzFO;
		return true;
	}
	else
	{
		return false;
	}
}

void NS_DeviceControlReq::BuildY(int ptz, int p, ostringstream& oss)
{
	char sTempPtz[17];//8*2+1

	//固定信息 A5H
	sTempPtz[0] = 'A';
	sTempPtz[1] = '5';

	//版本信息
	sTempPtz[2] = '0';
	sTempPtz[3] = 'F'; //0+5+A

	//地址的低4位
	sTempPtz[4] = '0';
	sTempPtz[5] = '0';

	//指令位
	sTempPtz[6] = '8';

	if (ptz==PtzYS)
		sTempPtz[7] = '1';
	else if (ptz==PtzYC)
		sTempPtz[7] = '2';
	else if (ptz==PtzYD)
		sTempPtz[7] = '3';


	//水平速度
	sTempPtz[8] ='0';
	sTempPtz[9] = '0';

	//垂直速度
	sTempPtz[10] = Num2HexChar(p>>4);
	sTempPtz[11] = Num2HexChar(p&0xF);
	
	
	//变倍速度、
	sTempPtz[12] = '0';
	//地址高四位
	sTempPtz[13] = '0';


	//校验码
	int nCheck = 0;
	for(int i=0; i<7; i++)
		nCheck += HexChar2Num(sTempPtz[2*i])<<4 | HexChar2Num(sTempPtz[2*i+1]);
	sTempPtz[14] = Num2HexChar((nCheck>>4)&0xf);
	sTempPtz[15] = Num2HexChar(nCheck&0xF);

	sTempPtz[16] = '\0';

	oss<<XML_ELEMENT1(PTZCmd, sTempPtz);
}

bool NS_DeviceControlReq::ParseY(char* info, int& ptz, int& pos)
{
	int re;
	
	if (info[3]&0x81)
		re = PtzYS;
	else if (info[3]&0x82)
		re = PtzYC;
	else if (info[3]&0x83)
		re = PtzYD;
	else
		re = -1;

	if (re!=-1)
	{
		ptz = re;
		pos = info[5];
		return true;
	}
	else
	{
		return false;
	}
}

void NS_DeviceControlReq::BuildPtz(int ptz, int p, ostringstream& oss)
{
	char sTempPtz[17];//8*2+1

	//固定信息 A5H
	sTempPtz[0] = 'A';
	sTempPtz[1] = '5';

	//版本信息
	sTempPtz[2] = '0';
	sTempPtz[3] = 'F'; //0+5+A

	//地址的低4位
	sTempPtz[4] = '0';
	sTempPtz[5] = '0';

	//指令位

	int tmpPtz6 = 0;
	int tmpPtz7 = 0;
	if (ptz==PtzUp)
		//sTempPtz[7] = Num2HexChar(8);
		tmpPtz7 = 8;
	else if(ptz==PtzDwon)
		//sTempPtz[7] = Num2HexChar(4);
		tmpPtz7 = 4;
	else if(ptz==PtzLeft)
		//sTempPtz[7] = Num2HexChar(2);
		tmpPtz7 = 2;
	else if(ptz==PtzRight)
		//sTempPtz[7] = Num2HexChar(1);
		tmpPtz7 = 1;
	else if(ptz==PtzLU)
		//sTempPtz[7] = Num2HexChar(10);
		tmpPtz7 = 10;
	else if(ptz==PtzLD)
		//sTempPtz[7] = Num2HexChar(6);
		tmpPtz7 = 6;
	else if(ptz==PtzRU)
		//sTempPtz[7] = Num2HexChar(9);
		tmpPtz7 = 9;
	else if(ptz==PtzRD)
		//sTempPtz[7] = Num2HexChar(5);
		tmpPtz7 = 5;
	else if (ptz==PtzZI)
		//sTempPtz[6] = Num2HexChar(1);
		tmpPtz6 = 1;
	else if (ptz==PtzZO)
		//sTempPtz[6] = Num2HexChar(2);
		tmpPtz6 = 2;
	else if(ptz==PtzAuto)
		tmpPtz7 = 15;

	sTempPtz[6] = Num2HexChar(tmpPtz6);
	sTempPtz[7] = Num2HexChar(tmpPtz7);
	
	//水平速度
	if (tmpPtz7&3)
	{
		sTempPtz[8] = Num2HexChar(p>>4);
		sTempPtz[9] = Num2HexChar(p&0xF);
	}
	else
	{
		sTempPtz[8] = '0';
		sTempPtz[9] = '0';
	}

	//垂直速度
	if (tmpPtz7&12)
	{
		sTempPtz[10] = Num2HexChar(p>>4);
		sTempPtz[11] = Num2HexChar(p&0xF);
	}
	else
	{
		sTempPtz[10] = '0';
		sTempPtz[11] = '0';
	}

	if (tmpPtz6)
	{
		p = p*15/255;
	
		//变倍速度、
		sTempPtz[12] = Num2HexChar(p&0xF);
		//地址高四位
		sTempPtz[13] = '0';
	}
	else
	{
		sTempPtz[12] = '0';
		sTempPtz[13] = '0';
	}

	//校验码
	int nCheck = 0;
	for(int i=0; i<7; i++)
		nCheck += HexChar2Num(sTempPtz[2*i])<<4 | HexChar2Num(sTempPtz[2*i+1]);
	sTempPtz[14] = Num2HexChar((nCheck>>4)&0xf);
	sTempPtz[15] = Num2HexChar(nCheck&0xF);

	sTempPtz[16] = '\0';

	oss<<XML_ELEMENT1(PTZCmd, sTempPtz);

}

bool NS_DeviceControlReq::ParsePtz(char* info, int& ptz, int &speed)
{
	if (info[3]&0xC0)
		return false;

	if (info[3]&0x20)
	{
		speed = ((int)info[6])>>4;
		ptz = PtzZO;
		return true;
	}
	if (info[3]&0x10)
	{
		speed = ((int)info[6])>>4;
		ptz = PtzZI;
		return true;
	}
	if(info[3]&8 && info[3]&4 && info[3]&2 && info[3]&1)
	{
		ptz = PtzAuto;
		speed = info[4] == 0 ? info[5] : info[4];
		return true;
	}
	else if (info[3]&8) //up
	{
		if (info[3]&2)//left
		{
			ptz = PtzLU;
			speed = info[4] == 0 ? info[5] : info[4];
	
			return true;
		}
		else if (info[3]&1) //right
		{
			ptz = PtzRU;
			speed = info[4] == 0 ? info[5] : info[4];
	
			return true;
		}	
		else 
		{
			ptz = PtzUp;
			speed = info[5];
	
			return true;
		}
	}
	else if (info[3]&4)
	{
		if (info[3]&2)//left
		{
			ptz = PtzLD;
			speed = info[4] == 0 ? info[5] : info[4];
			
			return true;
		}
		else if (info[3]&1) //right
		{
			ptz = PtzRD;
			speed = info[4] == 0 ? info[5] : info[4];
			
			return true;
		}
		else 
		{
			ptz = PtzDwon;
			speed = info[5];
			
			return true;
		}
	}
	else if (info[3]&2)//left
	{
		ptz = PtzLeft;
		speed = info[4];
		
		return true;
	}
	else if (info[3]&1) //right
	{
		ptz = PtzRight;
		speed = info[4];
		
		return true;
	}
	return false;
}

//解析
int NS_DeviceControlReq::Parse(string& strXml, void* pData)
{
	TDeviceControlReq* pReq = (TDeviceControlReq*)pData;
	TiXmlDocument xml;
	int re = MsgParse(xml, strXml, pReq->nSn, pReq->strDeviceId, &pReq->vExtendInfo);
	if (re)
		return re;

	TiXmlElement* pRoot = xml.RootElement();
	const char* pTemp;
	string strTemp;
	//解析ptz 
	TiXmlElement* pElement = pRoot->FirstChildElement("PTZCmd");
#if 0
	if (pElement==NULL)
		pReq->oPtzCmd.bControl = 0;
	else
	{
		re = ParsePtzInfo(pElement->GetText(), pReq->oPtzCmd);
		if (re)
			return re;
	}
#else
	if (pElement==NULL)
		pReq->ptz = -1;
	else
	{
		char out[8];
		if (!tranPtzInfo(pElement->GetText(), out))
			return E_EC_PARSE_XML_DOC;

		if (!ParsePtz(out, pReq->ptz, pReq->ptzParam) &&
			!ParseFi(out, pReq->ptz, pReq->ptzParam)&&
			!ParseY(out, pReq->ptz, pReq->ptzParam))
			pReq->ptz = PtzStop;
	}
#endif
	//解析远程控制
	pElement = pRoot->FirstChildElement("TeleBoot");
	if (pElement==NULL)
		pReq->bTeleBoot = false;
	else
	{
		pTemp = pElement->GetText();
		if (pTemp==NULL)
		{
			WRITE_ERROR("无远程启动报文\n");
			return E_EC_PARSE_XML_DOC;
		}
		if (string(pTemp)!="Boot")
		{
			WRITE_ERROR("无效的远程启动报文\n");
			return E_EC_PARSE_XML_DOC;
		}
		pReq->bTeleBoot = true;
	}

	//解析录像信息
	pElement = pRoot->FirstChildElement("RecordCmd");
	if (pElement==NULL)
		pReq->eRecordCmd = E_RC_NONE;
	else
	{
		pTemp = pElement->GetText();
		if (pTemp==NULL)
		{
			WRITE_ERROR("无录像报文\n");
			return E_EC_PARSE_XML_DOC;
		}
		strTemp = pTemp;
		if (strTemp=="Record")
			pReq->eRecordCmd = E_RC_RECORD;
		else if (strTemp=="StopRecord")
			pReq->eRecordCmd = E_RC_STOP_RECORD;
		else
		{
			WRITE_ERROR("无效的录像报文\n");
			return E_EC_PARSE_XML_DOC;
		}
	}

	//解布防撤防信息
	pElement = pRoot->FirstChildElement("GuardCmd");
	if (pElement==NULL)
		pReq->eGuardCmd= E_GC_NONE;
	else
	{
		pTemp = pElement->GetText();
		if (pTemp==NULL)
		{
			WRITE_ERROR("无布防撤防报文\n");
			return E_EC_PARSE_XML_DOC;
		}
		strTemp = pTemp;
		if (strTemp=="SetGuard")
			pReq->eGuardCmd = E_GC_SET_GUARD;
		else if (strTemp=="ResetGuard")
			pReq->eGuardCmd = E_GC_REST_GUARD;
		else
		{
			WRITE_ERROR("无效的布防撤防报文\n");
			return E_EC_PARSE_XML_DOC;
		}
	}	

	//解析报警复位
	pElement = pRoot->FirstChildElement("AlarmCmd");
	if (pElement==NULL)
		pReq->bAlarmCmd= false;
	else
	{
		pTemp = pElement->GetText();
		if (pTemp==NULL)
		{
			WRITE_ERROR("无报警复位报文\n");
			return E_EC_PARSE_XML_DOC;
		}
		if (string(pTemp)!="ResetAlarm")
		{
			WRITE_ERROR("无效的报警复位报文\n");
			return E_EC_PARSE_XML_DOC;
		}
		pReq->bAlarmCmd = true;
	}
	
	return E_EC_OK;
}


//构造
int NS_DeviceControlReq::Build(const void* pData, string& strXml)
{
	const TDeviceControlReq* pReq = (TDeviceControlReq*)pData;

	ostringstream oss;
	
	//构造头
	oss	<<XML_HEAD
		<<"<Control>\r\n"
		<<XML_CMD_TYPE("DeviceControl")
		<<XML_SN(pReq->nSn)
		<<XML_DEVICE_ID(pReq->strDeviceId);

/*	//构造云台信息
	if (pReq->oPtzCmd.bControl)
	{
		BuildPtzInfo(pReq->oPtzCmd, oss);
	}*/

	if (pReq->ptz!=-1)
	{
		if (pReq->ptz<PtzII)
			BuildPtz(pReq->ptz, pReq->ptzParam,oss);
		else if(pReq->ptz<PtzYS)
			BuildFi(pReq->ptz, pReq->ptzParam,oss);
		else
			BuildY(pReq->ptz, pReq->ptzParam,oss);

	}

	//是否远程启动
	if (pReq->bTeleBoot)
	{
		oss<<XML_ELEMENT1(TeleBoot,"Boot");
	}

	//录像信息
	if (pReq->eRecordCmd != E_RC_NONE)
	{
		oss<<XML_ELEMENT1(RecordCmd, 
			pReq->eRecordCmd==E_RC_RECORD?"Record":"StopRecord");
	}

	//报警撤防、布防信息
	if (pReq->eGuardCmd != E_GC_NONE)
	{
		oss<<XML_ELEMENT1(GuardCmd, 
			pReq->eGuardCmd==E_GC_SET_GUARD?"SetGuard":"ResetGuard");
	}

	//报警复位
	if (pReq->bAlarmCmd)
	{
		oss<<XML_ELEMENT1(AlarmCmd,"ResetAlarm");
	}

	//构造扩展信息
	if (!pReq->vExtendInfo.empty())
	{
		MsgBuildExtendInfo(pReq->vExtendInfo, oss);
	}

	strXml = oss.str();
	strXml += "</Control>\r\n";

	return E_EC_OK;
}

/*//解析ptz信息
int NS_DeviceControlReq::ParsePtzInfo(const char* pPtz, TPtzCmd& oPtzInfo)
{
	if(pPtz==NULL || strlen(pPtz)!=16)
	{
		WRITE_ERROR("云台信息无效\n");
		return E_EC_PARSE_XML_DOC;
	}
	oPtzInfo.bControl = true;

	//地址
	oPtzInfo.nAddress = HexChar2Num(pPtz[5])+(HexChar2Num(pPtz[4])<<4)+
		
		(HexChar2Num(pPtz[13])<<8);

	//变倍操作
	int nTemp = HexChar2Num(pPtz[6])&3;
	if (nTemp==0)
		oPtzInfo.eZoomOpt = E_ZOOM_NONE;
	else if(nTemp==1)
		oPtzInfo.eZoomOpt = E_ZOOM_IN;
	else
		oPtzInfo.eZoomOpt = E_ZOOM_OUT;

	//水平方向 第0、1位
	nTemp = HexChar2Num(pPtz[7])&3;
	if (nTemp==0)
		oPtzInfo.ePanOpt = E_PAN_NONE;
	else if(nTemp==1)
		oPtzInfo.ePanOpt = E_PAN_RIGHT;
	else
		oPtzInfo.ePanOpt = E_PAN_LEFT;

	//垂直方向
	nTemp = HexChar2Num(pPtz[7])&12;
	if (nTemp==0)
		oPtzInfo.eTiltOpt = E_TILT_NONE;
	else if(nTemp==4)
		oPtzInfo.eTiltOpt = E_TILT_DOWN;
	else
		oPtzInfo.eTiltOpt = E_TILT_UP;

	//水平速度
	oPtzInfo.nPanSpeed = (HexChar2Num(pPtz[8])<<4) + HexChar2Num(pPtz[9]);

	//垂直速度
	oPtzInfo.nTiltSpeed = (HexChar2Num(pPtz[10])<<4) + HexChar2Num(pPtz[11]);

	//变倍速度
	oPtzInfo.nZoomSpeed = HexChar2Num(pPtz[12]);
	return E_EC_OK;
}


//构造ptz信息
void NS_DeviceControlReq::BuildPtzInfo(const TPtzCmd& oPtzInfo, ostringstream& oss)
{
	assert(oPtzInfo.bControl);

	char sTempPtz[17];//8*2+1

	//固定信息 A5H
	sTempPtz[0] = 'A';
	sTempPtz[1] = '5';

	//版本信息
	sTempPtz[2] = '0';
	sTempPtz[3] = 'F'; //0+5+A

	//地址的低4位
	sTempPtz[4] = Num2HexChar(oPtzInfo.nAddress>>4);
	sTempPtz[5] = Num2HexChar(oPtzInfo.nAddress&0xF);

	//指令位
	sTempPtz[7] = 0;
	//水平方向 第0、1位
	if (oPtzInfo.ePanOpt!=E_PAN_NONE)
		sTempPtz[7] |= oPtzInfo.ePanOpt==E_PAN_LEFT?2:1;
	//垂直方向 第2、3位
	if (oPtzInfo.eTiltOpt!=E_TILT_NONE)
		sTempPtz[7] |= oPtzInfo.eTiltOpt==E_TILT_UP?8:4;
	sTempPtz[7] = Num2HexChar(sTempPtz[7]);
	//变倍信息 第4、5位
	sTempPtz[6] = 0;
	if (oPtzInfo.eZoomOpt!=E_ZOOM_NONE)
		sTempPtz[6] |=  oPtzInfo.eZoomOpt==E_ZOOM_OUT?2:1;
	sTempPtz[6] = Num2HexChar(sTempPtz[6]);


	//水平速度
	sTempPtz[8] = Num2HexChar(oPtzInfo.nPanSpeed>>4);
	sTempPtz[9] = Num2HexChar(oPtzInfo.nPanSpeed&0xF);

	//垂直速度
	sTempPtz[10] = Num2HexChar(oPtzInfo.nTiltSpeed>>4);
	sTempPtz[11] = Num2HexChar(oPtzInfo.nTiltSpeed&0xF);
	
	//变倍速度、
	sTempPtz[12] = Num2HexChar(oPtzInfo.nZoomSpeed&0xF);
	//地址高四位
	sTempPtz[13] = Num2HexChar(oPtzInfo.nAddress>>8);

	//校验码
	int nCheck = 0;
	for(int i=0; i<7; i++)
		nCheck += HexChar2Num(sTempPtz[2*i])<<4 | HexChar2Num(sTempPtz[2*i+1]);
	sTempPtz[14] = Num2HexChar((nCheck>>4)&0xf);
	sTempPtz[15] = Num2HexChar(nCheck&0xF);

	sTempPtz[16] = '\0';

	oss<<XML_ELEMENT1(PTZCmd, sTempPtz);
}
*/
