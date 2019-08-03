#pragma once

#include "irtpAnalyzer.h"

#define H264_STATRTCODE_LEN	4

class RtpH264Analyzer : public IRtpAnalyzer
{
public:
	RtpH264Analyzer(const CBFreamCallBack& callback,const std::string& pSps, const std::string& pPps);
	~RtpH264Analyzer(void);

	virtual int InputData(const RTPHEADER& rtpheader, const char* pBuf, unsigned short nBufLen);
	
private:
	int  SetH264StartCode(char* pBuf, int nOffset);
	
private:
	FU_INDICATOR	m_stFuIndictor;
	FU_HEADER		m_stNalHeader;

	CBFreamCallBack m_pFramCallBack; 

	char*			m_pSpsBuffer;
	char*			m_pPpsBuffer;
	int				m_nSpsLen;
	int				m_nPpsLen;

	char			*m_pVideoBuf;
	int				m_nVideoBufLen;
	
	int				m_nLastSeq;
	FrameType		m_nFreamType;
	int				m_bFirstSeq;

	bool			m_bThrowPack;
	bool			m_bIsRecvingIDRFream;
	bool			m_bIsWaitingIDRFream;
};
