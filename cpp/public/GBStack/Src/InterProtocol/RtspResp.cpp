#include "Public.h"
#include "RtspResp.h"

//解析
int NS_VodCtrlRtsp::Parse(string& strXml, void* pData)
{
	TVodCtrlResp* pRtsp = (TVodCtrlResp*)pData;
	

	const char* tmp = strXml.c_str();
	while(*tmp == ' ')
	{
		tmp ++;
	}
	if(memcmp(tmp, "RTSP/1.0", 8) != 0)
	{
		return E_EC_INVALID_PARAM;
	}

	if(pData == NULL)
	{
		return E_EC_OK;
	}

	pRtsp->nResult = false;
	pRtsp->nSn = 0;
	pRtsp->rtpinfo_seq = 0;
	pRtsp->rtpinfo_time = 0;

	const char* end = strstr(tmp,"OK");
	pRtsp->nResult = (end != NULL);

	tmp = strstr(strXml.c_str(),"CSeq:");
	if(tmp != NULL)
	{
		if ((end = strstr(tmp, "\r\n")) != NULL)
		{
			std::string cseqstr(tmp+5,end-(tmp+5));
			pRtsp->nSn = atoi(cseqstr.c_str());
		}
		else if ((end = strchr(tmp, '\n')) != NULL)
		{
			std::string cseqstr(tmp+5,end-(tmp+5));
			pRtsp->nSn = atoi(cseqstr.c_str());
		}
	/*
		end = strchr(tmp,'\n');
		if(end != NULL)
		{
			while(tmp != end)
			{
				if(*end == '\r' || *end == '\n')
				{
					end --;
				}
				else
				{
					break;
				}
			}
			std::string cseqstr(tmp,end-tmp);
			pRtsp->nSn = atoi(cseqstr.c_str());
		}*/
	}
	tmp = strstr(strXml.c_str(),"seq=");
	if(tmp != NULL)
	{
		if ((end = strchr(tmp,';')) != NULL)
		{
			std::string str(tmp+4,end-(tmp+4));
			pRtsp->rtpinfo_seq= atoi(str.c_str());
		}
		else if ((end = strstr(tmp, "\r\n")) != NULL)
		{
			std::string str(tmp+4,end-(tmp+4));
			pRtsp->rtpinfo_seq= atoi(str.c_str());
		}
		else if ((end = strchr(tmp, '\n')) != NULL)
		{
			std::string str(tmp+4,end-(tmp+4));
			pRtsp->rtpinfo_seq= atoi(str.c_str());
		}

		
		/*
		end = strchr(tmp,';');
		if(end == NULL)
		{
			end = strchr(tmp,'\n');
			// 以下是修改
			if(end != NULL)		
			{
				while(tmp != end)
				{
					if(*end == '\r' || *end == '\n'|| *end == ';')
					{
						end --;
					}
					else
					{
						break;
					}
				}
				std::string str(tmp,end-tmp);
				pRtsp->rtpinfo_seq= atoi(str.c_str());
			}
		}
		if(end != NULL)
		{
			while(tmp != end)
			{
				if(*end == '\r' || *end == '\n'|| *end == ';')
				{
					end --;
				}
				else
				{
					break;
				}
			}
			std::string str(tmp,end-tmp);
			pRtsp->rtpinfo_seq= atoi(str.c_str());
		}*/
	}
	tmp = strstr(strXml.c_str(),"rtptime=");
	if(tmp != NULL)
	{
		if ((end = strstr(tmp, "\r\n")) != NULL)
		{			
			std::string str(tmp+8, end-(tmp+8));
			pRtsp->rtpinfo_time= (long)atoi(str.c_str());
		}
		else if ((end = strchr(tmp,'\n')) != NULL)
		{			
			std::string str(tmp+8, end-(tmp+8));
			pRtsp->rtpinfo_time = (long)atoi(str.c_str());			
		}
		else if ((end = strchr(tmp, ';')) != NULL)
		{
			std::string str(tmp+8, end-(tmp+8));
			pRtsp->rtpinfo_time = (long)atoi(str.c_str());			
		}
	/*
		end = strchr(tmp,';');
		if(end == NULL)
		{
			end = strchr(tmp,'\n');

			if(end != NULL)
			{
				while(tmp != end)
				{
					if(*end == '\r' || *end == '\n' || *end == ';')
					{
						end --;
					}
					else
					{
						break;
					}
				}
				std::string str(tmp,end-tmp);
				pRtsp->rtpinfo_time = (long)atoi(str.c_str());
			}
		}

		if(end != NULL)
		{
			while(tmp != end)
			{
				if(*end == '\r' || *end == '\n' || *end == ';')
				{
					end --;
				}
				else
				{
					break;
				}
			}
			std::string str(tmp,end-tmp);
			pRtsp->rtpinfo_time = atoi(str.c_str());
		}
		*/
	}
	
	return E_EC_OK;
}

//构造
int NS_VodCtrlRtsp::Build(const void* pData, string& strXml)
{
	TVodCtrlResp* pRtsp = (TVodCtrlResp*)pData;

	if(!pRtsp->nResult)
	{
		return E_EC_INVALID_PARAM;
	}

	ostringstream oss;

	oss << "RTSP/1.0 200 OK\r\n"
		<<"CSeq:"<<pRtsp->nSn<<"\r\n";

	if(pRtsp->rtpinfo_seq != 0 || pRtsp->rtpinfo_time != 0)
	{
		oss <<"RTP-Info: seq="<<pRtsp->rtpinfo_seq<<";rtptime="<<pRtsp->rtpinfo_time<<"\r\n";
	}
		
	strXml = oss.str();
	return E_EC_OK;
}

