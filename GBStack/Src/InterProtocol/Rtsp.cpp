#include "Public.h"
#include "Rtsp.h"

//命令
const char* g_pRtspType[] = 
{"PLAY","PAUSE", "TEARDOWN"};

bool GetRtspValue(string& rtsp, const char* name, string& value)
{
	int pos = rtsp.find(name);
	if (pos<0)
		return false;
	
	pos+= strlen(name);
	int end_pos = rtsp.find("\r\n", pos);
	if (end_pos<0)
		return false;

	value = rtsp.substr(pos, end_pos-pos);
	return true;
}

//解析
int NS_VodCtrl::Parse(string& strXml, void* pData)
{
	TVodCtrlReq* pRtsp = (TVodCtrlReq*)pData;

	//找命令类型(第一个空格之前)
	int nPos = strXml.find(' ');
	if (nPos<=1)
	{
		//WRITE_ERROR("没找到命令头");
		return E_EC_INVALID_DOC;
	}

	bool SurportRtsp=false;

	string strTemp = strXml.substr(0, nPos);
	for (int i=0/*E_RT_PALY*/; i<=2/*E_RT_TEARDOWN*/; i++)
	{
		if (strTemp == g_pRtspType[i])
		{
			SurportRtsp = true;
			break;
		}
	}	

	if(pData == NULL)
	{
		return SurportRtsp ? E_EC_OK : E_EC_INVALID_CMD_TYPE;
	}
	
	
	pRtsp->lStartTime = -1;
	pRtsp->dSpeed = -1;

	strTemp = strXml.substr(0, nPos);
	int i=0;
	for (/*E_RT_PALY*/; i<=2/*E_RT_TEARDOWN*/; i++)
	{
		if (strTemp == g_pRtspType[i])
		{
			pRtsp->eType = (ERtspType)i;
			break;
		}
	}
	if (i>2)
	{
		WRITE_ERROR("无法识别的命令对象");
		return E_EC_INVALID_DOC;
	}

	//解析cseq
	if (!GetRtspValue(strXml, "\r\nCSeq:", strTemp))
	{
		WRITE_ERROR("读取CSeq字段出错");
		return E_EC_INVALID_DOC;
	}
	pRtsp->nSn = atoi(strTemp.c_str());

	if (pRtsp->eType==E_RT_PLAY)
	{
		//读取npt
		if (GetRtspValue(strXml, "\r\nRange:", strTemp))
		{
			const char* npttmp = strstr(strTemp.c_str(),"npt=");
			if(npttmp != NULL)
			{
				const char* starttime = npttmp+4;
				const char* endtime = strchr(starttime,'-');
				std::string timetmp;
				if(endtime != NULL)
				{
					timetmp = std::string(starttime,endtime-starttime);
				}
				else
				{
					timetmp = starttime;
				}
				pRtsp->lStartTime = atoi(timetmp.c_str());
			}			
		}
		
		if (GetRtspValue(strXml, "\r\nScale:", strTemp))
		{
			pRtsp->dSpeed = atof(strTemp.c_str());
		}
	}
	return E_EC_OK;
}

//构造
int NS_VodCtrl::Build(const void* pData, string& strXml)
{
	TVodCtrlReq* pRtsp = (TVodCtrlReq*)pData;
	if (pRtsp->eType<E_RT_PLAY || pRtsp->eType>E_RT_TEARDOWN)
	{
		WRITE_ERROR("类型无效\n");
		return E_EC_INVALID_PARAM;
	}

static int cseq = 0;
	
	pRtsp->nSn = cseq ++;
	
	ostringstream oss;
	
	oss<<g_pRtspType[pRtsp->eType]<<" RTSP/1.0\r\n"
		<<"CSeq: "<<pRtsp->nSn<<"\r\n";
	
	if (pRtsp->eType==E_RT_PAUSE)
	{
		//oss<<"PauseTime: now\r\n";
		oss<<"PauseTime: "<< pRtsp->lStartTime <<"\r\n";
	}
	else if(pRtsp->eType==E_RT_PLAY)
	{
		if(pRtsp->dSpeed > 0)
		{
			char Scale[32];
			int	 afterPoint = 0;
			int  brforPoint = (int)pRtsp->dSpeed;
			
			sprintf(Scale,"%f",pRtsp->dSpeed);			
			const char* tmp = strchr(Scale, '.');
			if (tmp != NULL)

			{
				afterPoint = atoi(tmp+1);
			}
			memset(Scale, 0, 32);			
			sprintf(Scale,"%1d.%d", brforPoint, afterPoint);
			
			oss<<"Scale: "<<Scale<<"\r\n";
		}
		if(pRtsp->lStartTime >= 0)
		{
			oss<<"Range: "<<"npt="<<pRtsp->lStartTime<<"-"<<"\r\n";		
		}
	}
	oss<<"\r\n";
	
	strXml = oss.str();
	return E_EC_OK;
}

