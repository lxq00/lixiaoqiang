#ifndef __SIPSDP_H__
#define __SIPSDP_H__

#include <stdint.h>
#include <list>
#include <map>
#include "eXosip2/eXosip.h"
#include "Base/Base.h"
#include <set>
using namespace std;
namespace Public{
namespace SIPStack {

#define SDPFORMAT  		"v=0\r\n" \
	"o=%s 0 0 IN IP4 %s\r\n" \
	"s=%s\r\n" \
	"%s" \
	"c=IN IP4 %s\r\n"\
	"t=%ld %ld\r\n" \
	"%s" \
	"a=recvonly \r\n"\
	"%s"\
	"%s"


std::list<SDPMediaInfo> getDefaultSDPList()
{
	std::list<SDPMediaInfo> defaultlist;
	
	SDPMediaInfo info;
	info.MediaType = "H264";
	info.Platload = 96;
	defaultlist.push_back(info);

	info.MediaType = "MPEG4";
	info.Platload = 97;
	defaultlist.push_back(info);

	return defaultlist;
}

#define REALPLAYDEF "Play"
#define VODPLAYDEF	"Playback"
#define DOWNPLAYDEF "Download"						
std::string getPlayTypeString(StreamSessionType type)
{
	if(type == StreamSessionType_VOD)
	{
		return VODPLAYDEF;
	}else if(type == StreamSessionType_Download)
	{
		return DOWNPLAYDEF;
	}
	return REALPLAYDEF;
}	

std::string s_buildSDPXml(const std::string& myid,const StreamSessionSDP& _sdpinfo)
{
	StreamSessionSDP sdpinfo = _sdpinfo;
	if(sdpinfo.MediaList.size() == 0)
	{
		sdpinfo.MediaList = getDefaultSDPList();
	}

	std::string sdpMediaInfo;
	std::string sdpmString;
	{
		std::set<int> 	ptList;
		for(std::list<SDPMediaInfo>::iterator iter = sdpinfo.MediaList.begin();iter != sdpinfo.MediaList.end();iter ++)
		{
			std::set<int>::iterator ptiter = ptList.find(iter->Platload);
			if(ptiter != ptList.end())
			{
				continue;
			}

			char sdpInfo[128] = {0};
			snprintf(sdpInfo,128,"a=rtpmap:%d %s/90000\r\n",iter->Platload,iter->MediaType.c_str());
			sdpMediaInfo += sdpInfo;

			ptList.insert(iter->Platload);
		}
		if(sdpinfo.type == StreamSessionType_Download)
		{
			char buff[64] = {0};
			snprintf(buff, 64, "a=downloadspeed:%d\r\n", sdpinfo.downSpeed);
			sdpMediaInfo += buff;
		}	

		{
			std::string sdpPtListStr;
			int sdpIndex = 0;
			std::set<int>::iterator piter;
			for(piter = ptList.begin();piter != ptList.end();piter++)
			{
				char sdpInfo[32] = {0};
				snprintf(sdpInfo,32,"%s%d",sdpIndex == 0? "":" ",*piter);
				sdpPtListStr += sdpInfo;
			}

		
			char tmpbuf[128] = {0};
			snprintf(tmpbuf,128,"m=video %d %s %s\r\n",sdpinfo.port,sdpinfo.SocketType == SocketType_UDP ? "RTP/AVP":"TCP",sdpPtListStr.c_str());

			sdpmString = tmpbuf;
		}
	}
	std::string sdpuString;
	{
		char tmpbuf[128] = {0};
		if(sdpinfo.type == StreamSessionType_Realplay || sdpinfo.type== StreamSessionType_ThirdCall)
		{
			sdpinfo.vodStartTime = 0;
			sdpinfo.VodEndTime = 0;
			if(sdpinfo.type == StreamSessionType_Realplay && sdpinfo.streamType != 0)
			{
				snprintf(tmpbuf,128,"u=%s:%d\r\n",sdpinfo.deviceId.c_str(),sdpinfo.streamType);
			}
		}
		else if(sdpinfo.type == StreamSessionType_Download)
		{
			snprintf(tmpbuf,128,"u=%s:3\r\n",sdpinfo.deviceId.c_str());
		}

		sdpuString = tmpbuf;
	}
	std::string ssrcString;
	{
		if(sdpinfo.ssrc != "")
		{
			char tmpbuf[128] = {0};
			snprintf(tmpbuf,128,"y=%s\r\n",sdpinfo.ssrc.c_str());
			ssrcString = tmpbuf;
		}
	}
	
	char xml[512];
	snprintf(xml,512,SDPFORMAT,myid.c_str()
		/*sdpinfo.deviceId.c_str()*/
		,sdpinfo.ipAddr.c_str(),
		getPlayTypeString(sdpinfo.type).c_str(),sdpuString.c_str(),
		sdpinfo.ipAddr.c_str(),sdpinfo.vodStartTime,sdpinfo.VodEndTime,sdpmString.c_str(),sdpMediaInfo.c_str(),ssrcString.c_str());

	return xml;
}

bool s_parseSdPByMessage(StreamSessionSDP& sdpinfo,const std::string& deviceId,const std::string& sdpString)
{	
	sdpinfo.deviceId = deviceId;

	const char*pXmlStr = sdpString.c_str();

	//deviceId
	{
		const char* srcid = strstr(pXmlStr,"o=");
		if(srcid != NULL)
		{
			srcid += 2;
			const char* srcidend = strchr(srcid,' ');
			if(srcidend != NULL)
			{
				sdpinfo.deviceId = std::string(srcid,srcidend - srcid);
			}
		}
	}
	
	//ipaddr
	{
		const char *addr = strstr(pXmlStr,"c=IN IP4 ");
		if(addr == NULL)
		{
			return false;
		}
		addr += 9;
		const char* addrend = strstr(addr,"\r\n");
		if(addrend == NULL)
			addrend = strstr(addr,"\n");
		if(addrend == NULL)
		{
			return false;
		}
		sdpinfo.ipAddr = std::string(addr,addrend - addr);
	}
	//port sockettype
	{
		const char* pport = strstr(pXmlStr,"m=video ");
		if(pport == NULL)
		{
			return false;
		}
		pport += 8;
		const char* portend = strchr(pport,' ');
		if(portend == NULL)
		{
			return false;
		}
		sdpinfo.port = atoi(std::string(pport,portend - pport).c_str());

		const char* pportend = strstr(pport,"\r\n");
		if(pportend == NULL)
			pportend = strstr(pport,"\n");
		if(pportend == NULL)
		{
			return false;
		}

		bool isTcp = strstr(std::string(pport,pportend - pport).c_str(),"TCP") != NULL;
		if(!isTcp)
		{
			isTcp = strstr(std::string(pport,pportend - pport).c_str(),"tcp") != NULL;
		}

		sdpinfo.SocketType = isTcp ? SocketType_TCP : SocketType_UDP;
	}
	
	///type
	{
		sdpinfo.type = StreamSessionType_Realplay;
		const char* realpaly = strstr(pXmlStr,"s=");
		if(realpaly != NULL)
		{
			realpaly += 2;
			if(strncasecmp(realpaly,VODPLAYDEF,strlen(VODPLAYDEF)) == 0)
			{
				sdpinfo.type = StreamSessionType_VOD;
			}
			else if(strncasecmp(realpaly,DOWNPLAYDEF,strlen(DOWNPLAYDEF)) == 0)
			{
				sdpinfo.type = StreamSessionType_Download;
			}
		}
	}
	
	///starttime  stoptime
	{
		if(sdpinfo.type != StreamSessionType_Realplay)
		{
			const char* timestr = strstr(pXmlStr,"t=");
			if(timestr == NULL)
			{
				delete pXmlStr;
				return false;
			}
			timestr += 2;
			const char* timeend = strstr(timestr,"\r\n");
			if(timeend == NULL)
				timeend = strstr(timestr,"\n");
			if(timeend == NULL)
			{
				return false;
			}
			char tmp[128];
			memcpy(tmp,timestr,timeend-timestr);
			tmp[timeend-timestr] = 0;
			sscanf(tmp,"%ld %ld",&sdpinfo.vodStartTime,&sdpinfo.VodEndTime);
		}
	}

	//downloadspeed
	{
		if (sdpinfo.type == StreamSessionType_Download)
		{
			const char* speed = strstr(pXmlStr, "a=downloadspeed:");
			if (NULL != speed)
			{
				const char* speed_end = strstr(speed + 16, "\r\n");
				if (speed_end == NULL){
					speed_end = strstr(speed + 16, "\n");
				}
				std::string s_speed(speed + 16, speed_end - (speed + 16));
				sdpinfo.downSpeed = atoi(s_speed.c_str());
			}
		}
	}

	//MediaList
	{
		const char* rtpmapHeader = "a=rtpmap:";
		//const int rtpmapLen = strlen(rtpmapHeader);
		const char* rtpmapString = strstr(pXmlStr, rtpmapHeader);
		while(NULL != rtpmapString)
		{
			const char* seperator = strstr(rtpmapString, "\r\n");
			int stringlen = seperator - rtpmapString;
			std::string rtpmapItem = std::string(rtpmapString, stringlen);
			std::string key_value = std::string(strchr(rtpmapItem.c_str(), ':') + 1, strchr(rtpmapItem.c_str(),'/') - strchr(rtpmapItem.c_str(),':') - 1);

			const char* space = strchr(key_value.c_str(),' ');
			int keylen = space - key_value.c_str();
			std::string key = std::string(key_value.c_str(), keylen);
			std::string value = std::string(space + 1, key_value.length() - keylen - 1);

			SDPMediaInfo info;
			info.Platload = atoi(key.c_str());
			info.MediaType = value;
			sdpinfo.MediaList.push_back(info);

			rtpmapString = strstr(rtpmapString + stringlen, rtpmapHeader);
		}
	}
	
	//streamType
	{
		if(sdpinfo.type == StreamSessionType_Realplay)
		{
			const char* tmp = strstr(pXmlStr,"u=");
			if(tmp != NULL)
			{
				const char* tmpend = strstr(tmp,"\r\n");
				if(tmpend == NULL)
					tmpend = strstr(tmp,"\n");
				if(tmpend != NULL)
				{
					std::string extu = string(tmp + 2,tmpend - tmp - 2);
					const char* tmp1 = strchr(extu.c_str(),':');
					if(tmp1 != NULL)
					{
						sdpinfo.streamType = atoi(tmp1 + 1);
					}
				}
			}
		}
	}	
	///ssrc
	{
		const char* tmp = strstr(pXmlStr,"y=");
		if(tmp != NULL)
		{
			tmp += 2;
			const char* tmpend = strstr(tmp,"\r\n");
			if(tmpend == NULL)
				tmpend = strstr(tmp,"\n");
			if(tmpend != NULL)
			{
				sdpinfo.ssrc = std::string(tmp,tmpend - tmp);
			}
		}
	}

	return true;
}

std::string s_BuildSubject(const std::string& myid,const StreamSessionSDP& sdpinfo)
{
	char xuliehao[256];
	snprintf_x(xuliehao,255,"%s:%c%s,%s:%d%s",sdpinfo.deviceId.c_str(),
		(sdpinfo.type==StreamSessionType_Realplay)?'0':'1',sdpinfo.ssrc.c_str(),
		myid.c_str(),
		(sdpinfo.type==StreamSessionType_Realplay)?'0':'1',sdpinfo.ssrc.c_str());

	return xuliehao;
}

bool s_ParseSubject(const std::string& subjectstr,std::string& mediassrcstr,std::string& recvssrcstr)
{
	char mediaid[255],mediassrc[255],recvid[255],recvssrc[255];
	sscanf(subjectstr.c_str(),"%[^:]:%[^,]%[^:]:%s",mediaid,mediassrc,recvid,recvssrc);

	mediassrcstr = &mediassrc[1];
	recvssrcstr = &recvssrc[1];


	return true;
}

}
}
#endif