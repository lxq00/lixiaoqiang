#pragma once

#include "irtpAnalyzer.h"

#define H264_STATRTCODE_LEN	4

class RtpH264Analyzer : public IRtpAnalyzer
{
public:
	RtpH264Analyzer(int nVideoPayload, int nAudioPayload, char* pSps, int nSpsLen, char* pPps, int nPpsLen);
	~RtpH264Analyzer(void);

	//virtual
	int Init(int nBufferSize);
	int InputData(char *pBuf, unsigned short nBufSize, int nOffset) ;
	int SetFreamDataCallBack(CBFreamCallBack pCallBack, uint64_t dwUserData1, void* lpUserData2);
	int ResetBuffer();
	void Free();
	
private:
	int  OnAnalyze(char* pBuf, int nBufLen);
	int  AnalyzeVideo(char* pBuf, int nBufLen);
	int  AnalyzeAudio(char* pBuf, int nBufLen);
	int  SetH264StartCode(char* pBuf, int nOffset);
	
private:
	RTPHEADER		m_stRtpHeader;
	FU_INDICATOR	m_stFuIndictor;
	FU_HEADER		m_stNalHeader;

	uint64_t		m_dwUser;
	void*			m_lpUser;
	CBFreamCallBack m_pFramCallBack; 

	char*			m_pSpsBuffer;
	char*			m_pPpsBuffer;
	int				m_nSpsLen;
	int				m_nPpsLen;

	char			*m_pVideoBuf;
	char			*m_pAudioBuf;
	int				m_nVideoBufLen;
	int				m_nAudioBufLen;

	int				m_nVideoPayload;
	int				m_nAudioPayload;
	int				m_nLastSeq;
	int				m_nFreamType;
	int				m_bFirstSeq;

	bool			m_bThrowPack;
	bool			m_bIsRecvingIDRFream;
	bool			m_bIsWaitingIDRFream;
};
