#include "rtcpAnalyzer.h"
#include <string.h>
#ifndef WIN32
#include <arpa/inet.h>
#endif


RtcpUnpack::RtcpUnpack(void)
{
	m_nSsrc = -1;
}

RtcpUnpack::~RtcpUnpack(void)
{
}

int RtcpUnpack::UnPackRtcpPacket(char* rtcpBuf, int nBufLen, LPRTCPHEADER lpRtcpHeader)
{
	if (NULL == lpRtcpHeader)
	{
		return -1;
	}

	if (nBufLen < (int)sizeof(RTCPHEADER))
		return -1;

	RTCPHEADER *rtcphdr = (RTCPHEADER *)rtcpBuf;

	if (rtcphdr->version != 2)
	{
		return -1;
	}

	switch (rtcphdr->packettype)
	{
	case 200://SR
		{
			int i;
			int num = rtcphdr->count;

			unsigned int length = (size_t)ntohs(rtcphdr->length);
			length++;
			length *= sizeof(unsigned int);

			unsigned int *ssrcptr = (unsigned int *)(rtcpBuf+sizeof(RTCPHEADER));
			unsigned int nSsrc = ntohl(*ssrcptr);
			m_nSsrc = nSsrc;

			for (i = 0 ; i <= num ; i++)
			{
				int i = num;
				i++;
			}
		}

		break;
	case 201://RR
		{
			break;
		}

		//...
	default:
		{
			break;
		}
	}

	return 0;
}

int RtcpUnpack::CreateRR(char* pBuffer, int& nBufLen)
{
	RTCPHEADER stHeader;
	memset(&stHeader, 0, sizeof(RTCPHEADER));

	RTCPReceiverReport stRecvReport;
	memset(&stRecvReport, 0, sizeof(RTCPReceiverReport));

	stHeader.version = 2;
	stHeader.packettype = 201;
	stHeader.padding = 0;
	stHeader.count = 1;
	stHeader.length = (uint16_t)htonl(32);

	if (-1 == (int)m_nSsrc)
	{
		return -1;
	}
	stRecvReport.ssrc = htonl(m_nSsrc);
	//stRecvReport
	if (NULL == pBuffer)
	{
		return -1;
	}
	// 	stRecvReport.exthighseqnr = 0;
	// 	stRecvReport.i

	int nHeaderLen = sizeof(RTCPHEADER);
	memcpy(pBuffer, &stHeader, nHeaderLen);

	int nRRLen = sizeof(RTCPReceiverReport);
	memcpy(pBuffer + nHeaderLen, &stRecvReport, nRRLen);

	nBufLen = nHeaderLen + nRRLen;

	return 0;
}
//
//int RtcpUnpack::CreateSDES(char* pBuffer, int& nBufLen)
//{
//	RTCPHEADER stHeader;
//	memset(&stHeader, 0, sizeof(RTCPHEADER));
//
//	SDESHEADER stSdes;
//	memset(&stSdes, 0, sizeof(SDESHEADER));
//
//	stHeader.version = 2;
//	stHeader.packettype = 202;
//	stHeader.padding = 0;
//	stHeader.count = 1;
//	stHeader.length = htons(32);
//
//	if (-1 == m_nSsrc)
//	{
//		return -1;
//	}
//	//stRecvReport
//	if (NULL == pBuffer)
//	{
//		return -1;
//	}
//	// 	stRecvReport.exthighseqnr = 0;
//	// 	stRecvReport.i
//	//stSdes.type = 2;
//	//strcpy(stSdes.szdes, "XUNMEI_RTSP");
//	//stSdes.lenth = htonl(32);
//
//	int nHeaderLen = sizeof(RTCPHEADER);
//	memcpy(pBuffer, &stHeader, nHeaderLen);
//
//	int nRRLen = sizeof(SDESHEADER);
//	memcpy(pBuffer + nHeaderLen, &stSdes, nRRLen);
//
//	nBufLen = nHeaderLen + nRRLen;
//
//	return 0;
//}
