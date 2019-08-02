#pragma  once
#include "RTSP/RTSPStructs.h"
#include "Base/Base.h"

using namespace Public::Base;

int rtsp_header_parse_range(const char* field, RANGE_INFO* range);
std::string rtsp_header_build_range(const RANGE_INFO& range);