#include "rtpJpegAnalyzer.h"


RtpJpegAnalyzer::RtpJpegAnalyzer(int nVideoPayload, int nAudioPayload)
{
	memset(m_pVideoBuf,		0, VIEOD_FRAME_LEN);
	memset(m_pAudioBuf,		0, AUDIO_FRAME_LEN);
	memset(&m_stRtpHeader,	0, sizeof(RTPHEADER));
	memset(&m_stJpegHeader,	0, sizeof(JPEGHEADER));

	m_nVideoBufLen			= 0;
	m_nAudioBufLen		    = 0;
	m_nFreamType			= 0;

	m_bFirstSeq				= -2;
	m_nLastSeq				= -2;

	m_nVideoPayload			= nVideoPayload;
	m_nAudioPayload			= nAudioPayload;
}

RtpJpegAnalyzer::~RtpJpegAnalyzer(void)
{
}

int RtpJpegAnalyzer::Init(int nBufferSize)
{
	//缓存机制,暂未实现
	return 0;
}

int RtpJpegAnalyzer::InputData(char *pBuf, unsigned short nBufSize, int nOffset)
{
	OnAnalyze(pBuf, nBufSize);

	return 0;
}

int RtpJpegAnalyzer::SetFreamDataCallBack(CBFreamCallBack pCallBack, uint64_t dwUserData1, void* lpUserData2)
{
	if (NULL == pCallBack)
	{
		return -1;
	}

	m_pFramCallBack = pCallBack;
	m_dwUser		= dwUserData1;
	m_lpUser		= lpUserData2;

	return 0;
}

int RtpJpegAnalyzer::ResetBuffer()
{
	return 0;
}

int RtpJpegAnalyzer::OnAnalyze(char* pBuf, int nBufLen)
{
	if(NULL == pBuf || nBufLen <= RTP_HEADER_LEN )
	{
		return -1;
	}

	char *pPayLoad = pBuf + RTP_HEADER_LEN;
	int nPayLoadLen			= nBufLen - RTP_HEADER_LEN;

	//拷贝RTP头数据
	memset(&m_stRtpHeader, 0, sizeof(RTPHEADER));
	memcpy(&m_stRtpHeader, pBuf, RTP_HEADER_LEN);

	//检验是否正确(通过RTP头版本号)
	if (m_stRtpHeader.v != RTP_VERSION)
	{
		return -1;
	}

	//视频荷载
	if (m_stRtpHeader.pt == m_nVideoPayload)
	{
		return AnalyzeVideo(pPayLoad, nPayLoadLen);
	}

	//音频荷载
	if (m_stRtpHeader.pt == m_nAudioPayload)
	{
		return AnalyzeAudio(pPayLoad, nPayLoadLen);
	}

	return 0;
}

int RtpJpegAnalyzer::AnalyzeVideo(char* pBuf, int nBufLen)
{
	//分析8个字节的JPEG头
	memcpy(&m_stJpegHeader, pBuf, JPEG_HEADER_LEN);
	
	long lOffset = ntohl(m_stJpegHeader.offset);
	memcpy(m_pVideoBuf, pBuf + JPEG_HEADER_LEN, nBufLen - JPEG_HEADER_LEN);
	//rfc2435说明 MJPEG帧结束必须通过rtp头的mark位判断
	if (1 == m_stRtpHeader.m)
	{
		m_nVideoBufLen = lOffset + nBufLen - JPEG_HEADER_LEN;
		m_pFramCallBack(m_pVideoBuf, m_nVideoBufLen, 5, m_stRtpHeader.seq, (uint64_t)m_dwUser, m_lpUser);
		memset(m_pVideoBuf, 0, VIEOD_FRAME_LEN);
		m_nVideoBufLen = 0;
	}

	return 0;
}

int RtpJpegAnalyzer::AnalyzeAudio(char* pBuf, int nBufLen)
{
	memcpy(m_pAudioBuf, pBuf, nBufLen);
	m_nAudioBufLen = nBufLen;
	int nFrameType = AUDIO_FRAME_TYPE;//音频数据帧类型
	m_pFramCallBack(m_pAudioBuf, m_nAudioBufLen, nFrameType, m_stRtpHeader.ts, (uint64_t)m_dwUser, m_lpUser);

	memset(m_pAudioBuf, 0, AUDIO_FRAME_LEN);
	m_nVideoBufLen = 0;
	return 0;
}

void RtpJpegAnalyzer::Free()
{

}