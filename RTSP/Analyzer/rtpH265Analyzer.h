#pragma once

#include "irtpAnalyzer.h"
#include "h265frame.h"
#include "Base/Base.h"

class RtpH265Analyzer : public IRtpAnalyzer
{
public:
	RtpH265Analyzer(const CBFreamCallBack& callback);
	~RtpH265Analyzer(void);

	//virtual
	virtual int InputData(const RTPHEADER& rtpheader, const char* pBuf, unsigned short nBufLen);
private:
	shared_ptr<H265Frame> pHevcFrame;
	CBFreamCallBack		 m_pFramCallBack;
};
