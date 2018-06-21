#pragma once

#include "Defs.h"
#include "Base/Base.h"
using namespace Public::Base;

namespace Public {
namespace RTMP {

class RTMP_API RTMPClient
{
public:
	RTMPClient(const std::string& url);
	~RTMPClient();

	//Á¬½Ó
	bool connect();

	int readPacket(uint32_t& timestamp, RTMPType& type,char* buffer, int maxlen);
private:
	struct RTMPClientInternal;
	RTMPClientInternal *internal;
};

}
}
