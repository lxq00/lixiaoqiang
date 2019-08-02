#pragma  once

#include "Base/Base.h"
#include "RTSP/RTSP.h"
using namespace Public::RTSP;
using namespace Public::Base;

int rtsp_header_parse_transport(const char* field, TRANSPORT_INFO* t);

std::string rtsp_header_build_transport(const TRANSPORT_INFO& transport);

