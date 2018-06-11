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
	//�������,��δʵ��
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

	//����RTPͷ����
	memset(&m_stRtpHeader, 0, sizeof(RTPHEADER));
	memcpy(&m_stRtpHeader, pBuf, RTP_HEADER_LEN);

	//�����Ƿ���ȷ(ͨ��RTPͷ�汾��)
	if (m_stRtpHeader.v != RTP_VERSION)
	{
		return -1;
	}

	//��Ƶ����
	if (m_stRtpHeader.pt == m_nVideoPayload)
	{
		return AnalyzeVideo(pPayLoad, nPayLoadLen);
	}

	//��Ƶ����
	if (m_stRtpHeader.pt == m_nAudioPayload)
	{
		return AnalyzeAudio(pPayLoad, nPayLoadLen);
	}

	return 0;
}

int  RtpXunmeiAnalyzer::AnalyzeVideo(char* pBuf, int nBufLen)
{
	//����Ƿ��ж���
	int nCurrentSeq = ntohs(m_stRtpHeader.seq);

	//��һ��������ʱ�򲻽����ж϶���,��ʱ�� m_nLastSeq Ϊ��ʼ״̬,���㶪��
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
	//65535��ʾseqѭ�������һ��,��һ��ʱΪ0,�������������������,�������л���
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
		//������NAL������,��������֡������ʼ��
		SetH264StartCode(m_pVideoBuf, 0);
		memcpy(m_pVideoBuf + H264_STATRTCODE_LEN, pBuf, nBufLen);
		m_nVideoBufLen = nBufLen + H264_STATRTCODE_LEN;

		switch (m_stFuIndictor.TYPE)
		{
		case 0://δ����
			break;
		case 1://��IDR֡ͼ�񲻲������ݻ���Ƭ��
			m_nFreamType = VIDEO_P_FRAME_TYPE;
			m_pFramCallBack(m_pVideoBuf, m_nVideoBufLen, m_nFreamType, m_stRtpHeader.ts, m_dwUser, m_lpUser);
			break;
		case 2://��IDR֡ͼ����A�����ݻ���Ƭ��
			break;
		case 3://��IDR֡ͼ����B�����ݻ���Ƭ��
			break;
		case 4://��IDR֡ͼ����C�����ݻ���Ƭ��
			break;
		case 5://IDR֡��ͼ��Ƭ��
			m_nFreamType = VIDEO_I_FRAME_TYPE;
			m_pFramCallBack(m_pVideoBuf, m_nVideoBufLen, m_nFreamType, m_stRtpHeader.ts, m_dwUser, m_lpUser);
			break;
		case 6://������ǿ��Ϣ(SEI)
			break;
		case 7://���в�����(SPS)
			m_nFreamType = H264_SPS_FRAME_TYPE;
			m_pFramCallBack(m_pVideoBuf, m_nVideoBufLen, m_nFreamType, m_stRtpHeader.ts, m_dwUser, m_lpUser);
			break;
		case 8://ͼ�������(PPS)
			m_nFreamType = H264_PPS_FRAME_TYPE;
			m_pFramCallBack(m_pVideoBuf, m_nVideoBufLen, m_nFreamType, m_stRtpHeader.ts, m_dwUser, m_lpUser);
			break;
		case 9://�ָ���
			if(0x30 == pBuf[1])//P֡
			{
				m_bIDRframeFlag = false;
			}
			if(0x10 == pBuf[1])//IDR֡
			{
				m_bIDRframeFlag = true;
			}
			break;
		case 10://���н�����
			break;
		case 11://��������
			break;
		case 12://�������
			break;
		case 13://���в�������չ
			break;
		case 14://����
			break;
		case 15://����
			break;
		case 16://����
			break;
		case 17://����
			break;
		case 18://����
			break;
		case 19://δ�ָ�ĸ�������ͼ��Ƭ��
			break;
		case 20://����
			break;
		case 21://����
			break;
		case 22://����
			break;
		case 23://����
			break;
		default://δ����
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
		memcpy(&m_stFuHeader, pBuf + 1, 1);//FU_HEADER��ֵ

		//�յ�֡��Ƭ���ĵ�һ������
		if (1 == m_stFuHeader.S) 
		{
			//�����ʱΪ����״̬,��ôֹͣ����
			m_bThrowPack = false;

			//�����Ƭ���ĵ�һ��ʱ���,��Ϊ֡��ʱ���
			m_lFreamTimestamp = m_stRtpHeader.ts;

			SetH264StartCode(m_pVideoBuf, 0);
			pBuf[1] = ( pBuf[0] & 0xE0 ) | NALType ;

			memcpy(m_pVideoBuf + H264_STATRTCODE_LEN, &pBuf[1], 1);
			m_nVideoBufLen += H264_STATRTCODE_LEN;
			m_nVideoBufLen += 1;
			//char szLogBuf[100] = {0};
			//sprintf(szLogBuf, "----m_stFuHeader.TYPE=%d----\n", m_stFuHeader.TYPE);
			//OutputDebugString(szLogBuf);
			//���IDR֡
			if (5 == m_stFuHeader.TYPE || m_bIDRframeFlag)
			{
				m_nFreamType = VIDEO_I_FRAME_TYPE;
				m_bIsWaitingIDRFream = false;
				m_bIsRecvingIDRFream = true; //���ڽ���IDR֡
			}
			else
			{
				m_nFreamType = VIDEO_P_FRAME_TYPE;
			}

			//������������,����ǰ���IDR֡����λ��FU_HEADER�����ֽ�
			memcpy(m_pVideoBuf + m_nVideoBufLen, pBuf + 2, nBufLen - 2);
			m_nVideoBufLen += nBufLen - 2;
		}
		//֡��Ƭ�������һ������,Ϊ�˸�ǿ�ļ����Էſ�����
		//������ΪFU_HEADER��E�ֶκ�rtspͷ��markλ������һ������Ч��
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
			//�������֡�ĵ�һ������,���Ҵ��ڶ�֡״̬,��ô�յ������а�������
			if (true == m_bThrowPack || true == m_bIsWaitingIDRFream) 
			{
				return -1;
			}

			//�������,��������
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
	int nFrameType = AUDIO_FRAME_TYPE;//��Ƶ����֡����
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
