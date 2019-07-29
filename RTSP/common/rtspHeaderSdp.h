#ifndef _SDP_PARSE_H_
#define _SDP_PARSE_H_
#include "Base/Base.h"
#include "RTSP/RTSP.h"
using namespace Public::RTSP;
using namespace Public::Base;


std::string rtsp_header_build_sdp(const MEDIA_INFO& info);
bool rtsp_header_parse_sdp(char const* sdpDescription, MEDIA_INFO* pMediaInfo);

#endif
