#include "RTMP/RTMPPusher.h"
#include "Simple-RTMP-Server/trunk/src/libs/srs_librtmp.hpp"
#include "Simple-RTMP-Server/trunk/src/kernel/srs_kernel_codec.hpp"

namespace Public {
namespace RTMP {

struct RTMPPusher::RTMPPusherInternal
{
	srs_rtmp_t	rtmp;
	bool		initflag;
};

RTMPPusher::RTMPPusher(const std::string& url)
{
	internal = new RTMPPusherInternal;
	internal->rtmp = srs_rtmp_create(url.c_str());
	internal->initflag = false;
}
RTMPPusher::~RTMPPusher()
{
	srs_rtmp_destroy(internal->rtmp);
}

//Á¬½Ó
bool RTMPPusher::connect()
{
	if (srs_rtmp_handshake(internal->rtmp) != 0) {
		return false;
	}
	if (srs_rtmp_connect_app(internal->rtmp) != 0) {
		return false;
	}
	if (srs_rtmp_publish_stream(internal->rtmp) != 0) {
		return false;
	}

	internal->initflag = true;

	return true;
}

bool RTMPPusher::writePacket(RTMPType type, uint32_t timestmap, const char* buffer, int maxlen)
{
	if (!internal->initflag || buffer == NULL || maxlen <= 0)
	{
		return false;
	}

	char atype = SrsFrameTypeScript;
	if (type == FlvTagAudio) atype = SrsFrameTypeAudio;
	else if (type == FlvTagVideo) atype = SrsFrameTypeAudio;
	else atype = SrsFrameTypeScript;

	if (srs_rtmp_write_packet(internal->rtmp, atype, timestmap, (char*)buffer, maxlen) != 0) {
		return false;
	}

	return true;
}

}
}
