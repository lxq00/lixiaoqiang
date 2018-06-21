#include "RTMP/RTMPClient.h"
#include "Simple-RTMP-Server/trunk/src/libs/srs_librtmp.hpp"

namespace Public {
namespace RTMP {

struct RTMPClient::RTMPClientInternal
{
	srs_rtmp_t	rmp;
	bool		initflag;
};

RTMPClient::RTMPClient(const std::string& url)
{
	internal = new RTMPClientInternal;
	internal->rmp = srs_rtmp_create(url.c_str());
	internal->initflag = false;
}
RTMPClient::~RTMPClient()
{
	srs_rtmp_destroy(internal->rmp);
	internal->initflag = false;
	SAFE_DELETE(internal);
}

bool RTMPClient::connect()
{
	if (srs_rtmp_handshake(internal->rmp) != 0) {
		return false;
	}
	if (srs_rtmp_connect_app(internal->rmp) != 0) {
		return false;
	}
	if (srs_rtmp_play_stream(internal->rmp) != 0) {
		return false;
	}
	internal->initflag = true;

	return true;
}

int RTMPClient::readPacket(uint32_t& timestamp, RTMPType& type, char* buffer, int maxlen)
{
	if (buffer == NULL || maxlen <= 0) return 0;

	int size = 0;;
	char vtype = SRS_RTMP_TYPE_AUDIO;
	char* data = NULL;
	uint32_t timestamp;

	if (srs_rtmp_read_packet(internal->rmp, &vtype, &timestamp, &data, &size) != 0) {
		return false;
	}

	if (srs_human_format_rtmp_packet(buffer, maxlen - 1, type, timestamp, data, size) != 0) {
		free(data);
		return false;
	}
	free(data);

	if (vtype == SRS_RTMP_TYPE_AUDIO) type = FlvTagAudio;
	else if (vtype == SRS_RTMP_TYPE_VIDEO) type = FlvTagVideo;
	else type = FlvTagScript;

	return strlen(buffer);
}


}
}
