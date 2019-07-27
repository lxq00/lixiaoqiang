#include "RTSPCMD.h"

#define PROTOCOL				"RTSP/1.0"


SDPInfo::SDPInfo(int startChannel) :channel(startChannel), ctrolChannel(startChannel + 1), clientPort(0), clientCtrolPort(0), serverPort(0), serverCtrolPort(0) {}

void SDPInfo::parse(const char* sdkinfo)
{
	if (sdkinfo == NULL) return;
	const char* pFlag = strstr(sdkinfo, "interleaved=");
	if (NULL != pFlag)
	{
		if (sscanf(pFlag, "interleaved=%d-%d", &channel, &ctrolChannel) != 2)
		{
		}
	}

	const char* pFlagClientPort = strstr(sdkinfo, "client_port=");
	if (NULL != pFlagClientPort)
	{
		if (sscanf(pFlagClientPort, "client_port=%d-%d", &clientPort, &clientCtrolPort) != 2)
		{
		}
	}

	const char* pFlagServerPort = strstr(sdkinfo, "server_port=");

	if (pFlagServerPort != NULL)
	{
		if (sscanf(pFlagServerPort, "server_port=%d-%d", &serverPort, &serverCtrolPort) != 2)
		{
		}
	}
}

std::string SDPInfo::build(RTP_RECV_Type m_nRecvType)
{
	char buffer[256];
	if (RTP_RECV_TYPE_TCP == m_nRecvType)
	{
		snprintf_x(buffer, 255, "RTP/AVP/TCP;unicast;interleaved=%d-%d", channel, ctrolChannel);
	}
	else if (RTP_RECV_TYPE_UDP == m_nRecvType)
	{
		sprintf(buffer, "RTP/AVP;unicast;client_port=%d-%d", clientPort, clientCtrolPort);
	}

	return buffer;
}

std::string RTSPUrlInfo::buildAuthenString(const std::string& cmd)
{
	if (!needAuth) return "";

	std::string authstring;
	if (needBasic)
	{
		std::string authyenstr = m_szUserName + ":" + m_szPassWord;

		std::string buffer = Base64::encode(authyenstr);

		authstring = std::string("Basic ") + buffer;
	}
	else
	{
		std::string authen1,authen2,authen3;
		{
			std::string strHash1src = m_szUserName + ":" + m_szRealm + ":" + m_szPassWord;


			Md5 md5;
			md5.init();
			md5.update((uint8_t const*)strHash1src.c_str(), strHash1src.length());
			authen1 = md5.hex();
		}

		{
			std::string strHash2src = cmd + ":" + m_szRtspUrl;

			Md5 md5;
			md5.init();
			md5.update((uint8_t const*)strHash2src.c_str(), strHash2src.length());
			authen2 = md5.hex();
		}

		{
			std::string strHash3src = std::string(authen1) + ":" + m_szNonce + ":" + authen2;

			Md5 md5;
			md5.init();
			md5.update((uint8_t const*)strHash3src.c_str(), strHash3src.length());
			authen3 = md5.hex();
		}

		authstring = std::string("Digest username=\"") + m_szUserName + "\", realm=\"" + m_szRealm +"\", nonce=\"" + m_szNonce + "\", uri=\""
			+ m_szRtspUrl + "\", response=\"" + authen3 + "\"";
	}

	return std::string("Authorization: ") + authstring + "\r\n";
}
bool RTSPUrlInfo::checkAuthen(const std::string& cmd,const std::string& username, const std::string& password, const std::string& pAuthorization)
{
	if (NULL == strstr(pAuthorization.c_str(), "Digest"))
	{
		const char* authenstart = strstr(pAuthorization.c_str(), "Basic ");
		if (authenstart == NULL) return false;
		
		std::string authenresult = pAuthorization.c_str() + strlen("Basic ");

		std::string authyenstr = username + ":" + password;
		std::string buffer = Base64::encode(authyenstr);

		return buffer == authenresult;
	}
	else
	{
		std::string  szrealm = getAuthenAttr(pAuthorization,"realm");
		std::string szRtspUrl = getAuthenAttr(pAuthorization, "uri");
		std::string szNonce = getAuthenAttr(pAuthorization, "nonce");
		std::string szResponse = getAuthenAttr(pAuthorization, "response");

		std::string authen1, authen2, authen3;
		{
			std::string strHash1src = username + ":" + szrealm + ":" + password;


			Md5 md5;
			md5.init();
			md5.update((uint8_t const*)strHash1src.c_str(), strHash1src.length());
			authen1 = md5.hex();
		}

		{
			std::string strHash2src = cmd + ":" + szRtspUrl;

			Md5 md5;
			md5.init();
			md5.update((uint8_t const*)strHash2src.c_str(), strHash2src.length());
			authen2 = md5.hex();
		}

		{
			std::string strHash3src = std::string(authen1) + ":" + szNonce + ":" + authen2;

			Md5 md5;
			md5.init();
			md5.update((uint8_t const*)strHash3src.c_str(), strHash3src.length());
			authen3 = md5.hex();
		}

		return response == authen3;
	}
}
RTSPUrlInfo::RTSPUrlInfo() :m_nCSeq(0), needAuth(false) {}

bool RTSPUrlInfo::parse(const std::string& url)
{
	//检查前缀
	char const* prefix = "rtsp://";
	unsigned const prefixLength = 7;

	if (url.length() < prefixLength || 0 != strncasecmp(url.c_str(), prefix, prefixLength))
	{
		return false;
	}
	//rtsp://192.168.0.13:554/cam/realmonitor?channel=1&subtype=0&unicast=true&proto=Onvif
	//rtsp://admin:123123@192.168.1.75/media/video1

	//解析出用户名密码
	const char* userstart = url.c_str() + prefixLength;
	const char* userend = strchr(userstart, '@');
	if (userend != NULL)
	{
		std::string userinfo(userstart, userend - userstart);
		const char* userinfoptr = userinfo.c_str();
		const char* passstart = strchr(userinfoptr, ':');

		m_szUserName = userinfo;
		if (passstart != NULL)
		{
			m_szUserName = std::string(userinfoptr, passstart - userinfoptr);
			m_szPassWord = passstart + 1;
		}

		userstart = userend + 1;
	}
	m_szRtspUrl = std::string(prefix) + userstart;

	const char* addrend = strchr(userstart, '/');
	if (addrend == NULL)
	{
		return false;
	}

	std::string addrinfo(userstart, addrend - userstart);
	m_szServerIp = addrinfo;
	m_nServerPort = 554;
	const char* addrinfoptr = addrinfo.c_str();
	const char* portstart = strchr(addrinfoptr, ':');
	if (portstart != NULL)
	{
		m_szServerIp = std::string(addrinfoptr, portstart - addrinfoptr);
		m_nServerPort = atoi(portstart + 1);
	}
	if (m_nServerPort < 1 || m_nServerPort > 65535)
	{
		return false;
	}

	return true;
}

bool RTSPUrlInfo::parseAuthenString(const std::string& pAuthorization)
{
	needAuth = true;

	if (NULL != strstr(pAuthorization.c_str(), "Digest"))
	{
		needBasic = false;

		m_szRealm = getAuthenAttr(pAuthorization,"realm");
		m_szNonce = getAuthenAttr(pAuthorization, "nonce");
	}
	else
	{
		needBasic = true;
	}

	return true;
}
std::string RTSPUrlInfo::getAuthenAttr(const std::string& pAuthorization, const std::string& key)
{
	const char* tmpstr = pAuthorization.c_str();
	
	std::string flag = key + "=\"";
	const char* findstr = strstr(tmpstr, flag.c_str());
	if (findstr == NULL)
	{
		return "";
	}
	findstr += flag.length();
	const char* endstr = strchr(findstr, '"');
	if (endstr == NULL)
	{
		return "";
	}
	return std::string(findstr, endstr - findstr);

}

CMDBuilder_OPTIONS::CMDBuilder_OPTIONS() :CMDBuilder("OPTIONS", RTSPCmd_OPTIONS) {}

CMDBuilder_OPTIONS::~CMDBuilder_OPTIONS() {}

std::string CMDBuilder_OPTIONS::build(shared_ptr<RTSPUrlInfo>& url)
{
	m_nCSeq = ++url->m_nCSeq;

	std::string buffer = cmd + " " + url->m_szRtspUrl + " " + PROTOCOL + "\r\n"
		+ "CSeq: " + Value(m_nCSeq).readString() + "\r\n"
		+ url->buildAuthenString(cmd)
		+ "User-Agent: " + url->m_userAgent + "\r\n"
		+ "\r\n";

	return buffer;
}

CMDBuilder_DESCRIBE::CMDBuilder_DESCRIBE() :CMDBuilder("DESCRIBE", RTSPCmd_DESCRIBE) { }
CMDBuilder_DESCRIBE::~CMDBuilder_DESCRIBE() {}

std::string CMDBuilder_DESCRIBE::build(shared_ptr<RTSPUrlInfo>& url)
{
	m_nCSeq = ++url->m_nCSeq;

	std::string buffer = cmd + " " + url->m_szRtspUrl + " " + PROTOCOL + "\r\n"
		+ "CSeq: " + Value(m_nCSeq).readString() + "\r\n"
		+ url->buildAuthenString(cmd)
		+ "User-Agent: " + url->m_userAgent + "\r\n"
		+ "Accept: application/sdp\r\n"
		+ "\r\n";

	return buffer;
}

CMDBuilder_SETUP::CMDBuilder_SETUP(const std::string& _trackurl, const std::string& _port, bool video)
	:CMDBuilder("SETUP", RTSPCmd_SETUP)
	,trackurl(_trackurl)
	,transport(_port)
	,isVideo(true)
{ }

CMDBuilder_SETUP::~CMDBuilder_SETUP() {}

std::string CMDBuilder_SETUP::build(shared_ptr<RTSPUrlInfo>& url)
{
	m_nCSeq = ++url->m_nCSeq;

	bool bUrlTypeTraclID = false;
	if (NULL != strstr(trackurl.c_str(), (char*)"rtsp://"))
	{
		bUrlTypeTraclID = true;
	}

	if (!bUrlTypeTraclID)
	{
		trackurl = url->m_szRtspUrl + "/"+ trackurl;
	}

	std::string sesionstr = url->m_szSession == "" ? "" : (std::string("Session: ") + url->m_szSession + "\r\n");

	std::string buffer = cmd + " " + trackurl + " " + PROTOCOL + "\r\n"
		+ "CSeq: " + Value(m_nCSeq).readString() + "\r\n"
		+ url->buildAuthenString(cmd)
		+ "Transport: " + transport + "\r\n"
		+ sesionstr
		+ "\r\n";

	return buffer;
}

CMDBuilder_PLAY::CMDBuilder_PLAY(const std::string& _type, const std::string& _starttime, const std::string& _stoptime) :CMDBuilder("PLAY", RTSPCmd_PLAY)
{
	type = _type;
	startTime = _starttime;
	stopTime = _stoptime;
}
CMDBuilder_PLAY::~CMDBuilder_PLAY() {}

std::string CMDBuilder_PLAY::build(shared_ptr<RTSPUrlInfo>& url)
{
	std::string playinfo = "Range: npt=0.000-\r\n";

	if (type != "")
	{
		playinfo = std::string("Range: ")+type+"="+startTime+"-"+stopTime+"\r\n";
	}

	m_nCSeq = ++url->m_nCSeq;

	std::string buffer = cmd + " " + url->m_szRtspUrl + " " + PROTOCOL + "\r\n"
		+ "CSeq: " + Value(m_nCSeq).readString() + "\r\n"
		+ url->buildAuthenString(cmd)
		+ "Session: " + url->m_szSession + "\r\n"
		+ playinfo
		+ "\r\n";

	return buffer;
}

CMDBuilder_GET_PARAMETER::CMDBuilder_GET_PARAMETER() :CMDBuilder("GET_PARAMETER", RTSPCmd_GET_PARAMETER) { }
CMDBuilder_GET_PARAMETER::~CMDBuilder_GET_PARAMETER() {}

std::string CMDBuilder_GET_PARAMETER::build(shared_ptr<RTSPUrlInfo>& url)
{
	m_nCSeq = ++url->m_nCSeq;

	std::string buffer = cmd + " " + url->m_szRtspUrl + " " + PROTOCOL + "\r\n"
		+ "CSeq: " + Value(m_nCSeq).readString() + "\r\n"
		+ url->buildAuthenString(cmd)
		+ "Session: " + url->m_szSession + "\r\n"
		+ "\r\n";

	return buffer;
}

CMDBuilder_TEARDOWN::CMDBuilder_TEARDOWN() :CMDBuilder("TEARDOWN", RTSPCmd_TEARDOWN) { }
CMDBuilder_TEARDOWN::~CMDBuilder_TEARDOWN() {}

std::string CMDBuilder_TEARDOWN::build(shared_ptr<RTSPUrlInfo>& url)
{
	m_nCSeq = ++url->m_nCSeq;

	std::string buffer = cmd + " " + url->m_szRtspUrl + " " + PROTOCOL + "\r\n"
		+ "CSeq: " + Value(m_nCSeq).readString() + "\r\n"
		+ url->buildAuthenString(cmd)
		+ "Session: " + url->m_szSession + "\r\n"
		+ "\r\n";

	return buffer;
}
