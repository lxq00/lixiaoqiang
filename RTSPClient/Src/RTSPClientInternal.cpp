#include "RTSPClientInternal.h"
#include "RTSPCodeTarns.h"

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//xunmei heart beat
static unsigned char normaltheartbeat[] = {0x24 ,0x01 ,0x00 ,0x34 ,0x81 ,0xC9 ,0x00 ,0x07 ,0x80 ,0x00 ,0x00 ,0x29 ,0x1A ,0xC3 ,0x79 ,0x75   
,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x5B ,0x9F ,0x00 ,0x00 ,0x07 ,0x7C ,0x80 ,0x7A ,0x30 ,0xCE 
,0x00 ,0x00 ,0xAD ,0x79 ,0x81 ,0xCA ,0x00 ,0x04 ,0x80 ,0x00 ,0x00 ,0x29 ,0x01 ,0x09 ,0x62 ,0x6C 
,0x75 ,0x65 ,0x73 ,0x74 ,0x6F ,0x72 ,0x6D ,0x00};

// static char normaltheartbeat[] = {0x24, 0x01, 0x00, 0x34, 0x81, 0xc9, 0x00, 0x07, 0x27, 0xf7, 0x0a, 0x8d, 0x04, 0xfe, 0x32, 0x09
// ,0x00, 0xff, 0xff, 0xff, 0x00, 0x01, 0x99, 0x6a, 0x00, 0x00, 0x03, 0x90, 0x0a, 0xda, 0x00, 0x00
// ,0x00, 0x00, 0x23, 0x91, 0x81, 0xca, 0x00, 0x04, 0x27, 0xf7, 0x0a, 0x8d, 0x01, 0x07, 0x52, 0x41
// ,0x4e, 0x44, 0x4f, 0x4e, 0x47, 0x00, 0x00, 0x00};

static unsigned char firstheartbeat[] = {0x24,0x01,0x00,0x1C,0x80 ,0xC9 ,0x00 ,0x01  ,0x80 ,0x00 ,0x00 ,0x29 ,0x81 ,0xCA ,0x00 ,0x04
,0x80 ,0x00 ,0x00 ,0x29 ,0x01 ,0x09 ,0x62 ,0x6C  ,0x75 ,0x65 ,0x73 ,0x74 ,0x6F ,0x72 ,0x6D,0x00};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


#define MAXRECVBUFFERSIZE		65*1024
#define MAXCTRLBUFFERSIZE		10240
#define HEADTBEATTIMEOUT		40000


RTSPClient::RTSPClientInternal::RTSPClientInternal(const std::string& rtspUrl):RTSPProtocol(rtspUrl)
{
	m_lPrevVideoTs = -1;
	m_lVideoTsDiff = 0;
	m_lCurrentVideoTs = 0;
	m_nFrameIndex = 0;
	m_nFrameRate = 0;
}

RTSPClient::RTSPClientInternal::~RTSPClientInternal()
{
	stop();
}

bool RTSPClient::RTSPClientInternal::start(const shared_ptr<IOWorker>& worker, uint32_t timeout, uint32_t startUdpport)
{
	RTSPProtocol::m_ioworker = worker;

	if (RTSPProtocol::m_nRecvType == RTP_RECV_TYPE_UDP)
	{
		RTSPProtocol::m_udpRecvPort[0] = startUdpport;
		RTSPProtocol::m_udpRecvPort[1] = startUdpport+1;
		RTSPProtocol::m_udpRecvPort[2] = startUdpport+2;
		RTSPProtocol::m_udpRecvPort[3] = startUdpport+3;

		m_videoUDPRecv = UDP::create(m_ioworker);
		m_videoUDPRecv->bind(RTSPProtocol::m_udpRecvPort[0]);
		m_videoUDPRecv->async_recvfrom(Socket::RecvFromCallback(&RTSPClientInternal::onVideoUDPRecvCallback, this), MAXRECVBUFFERSIZE);

		m_videoCtrlUDPRecv = UDP::create(m_ioworker);
		m_videoCtrlUDPRecv->bind(RTSPProtocol::m_udpRecvPort[1]);
		m_videoCtrlUDPRecv->async_recvfrom(Socket::RecvFromCallback(&RTSPClientInternal::onVideoCtrlUDPRecvCallback, this), MAXRECVBUFFERSIZE);

		m_audioUDPRecv = UDP::create(m_ioworker);
		m_audioUDPRecv->bind(RTSPProtocol::m_udpRecvPort[2]);
		m_audioUDPRecv->async_recvfrom(Socket::RecvFromCallback(&RTSPClientInternal::onAudioUDPRecvCallback, this), MAXRECVBUFFERSIZE);

		m_audioCtrlUDPRecv = UDP::create(m_ioworker);
		m_audioCtrlUDPRecv->bind(RTSPProtocol::m_udpRecvPort[3]);
		m_audioCtrlUDPRecv->async_recvfrom(Socket::RecvFromCallback(&RTSPClientInternal::onAudioCtrlUDPRecvCallback, this), MAXRECVBUFFERSIZE);
	}

	RTSPProtocol::m_dataCallback = RTSPProtocol::MediaDataCallback(&RTSPClientInternal::onVideoAndAudioTCPRecvCallback,this);
	RTSPProtocol::m_statusCallback = RTSPProtocol::StatusCallback(&RTSPClientInternal::onProtocolStatusCallback, this);
	RTSPProtocol::m_protocolCallback = RTSPProtocol::ProtocolCallback(&RTSPClientInternal::onProtocolCallback, this);

	if (!RTSPProtocol::startProtocol(timeout))
	{
		return false;
	}

	m_prevHeatBeatTime = Time::getCurrentMilliSecond();
	m_firstHeatbeat = true;

	m_poolTimerProc = make_shared<Timer>("RTSPClient::RTSPClientInternal");
	m_poolTimerProc->start(Timer::Proc(&RTSPClientInternal::onTimerProc, this), 0, 1000);

	return true;
}
bool RTSPClient::RTSPClientInternal::stop()
{
	RTSPProtocol::stopProcol();

	m_poolTimerProc = NULL;
	m_tcpsocket = NULL;

	m_videoUDPRecv = NULL;
	m_videoCtrlUDPRecv = NULL;
	m_audioUDPRecv = NULL;
	m_audioCtrlUDPRecv = NULL;

	m_pRtpAnalyzer = NULL;

	return true;
}
void RTSPClient::RTSPClientInternal::onTimerProc(unsigned long)
{
	uint64_t nowtime = Time::getCurrentMilliSecond();

	if (nowtime < m_prevHeatBeatTime || nowtime - m_prevHeatBeatTime >= HEADTBEATTIMEOUT)
	{
		if (RTP_RECV_TYPE_UDP == m_nRecvType)
		{
			RTSPProtocol::addHeatbeat();
		}
		else
		{
			char buffer[1024] = {0};
			uint32_t bufferlen = 0;
			if (m_firstHeatbeat)
			{
				bufferlen = sizeof(firstheartbeat);
				memcpy(buffer, firstheartbeat, bufferlen);
			}
			else
			{
				bufferlen = sizeof(normaltheartbeat);
				memcpy(buffer, normaltheartbeat, bufferlen);
			}
			buffer[1] = RTSPProtocol::m_videoSdp.channel;

			shared_ptr<Socket> tcp = RTSPProtocol::m_tcpsocket;
			if (tcp != NULL)
			{
				tcp->send(buffer, bufferlen);
			}
		}
		m_firstHeatbeat = false;
		m_prevHeatBeatTime = nowtime;
	}

	RTSPProtocol::onTimerProc();
}
int RTSPClient::RTSPClientInternal::creatRTPAnalyzer()
{
	if (0 == strncasecmp("H264", m_stMediaInfo.stStreamVideo.szCodec, 4))
	{
		m_pRtpAnalyzer = make_shared<RtpH264Analyzer>(m_stMediaInfo.stStreamVideo.nPayLoad, m_stMediaInfo.stStreamAudio.nPayLoad, (char*)m_szSpsBuffer, m_nSpsLen, (char*)m_szPpsBuffer, m_nPpsLen);
		//*ppRtpAnalyzer = new RtpXunmeiAnalyzer(m_stMediaInfo.stStreamVideo.nPayLoad, m_stMediaInfo.stStreamAudio.nPayLoad);

	}
	//不区分大小写比较4个字节的编码类型
	else if (0 == strncasecmp("JPEG", m_stMediaInfo.stStreamVideo.szCodec, 4))
	{
		m_pRtpAnalyzer = make_shared<RtpJpegAnalyzer>(m_stMediaInfo.stStreamVideo.nPayLoad, m_stMediaInfo.stStreamAudio.nPayLoad);
	}
	else
	{
		switch (m_stMediaInfo.stStreamVideo.nPayLoad)
		{
		case VIDEO_JPEG_90000:
			m_pRtpAnalyzer = make_shared<RtpJpegAnalyzer>(m_stMediaInfo.stStreamVideo.nPayLoad, m_stMediaInfo.stStreamAudio.nPayLoad);
			break;
		case VIDEO_H264_90000:
			m_pRtpAnalyzer = make_shared<RtpH264Analyzer>(m_stMediaInfo.stStreamVideo.nPayLoad, m_stMediaInfo.stStreamAudio.nPayLoad, (char*)m_szSpsBuffer, m_nSpsLen, (char*)m_szPpsBuffer, m_nPpsLen);
			break;
		default:
			break;
		}
	}

	if (m_pRtpAnalyzer != NULL)
	{
		m_pRtpAnalyzer->Init(0);

		m_pRtpAnalyzer->SetFreamDataCallBack(onFreamCallBack, (uint64_t)this, this);
	}

	return m_pRtpAnalyzer != NULL;
}
void RTSPClient::RTSPClientInternal::onVideoAndAudioTCPRecvCallback(const char* buffer, int len, bool isvideo)
{
	m_rtpDataCallback(isvideo, buffer, len , m_userData);

	if (m_pRtpAnalyzer == NULL)
	{
		creatRTPAnalyzer();
	}
	if (m_pRtpAnalyzer == NULL)
	{
		return;
	}

	m_pRtpAnalyzer->InputData((char*)buffer, len, 0);
}

void RTSPClient::RTSPClientInternal::onProtocolStatusCallback(RTSPStatus_t status, const std::string& errorinfo)
{
	if (status == RTSPStatus_ConnectSuccess)
	{
		MEDIA_PARAMETER param;
		converRTSPCode(&param);

		m_mediaCallback(param,m_userData);
	}

	if (m_statusCallback != NULL)
	{
		m_statusCallback(status, m_userData);
	}
}

void RTSPClient::RTSPClientInternal::onProtocolCallback(bool sender, const char* buffer, uint32_t len)
{
	m_protocolCallback(sender, buffer, len, m_userData);
}

void RTSPClient::RTSPClientInternal::onVideoUDPRecvCallback(const weak_ptr<Socket>& sock, const char* buffer, int len, const NetAddr& addr)
{
	onVideoAndAudioTCPRecvCallback(buffer, len, true);
	m_videoUDPRecv->async_recvfrom(Socket::RecvFromCallback(&RTSPClientInternal::onVideoUDPRecvCallback, this), MAXRECVBUFFERSIZE);
}
void RTSPClient::RTSPClientInternal::onVideoCtrlUDPRecvCallback(const weak_ptr<Socket>& sock, const char* buffer, int len, const NetAddr& addr)
{
	m_videoCtrlUDPRecv->async_recvfrom(Socket::RecvFromCallback(&RTSPClientInternal::onVideoCtrlUDPRecvCallback, this), MAXRECVBUFFERSIZE);
}
void RTSPClient::RTSPClientInternal::onAudioUDPRecvCallback(const weak_ptr<Socket>& sock, const char* buffer, int len, const NetAddr& addr)
{
	onVideoAndAudioTCPRecvCallback(buffer, len, false);
	m_audioUDPRecv->async_recvfrom(Socket::RecvFromCallback(&RTSPClientInternal::onAudioUDPRecvCallback, this), MAXRECVBUFFERSIZE);
}
void RTSPClient::RTSPClientInternal::onAudioCtrlUDPRecvCallback(const weak_ptr<Socket>& sock, const char* buffer, int len, const NetAddr& addr)
{
	m_audioCtrlUDPRecv->async_recvfrom(Socket::RecvFromCallback(&RTSPClientInternal::onAudioCtrlUDPRecvCallback, this), MAXRECVBUFFERSIZE);
}

void RTSPClient::RTSPClientInternal::onFreamCallBack(char *pFrameBuf, int nFrameSize, int nFrameType, long lTimestamp, uint64_t dwUser, void* lpUser)
{
	RTSPClientInternal* internal = (RTSPClientInternal*)lpUser;
	if (internal == NULL || internal->m_dataCallback == NULL) return;

	//如果还没有注册回调,那么将数据都丢弃,不进行分析回调
	FRAME_INFO stFramInfo;
	memset(&stFramInfo, 0, sizeof(stFramInfo));

	switch (nFrameType)
	{
	case 0://H264 IDR帧
		internal->m_bFirstIFream = true;
		break;
	case 1: //p帧
		break;
	case 2://音频帧
		break;
	case 3://H264 sps
		if (nFrameSize <= MIDDLE_BUFFER_LEN)
		{
			memcpy(internal->m_szSpsBuffer, pFrameBuf, nFrameSize);
			internal->m_nSpsLen = nFrameSize;
		}
		return;
	case 4://H264 pps
		if (nFrameSize <= MIDDLE_BUFFER_LEN)
		{
			memcpy(internal->m_szPpsBuffer, pFrameBuf, nFrameSize);
			internal->m_nPpsLen = nFrameSize;
		}
		return;
	case 5://MJPEG
		internal->setFrameInfo(0, nFrameSize, internal->m_stMediaInfo.stStreamVideo.nWidth, internal->m_stMediaInfo.stStreamVideo.nHight, lTimestamp, stFramInfo);
		internal->m_dataCallback(pFrameBuf, nFrameSize, &stFramInfo, internal->m_userData);
		return;
	}
	
	//如果还没有收到第一个I帧, 那么所有数据都丢弃
	if (!internal->m_bFirstIFream)
	{
		return;
	}

	if (!internal->m_bSendHeader)
	{
		char *pSpsPpsBuffer = new char[internal->m_nSpsLen + internal->m_nPpsLen];
		memcpy(pSpsPpsBuffer, internal->m_szSpsBuffer, internal->m_nSpsLen);
		memcpy(pSpsPpsBuffer + internal->m_nSpsLen, internal->m_szPpsBuffer, internal->m_nPpsLen);
		internal->m_bSendHeader = true;

		internal->setFrameInfo(3, internal->m_nPpsLen + internal->m_nSpsLen, 0, 0, 0, stFramInfo);
		internal->m_dataCallback(pSpsPpsBuffer, internal->m_nPpsLen + internal->m_nSpsLen, &stFramInfo, internal->m_userData);
		delete []pSpsPpsBuffer;
		pSpsPpsBuffer = NULL;
	}

	internal->setFrameInfo(nFrameType, nFrameSize, internal->m_stMediaInfo.stStreamVideo.nWidth, internal->m_stMediaInfo.stStreamVideo.nHight, lTimestamp, stFramInfo);
	internal->m_dataCallback(pFrameBuf, nFrameSize, &stFramInfo, internal->m_userData);
}

void RTSPClient::RTSPClientInternal::setFrameInfo(int nFrameType, int nFrameSize, int nWidth, int nHight, long lTimeStamp, FRAME_INFO& stFrameInfo)
{
	memset(&stFrameInfo, 0, sizeof(FRAME_INFO));

	if (0 == nFrameType || 1 == nFrameType)
	{
		if (-1 == m_lPrevVideoTs)
		{
			m_lPrevVideoTs = lTimeStamp;
		}
		m_lVideoTsDiff = lTimeStamp - m_lPrevVideoTs;
		m_lPrevVideoTs = lTimeStamp;
		m_lCurrentVideoTs += m_lVideoTsDiff * 1000 / m_stMediaInfo.stStreamVideo.nSampRate;
		stFrameInfo.lFrameTimestamp = m_lCurrentVideoTs;
	}
#ifdef WIN32
	SYSTEMTIME stSysTime;
	memset(&stSysTime, 0, sizeof(SYSTEMTIME));
	GetLocalTime(&stSysTime);

	stFrameInfo.nYear = stSysTime.wYear;
	stFrameInfo.nMonth = stSysTime.wMonth;
	stFrameInfo.nDay = stSysTime.wDay;

	stFrameInfo.nHour = stSysTime.wHour;
	stFrameInfo.nMinute = stSysTime.wMinute;
	stFrameInfo.nSecond = stSysTime.wSecond;
#else
	struct tm *t;
	time_t tt;
	time(&tt);
	t = localtime(&tt);
	stFrameInfo.nYear = t->tm_year + 1990;
	stFrameInfo.nMonth = t->tm_mon + 1;
	stFrameInfo.nDay = t->tm_mday;
	stFrameInfo.nHour = t->tm_hour;
	stFrameInfo.nMinute = t->tm_min;
	stFrameInfo.nSecond = t->tm_sec;
#endif
	stFrameInfo.nFrameNum = ++m_nFrameIndex;
	stFrameInfo.nFramesize = nFrameSize;
	stFrameInfo.nFrameRate = m_nFrameRate;

	stFrameInfo.nFrameType = nFrameType;

	stFrameInfo.nFrameWidth = nWidth;
	stFrameInfo.nFrameHeight = nHight;
}

void RTSPClient::RTSPClientInternal::converRTSPCode(LPMEDIA_PARAMETER lpMdeiaParam)
{
	if (lpMdeiaParam == NULL)
	{
		return ;
	}
	lpMdeiaParam->bHasVideo = m_stMediaInfo.bHasVideo;
	lpMdeiaParam->bHasAudio = m_stMediaInfo.bHasAudio;
	lpMdeiaParam->nVideoRtpPayLoad = m_stMediaInfo.stStreamVideo.nPayLoad;
	lpMdeiaParam->nAudioRtpPayLoad = m_stMediaInfo.stStreamAudio.nPayLoad;
	lpMdeiaParam->nVideoSampRate = m_stMediaInfo.stStreamVideo.nSampRate;
	lpMdeiaParam->nVideoWidth = m_stMediaInfo.stStreamVideo.nWidth;
	lpMdeiaParam->nVideoHight = m_stMediaInfo.stStreamVideo.nHight;
	lpMdeiaParam->fVideoFrameRate = (float)m_nFrameRate;//m_stMediaInfo.stStreamVideo.fFramRate;

	GetVideoCodec(m_stMediaInfo.stStreamAudio.nPayLoad, m_stMediaInfo.stStreamVideo.szCodec, lpMdeiaParam->nVideoXMDecodID);
	GetAudioCodec(m_stMediaInfo.stStreamAudio.nPayLoad, m_stMediaInfo.stStreamAudio.szCodec, lpMdeiaParam->nAudioXMDecodID, lpMdeiaParam->nAudioChannels, lpMdeiaParam->nAudioSampRate = m_stMediaInfo.stStreamAudio.nSampRate);
	lpMdeiaParam->nAudioChannels = m_stMediaInfo.stStreamAudio.nChannles;
	lpMdeiaParam->nAudioSampRate = m_stMediaInfo.stStreamAudio.nSampRate;
}