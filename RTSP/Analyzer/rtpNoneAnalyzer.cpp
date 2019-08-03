#include "rtpNoneAnalyzer.h"


RtpNoneAnalyzer::RtpNoneAnalyzer(const CBFreamCallBack& callback, FrameType type)
{
	m_pFramCallBack = callback;

	m_pVideoBuf = new char[VIEOD_FRAME_LEN];

	m_nVideoBufLen			= 0;
	m_nFreamType			= type;

	m_nLastSeq = 0;
	m_lossstatus = false;
}

RtpNoneAnalyzer::~RtpNoneAnalyzer(void)
{
	delete[] m_pVideoBuf;
}
int RtpNoneAnalyzer::InputData(const RTPHEADER& m_stRtpHeader, const char* pBuf, unsigned short nBufLen)
{
	//遇到mark将丢包状态修改，下一个包开始组
	if (m_lossstatus && m_stRtpHeader.m)
	{
		m_lossstatus = false;
		return 0;
	}

	//数据丢包
	if (m_nLastSeq != 0 && (uint16_t)(m_nLastSeq + 1) != ntohs(m_stRtpHeader.seq))
	{
		assert(0);

		m_nLastSeq = 0;
		m_nVideoBufLen = 0;
		m_lossstatus = true;

		return 0;
	}
	m_nLastSeq = ntohs(m_stRtpHeader.seq);

	if (m_nVideoBufLen + nBufLen > VIEOD_FRAME_LEN)
	{
		assert(0);
		return -1;
	}
	
	if (m_nVideoBufLen == 0 && m_stRtpHeader.m)
	{
		m_pFramCallBack(m_nFreamType, pBuf, nBufLen, ntohl(m_stRtpHeader.ts));
	}
	else
	{
		memcpy(m_pVideoBuf + m_nVideoBufLen, pBuf, nBufLen);
		m_nVideoBufLen += nBufLen;

		if (m_stRtpHeader.m)
		{
			m_pFramCallBack(m_nFreamType, m_pVideoBuf, m_nVideoBufLen, ntohl(m_stRtpHeader.ts));

			m_nVideoBufLen = 0;
		}
	}

	return 0;
}
