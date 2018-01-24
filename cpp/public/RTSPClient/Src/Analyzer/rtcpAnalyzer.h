#pragma once
#include "Base/Base.h"
using namespace Xunmei::Base;

#define RTCP_VERSION							 2

#define RTCPTYPE_SR							200
#define RTCPTYPE_RR							201
#define RTCPTYPE_SDES						202
#define RTCPTYPE_BYE						203
#define RTCPTYPE_APP						204

#define RTCP_SDES_ID_CNAME						1
#define RTCP_SDES_ID_NAME						2
#define RTCP_SDES_ID_EMAIL						3
#define RTCP_SDES_ID_PHONE						4
#define RTCP_SDES_ID_LOCATION					5
#define RTCP_SDES_ID_TOOL						6
#define RTCP_SDES_ID_NOTE						7
#define RTCP_SDES_ID_PRIVATE					8
#define RTCP_SDES_NUMITEMS_NONPRIVATE			7
#define RTCP_SDES_MAXITEMLENGTH					255

#define RTCP_BYE_MAXREASONLENGTH				255
#define RTCP_DEFAULTMININTERVAL					5.0	
#define RTCP_DEFAULTBANDWIDTHFRACTION			0.05
#define RTCP_DEFAULTSENDERFRACTION				0.25
#define RTCP_DEFAULTHALFATSTARTUP				true
#define RTCP_DEFAULTIMMEDIATEBYE				true
#define RTCP_DEFAULTSRBYE						true

typedef struct _RTCPHEADER
{
	uint8_t count:5; /* varies by packet type */ 
	uint8_t padding:1;      /* padding flag */ 
	uint8_t version:2; /* protocol version */ 

	uint8_t packettype;    /* RTCP packet type */ 
	uint16_t length; /* pkt len in words, w/o this word */
}RTCPHEADER, *LPRTCPHEADER;

typedef struct RTPSourceIdentifier
{
	uint32_t ssrc;
}RTPSOURCEIDENTIFIER;


typedef struct RTCPSenderReport
{
	uint32_t ntptime_msw;
	uint32_t ntptime_lsw;
	uint32_t rtptimestamp;
	uint32_t packetcount;
	uint32_t octetcount;
}RTCPSENDERREPORT;

typedef struct RTCPReceiverReport
{
	uint32_t ssrc; // Identifies about which SSRC's data this report is...
	uint8_t fractionlost;
	uint8_t packetslost[3];
	uint32_t exthighseqnr;
	uint32_t jitter;
	uint32_t lsr;
	uint32_t dlsr;
}RTCPRECEIVERREPORT;

typedef struct RTCPSDESHeader
{
	uint8_t sdesid;
	uint8_t length;
}RTCPSDESHEADER;


class RtcpUnpack
{
public:
	RtcpUnpack(void);
	~RtcpUnpack(void);

	int UnPackRtcpPacket(char* rtcpBuf, int nBufLen,  LPRTCPHEADER lpRtcpHeader);
	int CreateRR(char* pBuffer, int& nBufLen);
	//int CreateSDES(char* pBuffer, int& nBufLen);

	unsigned int m_nSsrc;
};
