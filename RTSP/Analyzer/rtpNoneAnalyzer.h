#pragma once

#include "irtpAnalyzer.h"

#define JPEG_HEADER_LEN		8

class RtpNoneAnalyzer : public IRtpAnalyzer
{
public:
	RtpNoneAnalyzer(const CBFreamCallBack& callback, FrameType type);
	~RtpNoneAnalyzer(void);

	int InputData(const RTPHEADER& rtpheader, const char* pBuf, unsigned short nBufLen);
private:
	CBFreamCallBack m_pFramCallBack;

	char*			m_pVideoBuf;
	int				m_nVideoBufLen;
	
	FrameType		m_nFreamType;
	uint16_t		m_nLastSeq;
	bool			m_lossstatus;
};
