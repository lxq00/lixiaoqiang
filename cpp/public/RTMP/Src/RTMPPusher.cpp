#pragma once

#include "Defs.h"
#include "Base/Base.h"
using namespace Public::Base;

namespace Public {
namespace RTMP {

class RTMP_API RTMPPusher
{
public:
	RTMPPusher(const std::string& url);
	~RTMPPusher();

	//����
	bool connect();

	int write(const char* buffer, int maxlen);
private:
	struct RTMPPusherInternal;
	RTMPPusherInternal *internal;
};

}
}
