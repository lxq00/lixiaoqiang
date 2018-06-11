#include "rtpXunmeiAnalyzer.h"
#ifndef WIN32
#include <arpa/inet.h>
#endif
RtpXunmeiAnalyzer::RtpXunmeiAnalyzer(int nVideoPayload, int nAudioPayload)
{
	memset(m_pVideoBuf,		0, VIEOD_FRAME_LEN);
	memset(m_pAudioBuf,		0, AUDIO_FRAME_LEN);
	memset(m_pSpsBuf,		0, MIDDLE_BUFFER_LEN);
	memset(m_pPpsBuf,		0, SHORT_BUFFER_LEN);
	memset(&m_stRtpHeader,	0, sizeof(RTPHEADER));
	memset(&m_stFuIndictor, 0, sizeof(FU_INDICATOR));
	memset(&m_stFuHeader,	0, sizeof(FU_HEADER));

	m_nSpsLen				= 0;
	m_nPpsLen				= 0;
	m_nVideoBufLen			= 0;
	m_nAudioBufLen		    = 0;
	m_nFreamType			= 0;

	m_lFreamTimestamp		= 0; 
	m_bFirstSeq				= -2;
	m_nLastSeq				= -2;

	m_bThrowPack			= false;
	m_bIsRecvingIDRFream	= false;
	m_bIsWaitingIDRFream	= false;
	m_bIDRframeFlag			= false;

	m_nVideoPayload			= nVideoPayload;
	m_nAudioPayload			= nAudioPayload;
}

RtpXunmeiAnalyzer::~RtpXunmeiAnalyzer()
{
}

int RtpXunmeiAnalyzer::Init(int nBufferSize)
{
	//缓存机制,暂未实现
	return 0;
}

int RtpXunmeiAnalyzer::InputData(char *pBuf, unsigned short nBufSize, int nOffset)
{
	OnAnalyze(pBuf, nBufSize);

	return 0;
}

int RtpXunmeiAnalyzer::SetFreamDataCallBack(CBFreamCallBack pCallBack, uint32_t dwUserData1, void* lpUserData2)
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

int RtpXunmeiAnalyzer::ResetBuffer()
{
	return 0;
}

int RtpXunmeiAnalyzer::OnAnalyze(char* pBuf, int nBufLen)
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

int  RtpXunmeiAnalyzer::AnalyzeVideo(char* pBuf, int nBufLen)
{
	//检查是否有丢包
	int nCurrentSeq = ntohs(m_stRtpHeader.seq);

	//第一次来包的时候不进行判断丢包,此时的 m_nLastSeq 为初始状态,不算丢包
	if (m_nLastSeq + 1 < nCurrentSeq && m_nLastSeq != m_bFirstSeq)
	{
		m_bThrowPack   = true;
		if (true == m_bIsRecvingIDRFream)
		{
			m_bIsWaitingIDRFream = true;
		}

		char szLogBuffer[256] = {0};
		sprintf(szLogBuffer, "======== lost pack ======= pre seq:%d  cur seq:%d\r\n", m_nLastSeq, nCurrentSeq);
#ifdef WIN32
		OutputDebugString(szLogBuffer);
#else
		printf(szLogBuffer);
#endif
	}
	//65535表示seq循环的最后一个,下一个时为0,出现这种情况是正常的,不是序列混乱
	else if (m_nLastSeq + 1 > nCurrentSeq && m_nLastSeq != 65535)
	{
		char szLogBuffer[256] = {0};
		sprintf(szLogBuffer, "******** error queue ******** pre seq:%d  cur seq:%d\r\n", m_nLastSeq, nCurrentSeq);
#ifdef WIN32
		OutputDebugString(szLogBuffer);
#else
		printf(szLogBuffer);
#endif
	}

	m_nLastSeq = nCurrentSeq;

	//get nalu header
	memcpy(&m_stFuIndictor, pBuf, 1);

	if ( m_stFuIndictor.TYPE >0 &&  m_stFuIndictor.TYPE < 24)
	{
		//单个个NAL包数据,首先设置帧数据起始码
		SetH264StartCode(m_pVideoBuf, 0);
		memcpy(m_pVideoBuf + H264_STATRTCODE_LEN, pBuf, nBufLen);
		m_nVideoBufLen = nBufLen + H264_STATRTCODE_LEN;

		switch (m_stFuIndictor.TYPE)
		{
		case 0://未定义
			break;
		case 1://非IDR帧图像不采用数据划分片段
			m_nFreamType = VIDEO_P_FRAME_TYPE;
			m_pFramCallBack(m_pVideoBuf, m_nVideoBufLen, m_nFreamType, m_stRtpHeader.ts, m_dwUser, m_lpUser);
			break;
		case 2://非IDR帧图像中A类数据划分片段
			break;
		case 3://非IDR帧图像中B类数据划分片段
			break;
		case 4://非IDR帧图像中C类数据划分片段
			break;
		case 5://IDR帧的图像片段
			m_nFreamType = VIDEO_I_FRAME_TYPE;
			m_pFramCallBack(m_pVideoBuf, m_nVideoBufLen, m_nFreamType, m_stRtpHeader.ts, m_dwUser, m_lpUser);
			break;
		case 6://补充增强信息(SEI)
			break;
		case 7://序列参数集(SPS)
			m_nFreamType = H264_SPS_FRAME_TYPE;
			m_pFramCallBack(m_pVideoBuf, m_nVideoBufLen, m_nFreamType, m_stRtpHeader.ts, m_dwUser, m_lpUser);
			break;
		case 8://图像参数集(PPS)
			m_nFreamType = H264_PPS_FRAME_TYPE;
			m_pFramCallBack(m_pVideoBuf, m_nVideoBufLen, m_nFreamType, m_stRtpHeader.ts, m_dwUser, m_lpUser);
			break;
		case 9://分隔符
			if(0x30 == pBuf[1])//P帧
			{
				m_bIDRframeFlag = false;
			}
			if(0x10 == pBuf[1])//IDR帧
			{
				m_bIDRframeFlag = true;
			}
			break;
		case 10://序列结束符
			break;
		case 11://流结束符
			break;
		case 12://填充数据
			break;
		case 13://序列参数集扩展
			break;
		case 14://保留
			break;
		case 15://保留
			break;
		case 16://保留
			break;
		case 17://保留
			break;
		case 18://保留
			break;
		case 19://未分割的辅助编码图像片段
			break;
		case 20://保留
			break;
		case 21://保留
			break;
		case 22://保留
			break;
		case 23://保留
			break;
		default://未定义
			break;
		}

		memset(m_pVideoBuf, 0, VIEOD_FRAME_LEN);
		m_nVideoBufLen = 0;
		return 0;
	}
	else if (24 == m_stFuIndictor.TYPE)//STAP-A
	{
	}
	else if (25 == m_stFuIndictor.TYPE)//STAP-B
	{
	}
	else if (26 == m_stFuIndictor.TYPE)//MTAP16
	{
	}
	else if (27 == m_stFuIndictor.TYPE)//MTAP24
	{
	}
	else if (28 == m_stFuIndictor.TYPE)//FU_A
	{
		int NALType = pBuf[1] & 0x1f;
		memset(&m_stFuHeader, 0 ,sizeof(m_stFuHeader));
		memcpy(&m_stFuHeader, pBuf + 1, 1);//FU_HEADER赋值

		//收到帧分片包的第一包数据
		if (1 == m_stFuHeader.S) 
		{
			//如果这时为丢包状态,那么停止丢包
			m_bThrowPack = false;

			//保存分片包的第一包时间戳,作为帧的时间戳
			m_lFreamTimestamp = m_stRtpHeader.ts;

			SetH264StartCode(m_pVideoBuf, 0);
			pBuf[1] = ( pBuf[0] & 0xE0 ) | NALType ;

			memcpy(m_pVideoBuf + H264_STATRTCODE_LEN, &pBuf[1], 1);
			m_nVideoBufLen += H264_STATRTCODE_LEN;
			m_nVideoBufLen += 1;
			//char szLogBuf[100] = {0};
			//sprintf(szLogBuf, "----m_stFuHeader.TYPE=%d----\n", m_stFuHeader.TYPE);
			//OutputDebugString(szLogBuf);
			//检测IDR帧
			if (5 == m_stFuHeader.TYPE || m_bIDRframeFlag)
			{
				m_nFreamType = VIDEO_I_FRAME_TYPE;
				m_bIsWaitingIDRFream = false;
				m_bIsRecvingIDRFream = true; //正在接收IDR帧
			}
			else
			{
				m_nFreamType = VIDEO_P_FRAME_TYPE;
			}

			//拷贝荷载数据,跳过前面的IDR帧检验位和FU_HEADER两个字节
			memcpy(m_pVideoBuf + m_nVideoBufLen, pBuf + 2, nBufLen - 2);
			m_nVideoBufLen += nBufLen - 2;
		}
		//帧分片包的最后一包数据,为了更强的兼容性放宽限制
		//我们认为FU_HEADER的E字段和rtsp头得mark位符合其一都是有效的
		else if (1 == m_stRtpHeader.m || 1 == m_stFuHeader.E)
		{
			if (true == m_bThrowPack || true == m_bIsWaitingIDRFream)
			{
				return -1;
			}

			memcpy(m_pVideoBuf + m_nVideoBufLen, pBuf + 2, nBufLen - 2);
			m_nVideoBufLen += nBufLen - 2;

			m_pFramCallBack(m_pVideoBuf, m_nVideoBufLen, m_nFreamType, m_lFreamTimestamp, m_dwUser, m_lpUser);

			memset(m_pVideoBuf, 0, VIEOD_FRAME_LEN);
			m_nVideoBufLen = 0;
			return 0;
		}
		else
		{
			//如果不是帧的第一包数据,而且处于丢帧状态,那么收到的所有包都丢弃
			if (true == m_bThrowPack || true == m_bIsWaitingIDRFream) 
			{
				return -1;
			}

			//正常情况,拷贝数据
			memcpy(m_pVideoBuf + m_nVideoBufLen, pBuf + 2, nBufLen - 2);
			m_nVideoBufLen += nBufLen - 2;
		}
	}

	return 0;
}

int  RtpXunmeiAnalyzer::AnalyzeAudio(char* pBuf, int nBufLen)
{
	memcpy(m_pAudioBuf, pBuf, nBufLen);
	m_nAudioBufLen = nBufLen;
	int nFrameType = AUDIO_FRAME_TYPE;//音频数据帧类型
	m_pFramCallBack(m_pAudioBuf, m_nAudioBufLen, nFrameType, m_stRtpHeader.ts, m_dwUser, m_lpUser);

	memset(m_pAudioBuf, 0, AUDIO_FRAME_LEN);
	m_nVideoBufLen = 0;

	return 0;
}

int RtpXunmeiAnalyzer::SetH264StartCode(char* pBuf, int nOffset)
{
	if (NULL == pBuf)
	{
		return -1;
	}

	memset(pBuf + nOffset, 0x00, 1);
	memset(pBuf + nOffset + 1, 0x00, 1);
	memset(pBuf + nOffset + 2, 0x00, 1);
	memset(pBuf + nOffset + 3, 0x01, 1);

	return 0;
}
