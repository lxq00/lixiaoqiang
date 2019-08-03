#include "rtpH265Analyzer.h"
#include <string.h>
#include <stddef.h>
#include <stdio.h>
#ifdef WIN32
#include <WinSock.h>
#else
#include <sys/socket.h>
#include <arpa/inet.h>
#endif

RtpH265Analyzer::RtpH265Analyzer(const CBFreamCallBack& callback)
	: m_pFramCallBack(callback)
{
	pHevcFrame = make_shared<H265Frame>();
	m_pFramCallBack = callback;
}

RtpH265Analyzer::~RtpH265Analyzer()
{

}
int RtpH265Analyzer::InputData(const RTPHEADER& rtpheader, const char* pBuf, unsigned short nBufSize)
{
	int iGoodFrame = pHevcFrame->handleHevcFrame(ntohs(rtpheader.seq), ntohl(rtpheader.ts), (const uint8_t*)pBuf, nBufSize);
	if (iGoodFrame <= 0)
	{
		return 0;
	}

	m_pFramCallBack(FrameType_VIDEO_FRAME,(char*)pHevcFrame->m_buf, pHevcFrame->m_buflen, ntohl(rtpheader.ts));
	
	return 0;
}
