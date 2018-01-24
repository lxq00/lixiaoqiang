#include "rtpH264Analyzer.h"


RtpH264Analyzer::RtpH264Analyzer(int nVideoPayload, int nAudioPayload, char* pSps, int nSpsLen, char* pPps, int nPpsLen)
{
	m_pVideoBuf = new char[VIEOD_FRAME_LEN];
	m_pAudioBuf = new char[AUDIO_FRAME_LEN];
	memset(m_pVideoBuf,		0, VIEOD_FRAME_LEN);
	memset(m_pAudioBuf,		0, AUDIO_FRAME_LEN);

	memset(&m_stRtpHeader,	0, sizeof(RTPHEADER));
	memset(&m_stFuIndictor, 0, sizeof(FU_INDICATOR));
	memset(&m_stNalHeader,	0, sizeof(FU_HEADER));

	m_nSpsLen				= 0;
	m_nPpsLen				= 0;
	m_nVideoBufLen			= 0;
	m_nAudioBufLen		    = 0;
	m_nFreamType			= -1;

	m_bFirstSeq				= -2;
	m_nLastSeq				= -2;

	m_bThrowPack			= false;
	m_bIsRecvingIDRFream	= false;
	m_bIsWaitingIDRFream	= false;

	m_nVideoPayload			= nVideoPayload;
	m_nAudioPayload			= nAudioPayload;
	m_pSpsBuffer			= new char[nSpsLen];
	m_pPpsBuffer			= new char[nPpsLen];


	m_nSpsLen				= nSpsLen;
	m_nPpsLen				= nPpsLen;
	memcpy(m_pSpsBuffer, pSps, nSpsLen);
	memcpy(m_pPpsBuffer, pPps, nPpsLen);
}

RtpH264Analyzer::~RtpH264Analyzer()
{
	if (m_pSpsBuffer != NULL)
	{
		delete[] m_pSpsBuffer;
		m_pSpsBuffer = NULL;
	}

	if (m_pPpsBuffer = NULL)
	{
		delete[] m_pPpsBuffer;
		m_pPpsBuffer = NULL;
	}
}

int RtpH264Analyzer::Init(int nBufferSize)
{
	//�������,��δʵ��
	return 0;
}

int RtpH264Analyzer::InputData(char *pBuf, unsigned short nBufSize, int nOffset)
{
	OnAnalyze(pBuf, nBufSize);

	return 0;
}

int RtpH264Analyzer::SetFreamDataCallBack(CBFreamCallBack pCallBack, uint64_t dwUserData1, void* lpUserData2)
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

int RtpH264Analyzer::ResetBuffer()
{
	return 0;
}

int RtpH264Analyzer::OnAnalyze(char* pBuf, int nBufLen)
{
	if(NULL == pBuf || nBufLen <= RTP_HEADER_LEN )
	{
		return -1;
	}

	char *pPayLoad  = pBuf + RTP_HEADER_LEN;
	int nPayLoadLen	= nBufLen - RTP_HEADER_LEN;

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
		int nRet = AnalyzeVideo(pPayLoad, nPayLoadLen);
		return nRet;
	}

	//��Ƶ����
	if (m_stRtpHeader.pt == m_nAudioPayload)
	{
		int nRet = AnalyzeAudio(pPayLoad, nPayLoadLen);
		return nRet;
	}
	
	return 0;
}

int  RtpH264Analyzer::AnalyzeVideo(char* pBuf, int nBufLen)
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
	}
	//65535��ʾseqѭ�������һ��,��һ��ʱΪ0,�������������������,�������л���
	else if (m_nLastSeq + 1 > nCurrentSeq && m_nLastSeq != 65535)
	{
		//������
	}

	m_nLastSeq = nCurrentSeq;

	//get nalu header
	memcpy(&m_stFuIndictor, pBuf, 1);

	/*RFC3984 5.2  RTP���ظ�ʽ�Ĺ����ṹ
	  ע��:���淶û�����Ʒ�װ�ڵ���NAL��Ԫ���ͷ�Ƭ���Ĵ�С����װ�ھۺϰ��е�NAL��Ԫ��СΪ65535�ֽ�
	  ����NAL��Ԫ��: ������ֻ����һ��NAL��Ԫ��NALͷ���������ԭʼNAL��Ԫ����,���ڷ�Χ1~23֮��
	*/
	if ( m_stFuIndictor.TYPE >0 &&  m_stFuIndictor.TYPE < 24)
	{
		char *pVideoBuffer = NULL;
		int nVideoBufferSize = 0;
		if (m_stFuIndictor.TYPE != 5)
		{
			nVideoBufferSize = H264_STATRTCODE_LEN + nBufLen;
			pVideoBuffer = new char[nVideoBufferSize];
			//������NAL������,��������֡������ʼ��
			SetH264StartCode(pVideoBuffer, 0);
			memcpy(pVideoBuffer + H264_STATRTCODE_LEN, pBuf, nBufLen);
		}
		else
		{
			nVideoBufferSize = H264_STATRTCODE_LEN + m_nSpsLen + m_nPpsLen + nBufLen;
			pVideoBuffer = new char[nVideoBufferSize];
			int offset = 0;
			memcpy(pVideoBuffer + offset, m_pSpsBuffer, m_nSpsLen);
			offset += m_nSpsLen;
			memcpy(pVideoBuffer + offset, m_pPpsBuffer, m_nPpsLen);
			offset += m_nPpsLen;
			SetH264StartCode(pVideoBuffer + offset, 0);
			offset += H264_STATRTCODE_LEN;
			memcpy(pVideoBuffer + offset, pBuf, nBufLen);
		}
		m_nFreamType = VIDEO_P_FRAME_TYPE;
		switch (m_stFuIndictor.TYPE)
		{
		case 0://δ����
			break;
		case 1://��IDR֡ͼ�񲻲������ݻ���Ƭ��
			m_nFreamType = VIDEO_P_FRAME_TYPE;
//			m_pFramCallBack(m_pVideoBuf, m_nVideoBufLen, m_nFreamType, ntohl(m_stRtpHeader.ts), (uint64_t)m_dwUser, m_lpUser);
			break;
		case 2://��IDR֡ͼ����A�����ݻ���Ƭ��
			break;
		case 3://��IDR֡ͼ����B�����ݻ���Ƭ��
			break;
		case 4://��IDR֡ͼ����C�����ݻ���Ƭ��
			break;
		case 5://IDR֡��ͼ��Ƭ��
			m_nFreamType = VIDEO_I_FRAME_TYPE;
//			m_pFramCallBack(m_pVideoBuf, m_nVideoBufLen, m_nFreamType, ntohl(m_stRtpHeader.ts), (uint64_t)m_dwUser, m_lpUser);
			break;
		case 6://������ǿ��Ϣ(SEI)
			break;
		case 7://���в�����(SPS)
			m_nFreamType = H264_SPS_FRAME_TYPE;
			if (m_pSpsBuffer != NULL)
			{
				delete[] m_pSpsBuffer;
				m_pSpsBuffer = NULL;
			}
			m_pSpsBuffer = pVideoBuffer;
			m_nSpsLen = nVideoBufferSize;
//			m_pFramCallBack(m_pVideoBuf, m_nVideoBufLen, m_nFreamType, ntohl(m_stRtpHeader.ts), (uint64_t)m_dwUser, m_lpUser);
			break;
		case 8://ͼ�������(PPS)
			m_nFreamType = H264_PPS_FRAME_TYPE;
			if (m_pPpsBuffer != NULL)
			{
				delete[] m_pPpsBuffer;
				m_pPpsBuffer = NULL;
			}
			m_pPpsBuffer = pVideoBuffer;
			m_nPpsLen = nVideoBufferSize;
//			m_pFramCallBack(m_pVideoBuf, m_nVideoBufLen, m_nFreamType, ntohl(m_stRtpHeader.ts), (uint64_t)m_dwUser, m_lpUser);
			break;
		case 9://�ָ���
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
		if (pVideoBuffer == NULL)
		{
			return 0;
		}
		m_pFramCallBack(pVideoBuffer, nVideoBufferSize, m_nFreamType, ntohl(m_stRtpHeader.ts), (uint64_t)m_dwUser, m_lpUser);
		if (pVideoBuffer != m_pSpsBuffer && pVideoBuffer != m_pPpsBuffer)
		{
			delete[]pVideoBuffer;
			pVideoBuffer = NULL;
		}
		return 0;
	}
	/*RFC3984 5.2  RTP���ظ�ʽ�Ĺ����ṹ
	  �ۺϰ�: ���������ھۺ϶��NAL��Ԫ������RTP�����С�������4�ְ汾,��ʱ��ۺϰ�����A(STAP-A)
	  ��ʱ��ۺϰ�����B(STAP-B),��ʱ��ۺϰ�����(MTAP)16λλ��(MTAP16), ��ʱ��ۺϰ�����(MTAP)24λλ��(MTAP24)
	  ���� STAP-A, STAP-B, MTAP16, MTAP24��NAL��Ԫ���ͺŷֱ���24, 25, 26, 27��
	*/
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
	/*RFC3984 5.2  RTP���ظ�ʽ�Ĺ����ṹ
	  ��Ƭ��: ���ڷ�Ƭ����NAL��Ԫ�����RTP��, �ִ��������汾FU-A, FU-B, ��NAL��Ԫ����28, 29��ʶ��
	*/
	else if (28 == m_stFuIndictor.TYPE)//FU_A
	{
		int NALType = pBuf[1] & 0x1f;
		memset(&m_stNalHeader, 0 ,sizeof(m_stNalHeader));
		memcpy(&m_stNalHeader, pBuf + 1, 1);//FU_HEADER��ֵ

		//�յ�֡��Ƭ���ĵ�һ������
		if (1 == m_stNalHeader.S) 
		{
			//�����ʱΪ����״̬,��ôֹͣ����
			m_bThrowPack = false;
			//���IDR֡
			//5 == m_stFuHeader.TYPE ��׼�жϷ���
			//7 == m_stFuHeader.TYPE Ϊ�˼�������SNP3370
			m_nFreamType = VIDEO_P_FRAME_TYPE;
			if (5 == m_stNalHeader.TYPE || 7 == m_stNalHeader.TYPE)
			{
				m_nFreamType = VIDEO_I_FRAME_TYPE;
				m_bIsWaitingIDRFream = false;
				m_bIsRecvingIDRFream = true; //���ڽ���IDR֡

				//IDR֡ǰд��sps��pps
				memcpy(m_pVideoBuf + m_nVideoBufLen, m_pSpsBuffer, m_nSpsLen);
				m_nVideoBufLen += m_nSpsLen;
				memcpy(m_pVideoBuf + m_nVideoBufLen, m_pPpsBuffer, m_nPpsLen);
				m_nVideoBufLen += m_nPpsLen;
			}

			//����֡���ݵ���ʼ��
			SetH264StartCode(m_pVideoBuf, m_nVideoBufLen);
			m_nVideoBufLen += H264_STATRTCODE_LEN;
			
			pBuf[1] = ( pBuf[0] & 0xE0 ) | NALType ;

			memcpy(m_pVideoBuf + m_nVideoBufLen, &pBuf[1], 1);
			m_nVideoBufLen += 1;

			if (m_nVideoBufLen + nBufLen - 2 > VIEOD_FRAME_LEN)
			{
				return -1;
			}
			//������������,����ǰ���IDR֡����λ��FU_HEADER�����ֽ�
			memcpy(m_pVideoBuf + m_nVideoBufLen, pBuf + 2, nBufLen - 2);
			m_nVideoBufLen += nBufLen - 2;
		}
		//֡��Ƭ�������һ������,Ϊ�˸�ǿ�ļ����Էſ�����
		//������ΪFU_HEADER��E�ֶκ�rtspͷ��markλ������һ������Ч��
		else if (1 == m_stRtpHeader.m || 1 == m_stNalHeader.E)
		{
			if (true == m_bThrowPack || true == m_bIsWaitingIDRFream)
			{
				return -1;
			}

			if (m_nVideoBufLen + nBufLen - 2 > VIEOD_FRAME_LEN)
			{
				return -1;
			}
			memcpy(m_pVideoBuf + m_nVideoBufLen, pBuf + 2, nBufLen - 2);
			m_nVideoBufLen += nBufLen - 2;

			m_pFramCallBack(m_pVideoBuf, m_nVideoBufLen, m_nFreamType, ntohl(m_stRtpHeader.ts), (uint64_t)m_dwUser, m_lpUser);

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

			if (m_nVideoBufLen + nBufLen - 2 > VIEOD_FRAME_LEN)
			{
				return -1;
			}
			//�������,��������
			memcpy(m_pVideoBuf + m_nVideoBufLen, pBuf + 2, nBufLen - 2);
			m_nVideoBufLen += nBufLen - 2;
		}
	}
	else if (29 == m_stFuIndictor.TYPE)//FU-B
	{
	}
	else//30-31 δ����
	{
	}

	return 0;
}

int  RtpH264Analyzer::AnalyzeAudio(char* pBuf, int nBufLen)
{
	if (nBufLen > AUDIO_FRAME_LEN || NULL == pBuf)
	{
		return -1;
	}
	memcpy(m_pAudioBuf, pBuf, nBufLen);
	m_nAudioBufLen = nBufLen;
	int nFrameType = AUDIO_FRAME_TYPE;//��Ƶ����֡����
	
	m_pFramCallBack(m_pAudioBuf, m_nAudioBufLen, nFrameType, ntohl(m_stRtpHeader.ts), (uint64_t)m_dwUser, m_lpUser);
	memset(m_pAudioBuf, 0, AUDIO_FRAME_LEN);
	m_nAudioBufLen = 0;

	return 0;
}

int RtpH264Analyzer::SetH264StartCode(char* pBuf, int nOffset)
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

void RtpH264Analyzer::Free()
{	
	if (NULL != m_pVideoBuf)
	{
		delete []m_pVideoBuf;
		m_pVideoBuf = NULL;
	}

	if (NULL != m_pAudioBuf)
	{
		delete []m_pAudioBuf;
		m_pAudioBuf = NULL;
	}
	
	if (NULL != m_pSpsBuffer)
	{
		delete []m_pSpsBuffer;
		m_pSpsBuffer = NULL;
	}

	if (NULL != m_pPpsBuffer)
	{
		delete []m_pPpsBuffer;
		m_pPpsBuffer = NULL;
	}

}