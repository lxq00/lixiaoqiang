#pragma once

#include "irtpAnalyzer.h"

#define JPEG_HEADER_LEN		8

class RtpJpegAnalyzer : public IRtpAnalyzer
{
public:
	RtpJpegAnalyzer(int nVideoPayload, int nAudioPayload);
	~RtpJpegAnalyzer(void);

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

private:
	RTPHEADER		m_stRtpHeader;
	JPEGHEADER		m_stJpegHeader;
	uint64_t		m_dwUser;
	void*			m_lpUser;
	CBFreamCallBack m_pFramCallBack;

	char			m_pVideoBuf[VIEOD_FRAME_LEN];
	char			m_pAudioBuf[AUDIO_FRAME_LEN];
	int				m_nVideoBufLen;
	int				m_nAudioBufLen;

	int				m_nVideoPayload;
	int				m_nAudioPayload;
	int				m_nLastSeq;
	int				m_nFreamType;
	int				m_bFirstSeq;
	
};
