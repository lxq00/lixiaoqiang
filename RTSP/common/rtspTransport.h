#ifndef _rtsp_header_transport_h_
#define _rtsp_header_transport_h_
#include "RTSP/RTSPStructs.h"


int rtsp_header_transport(const char* field, TRANSPORT_INFO* t);

std::string buildTransport(const TRANSPORT_INFO& transport);

#endif
