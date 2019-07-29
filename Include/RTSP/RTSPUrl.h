#pragma once
#pragma once
#include "Defs.h"
#include "Base/Base.h"
#include "RTSPStructs.h"
using namespace Public::Base;

namespace Public {
namespace RTSP {

class RTSP_API RTSPUrl
{
public:
	std::string username;
	std::string password;
	std::string rtspurl;
	std::string serverip;
	int			serverport;
public:
	RTSPUrl();
	RTSPUrl(const RTSPUrl& url);
	RTSPUrl(const std::string& url);
	~RTSPUrl();

	bool parse(const std::string& url);
};

}
}