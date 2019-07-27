#pragma once
#ifndef _SDP_PARSE_H_
#define _SDP_PARSE_H_
#include "Base/Base.h"
using namespace Public::Base;

#include "RTSPClient/RTSPStructs.h"

std::string BuildSdp(const MEDIA_INFO& info);
bool ParseSDP(char const* sdpDescription, MEDIA_INFO* pMediaInfo);
bool parseSDPLine(char const* input, char const*& nextLine);
bool parseSDPLine_s(char const* sdpLine);
bool parseSDPLine_i(char const* sdpLine);
bool parseSDPLine_b(char const* sdpLine);
bool parseSDPLine_c(char const* sdpLine);
bool parseSDPAttribute_type(char const* sdpLine);
bool parseSDPAttribute_control(char const* sdpLine, char *pcontrol);
bool parseSDPAttribute_range(char const* sdpLine, char* pRange);
bool parseSDPAttribute_source_filter(char const* sdpLine);
bool parseSDPAttribute_rtpmap(char const* sdpLine, char* pCodecName, int &nPayLoad, int &nTimestampFrequency, int &nNumChannels);
bool parseSDPAttribute_framerate(char const* sdpLine, double &fVideoFPS);
bool parseSDPAttribute_fmtp(char const* sdpLine, char* pSpsBufer);
bool parseSDPAttribute_x_dimensions(char const* sdpLine, int& nWidth, int& nHeight);

//...........................................static.............................................

static char* lookupPayloadFormat(unsigned char rtpPayloadType, unsigned& rtpTimestampFrequency, unsigned& numChannels);
static unsigned guessRTPTimestampFrequency(char const* mediumName, char const* codecName);
static char* parseCLine(char const* sdpLine);
static bool parseRangeAttribute(char const* sdpLine, char* pRange);

bool ParseTrackID(char *sdp, MEDIA_INFO *sMediaInfo);

#endif
