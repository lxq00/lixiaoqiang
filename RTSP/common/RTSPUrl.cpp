#include "RTSP/RTSPUrl.h"

using namespace Public::RTSP;


RTSPUrl::RTSPUrl() :serverport(0) {}
RTSPUrl::RTSPUrl(const std::string& url) : serverport(0) { parse(url); }
RTSPUrl::RTSPUrl(const RTSPUrl& url)
{
	username = url.username;
	password = url.password;
	rtspurl = url.rtspurl;
	serverip = url.serverip;
	serverport = url.serverport;
}
RTSPUrl::~RTSPUrl() {}

bool RTSPUrl::parse(const std::string& url)
{
	//检查前缀
	char const* prefix = "rtsp://";
	unsigned const prefixLength = strlen(prefix);

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

		username = userinfo;
		if (passstart != NULL)
		{
			username = std::string(userinfoptr, passstart - userinfoptr);
			password = passstart + 1;
		}

		userstart = userend + 1;
	}
	rtspurl = std::string(prefix) + userstart;

	const char* addrend = strchr(userstart, '/');
	if (addrend == NULL)
	{
		return false;
	}

	std::string addrinfo(userstart, addrend - userstart);
	serverip = addrinfo;
	serverport = 554;

	const char* addrinfoptr = addrinfo.c_str();
	const char* portstart = strchr(addrinfoptr, ':');
	if (portstart != NULL)
	{
		serverip = std::string(addrinfoptr, portstart - addrinfoptr);
		serverport = atoi(portstart + 1);
	}
	if (serverport < 1 || serverport > 65535)
	{
		return false;
	}

	return true;
}

