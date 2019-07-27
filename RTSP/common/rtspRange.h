#pragma  once
#include "RTSP/RTSPStructs.h"
#include "Base/Base.h"

using namespace Public::Base;

int rtsp_header_range(const char* field, MEDIA_INFO* range);
