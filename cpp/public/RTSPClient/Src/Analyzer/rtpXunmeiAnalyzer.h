#pragma once

#include "irtpAnalyzer.h"

#define H264_STATRTCODE_LEN	4

class RtpXunmeiAnalyzer : public IRtpAnalyzer
{
public:
	RtpXunmeiAnalyzer(int nVideoPayload, int nAudioPayload);
	~RtpXunmeiAnalyzer(void);

	//virtual
	int Init(int nBufferSize);
	int InputData(char *pBuf, unsigned short nBufSize, int nOffset) ;
	int SetFreamDataCallBack(CBFreamCallBack pCallBack, uint32_t dwUserData1, void* lpUserData2);
	int ResetBuffer();

private:
	int  OnAnalyze(char* pBuf, int nBufLen);
	int  AnalyzeVideo(char* pBuf, int nBufLen);
	int  AnalyzeAudio(char* pBuf, int nBufLen);
	int  SetH264StartCode(char* pBuf, int nOffset);

private:
	RTPHEADER		m_stRtpHeader;
	FU_INDICATOR	m_stFuIndictor;
	FU_HEADER		m_stFuHeader;

	uint32_t		m_dwUser;
	void*			m_lpUser;
	CBFreamCallBack m_pFramCallBack; 

	char			m_pSpsBuf[MIDDLE_BUFFER_LEN];
	char			m_pPpsBuf[SHORT_BUFFER_LEN];
	int				m_nSpsLen;
	int				m_nPpsLen;

	char			m_pVideoBuf[VIEOD_FRAME_LEN];
	char			m_pAudioBuf[AUDIO_FRAME_LEN];
	int				m_nVideoBufLen;
	int				m_nAudioBufLen;

	int				m_nVideoPayload;
	int				m_nAudioPayload;
	int				m_nLastSeq;
	int				m_nFreamType;
	int				m_bFirstSeq;

	long			m_lFreamTimestamp; 

	bool			m_bThrowPack;
	bool			m_bIsRecvingIDRFream;
	bool			m_bIsWaitingIDRFream;

	bool			m_bIDRframeFlag;
};
