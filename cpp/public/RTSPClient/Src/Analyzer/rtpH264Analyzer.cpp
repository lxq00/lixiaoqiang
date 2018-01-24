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
	//缓存机制,暂未实现
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
		int nRet = AnalyzeVideo(pPayLoad, nPayLoadLen);
		return nRet;
	}

	//音频荷载
	if (m_stRtpHeader.pt == m_nAudioPayload)
	{
		int nRet = AnalyzeAudio(pPayLoad, nPayLoadLen);
		return nRet;
	}
	
	return 0;
}

int  RtpH264Analyzer::AnalyzeVideo(char* pBuf, int nBufLen)
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
	}
	//65535表示seq循环的最后一个,下一个时为0,出现这种情况是正常的,不是序列混乱
	else if (m_nLastSeq + 1 > nCurrentSeq && m_nLastSeq != 65535)
	{
		//待处理
	}

	m_nLastSeq = nCurrentSeq;

	//get nalu header
	memcpy(&m_stFuIndictor, pBuf, 1);

	/*RFC3984 5.2  RTP荷载格式的公共结构
	  注释:本规范没有限制封装在单个NAL单元包和分片包的大小。封装在聚合包中的NAL单元大小为65535字节
	  单个NAL单元包: 荷载中只包含一个NAL单元。NAL头类型域等于原始NAL单元类型,即在范围1~23之间
	*/
	if ( m_stFuIndictor.TYPE >0 &&  m_stFuIndictor.TYPE < 24)
	{
		char *pVideoBuffer = NULL;
		int nVideoBufferSize = 0;
		if (m_stFuIndictor.TYPE != 5)
		{
			nVideoBufferSize = H264_STATRTCODE_LEN + nBufLen;
			pVideoBuffer = new char[nVideoBufferSize];
			//单个个NAL包数据,首先设置帧数据起始码
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
		case 0://未定义
			break;
		case 1://非IDR帧图像不采用数据划分片段
			m_nFreamType = VIDEO_P_FRAME_TYPE;
//			m_pFramCallBack(m_pVideoBuf, m_nVideoBufLen, m_nFreamType, ntohl(m_stRtpHeader.ts), (uint64_t)m_dwUser, m_lpUser);
			break;
		case 2://非IDR帧图像中A类数据划分片段
			break;
		case 3://非IDR帧图像中B类数据划分片段
			break;
		case 4://非IDR帧图像中C类数据划分片段
			break;
		case 5://IDR帧的图像片段
			m_nFreamType = VIDEO_I_FRAME_TYPE;
//			m_pFramCallBack(m_pVideoBuf, m_nVideoBufLen, m_nFreamType, ntohl(m_stRtpHeader.ts), (uint64_t)m_dwUser, m_lpUser);
			break;
		case 6://补充增强信息(SEI)
			break;
		case 7://序列参数集(SPS)
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
		case 8://图像参数集(PPS)
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
		case 9://分隔符
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
	/*RFC3984 5.2  RTP荷载格式的公共结构
	  聚合包: 本类型用于聚合多个NAL单元到单个RTP荷载中。本包有4种版本,单时间聚合包类型A(STAP-A)
	  单时间聚合包类型B(STAP-B),多时间聚合包类型(MTAP)16位位移(MTAP16), 多时间聚合包类型(MTAP)24位位移(MTAP24)
	  赋予 STAP-A, STAP-B, MTAP16, MTAP24的NAL单元类型号分别是24, 25, 26, 27。
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
	/*RFC3984 5.2  RTP荷载格式的公共结构
	  分片包: 用于分片单个NAL单元到多个RTP包, 现存在两个版本FU-A, FU-B, 用NAL单元类型28, 29标识。
	*/
	else if (28 == m_stFuIndictor.TYPE)//FU_A
	{
		int NALType = pBuf[1] & 0x1f;
		memset(&m_stNalHeader, 0 ,sizeof(m_stNalHeader));
		memcpy(&m_stNalHeader, pBuf + 1, 1);//FU_HEADER赋值

		//收到帧分片包的第一包数据
		if (1 == m_stNalHeader.S) 
		{
			//如果这时为丢包状态,那么停止丢包
			m_bThrowPack = false;
			//检测IDR帧
			//5 == m_stFuHeader.TYPE 标准判断方法
			//7 == m_stFuHeader.TYPE 为了兼容三星SNP3370
			m_nFreamType = VIDEO_P_FRAME_TYPE;
			if (5 == m_stNalHeader.TYPE || 7 == m_stNalHeader.TYPE)
			{
				m_nFreamType = VIDEO_I_FRAME_TYPE;
				m_bIsWaitingIDRFream = false;
				m_bIsRecvingIDRFream = true; //正在接收IDR帧

				//IDR帧前写入sps和pps
				memcpy(m_pVideoBuf + m_nVideoBufLen, m_pSpsBuffer, m_nSpsLen);
				m_nVideoBufLen += m_nSpsLen;
				memcpy(m_pVideoBuf + m_nVideoBufLen, m_pPpsBuffer, m_nPpsLen);
				m_nVideoBufLen += m_nPpsLen;
			}

			//设置帧数据的起始码
			SetH264StartCode(m_pVideoBuf, m_nVideoBufLen);
			m_nVideoBufLen += H264_STATRTCODE_LEN;
			
			pBuf[1] = ( pBuf[0] & 0xE0 ) | NALType ;

			memcpy(m_pVideoBuf + m_nVideoBufLen, &pBuf[1], 1);
			m_nVideoBufLen += 1;

			if (m_nVideoBufLen + nBufLen - 2 > VIEOD_FRAME_LEN)
			{
				return -1;
			}
			//拷贝荷载数据,跳过前面的IDR帧检验位和FU_HEADER两个字节
			memcpy(m_pVideoBuf + m_nVideoBufLen, pBuf + 2, nBufLen - 2);
			m_nVideoBufLen += nBufLen - 2;
		}
		//帧分片包的最后一包数据,为了更强的兼容性放宽限制
		//我们认为FU_HEADER的E字段和rtsp头得mark位符合其一都是有效的
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
			//如果不是帧的第一包数据,而且处于丢帧状态,那么收到的所有包都丢弃
			if (true == m_bThrowPack || true == m_bIsWaitingIDRFream) 
			{
				return -1;
			}

			if (m_nVideoBufLen + nBufLen - 2 > VIEOD_FRAME_LEN)
			{
				return -1;
			}
			//正常情况,拷贝数据
			memcpy(m_pVideoBuf + m_nVideoBufLen, pBuf + 2, nBufLen - 2);
			m_nVideoBufLen += nBufLen - 2;
		}
	}
	else if (29 == m_stFuIndictor.TYPE)//FU-B
	{
	}
	else//30-31 未定义
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
	int nFrameType = AUDIO_FRAME_TYPE;//音频数据帧类型
	
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