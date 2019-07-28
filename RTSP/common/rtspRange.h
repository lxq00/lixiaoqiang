#pragma  once
#include "RTSP/RTSPStructs.h"
#include "Base/Base.h"

using namespace Public::Base;

int rtsp_header_range(const char* field, RANGE_INFO* range);
std::string rtspBuildRange(const RANGE_INFO& range);