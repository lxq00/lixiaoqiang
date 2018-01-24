
#pragma once
#include "RTSPClient/RTSPClient.h"
#include "Analyzer/rtcpAnalyzer.h"
#include "Analyzer/rtpH264Analyzer.h"
#include "Analyzer/rtpJpegAnalyzer.h"
#include "Analyzer/rtpXunmeiAnalyzer.h"
#include "RTSPClient/RTSPClientErrorCode.h"
#include "RTSP/RTSPProtocol.h"

using namespace Xunmei::RTSPClient;

struct RTSPClient::RTSPClientInternal:public RTSPProtocol
{
public:
	RTSPClientInternal(const std::string& rtspUrl);
	~RTSPClientInternal();

	bool start(const shared_ptr<IOWorker>& worker, uint32_t timeout,uint32_t startUdpport = 0);
	bool stop();

	void converRTSPCode(LPMEDIA_PARAMETER param);
private:
	shared_ptr<Socket>			m_videoUDPRecv;
	shared_ptr<Socket>			m_videoCtrlUDPRecv;
	shared_ptr<Socket>			m_audioUDPRecv;
	shared_ptr<Socket>			m_audioCtrlUDPRecv;
	shared_ptr<Timer>			m_poolTimerProc;
public:
	RTSPClientManager*			m_manager;
	RTSPClient_DataCallBack		m_dataCallback;
	RTSPClient_StatusCallback	m_statusCallback;
	RTSPClient_ProtocolCallback m_protocolCallback;
	RTSPClient_MediaCallback	m_mediaCallback;
	void*						m_userData;
private:
	shared_ptr<IRtpAnalyzer>	m_pRtpAnalyzer;
	char*						m_videoUDPRecvBuffer;
	char*						m_videoCtrlRecvBuffer;
	char*						m_audioUDPRecvBuffer;
	char*						m_audioUDPCtrlRecvBuffer;

	uint64_t					m_prevHeatBeatTime;
	bool						m_firstHeatbeat;
private:
	static void onFreamCallBack(char *pFrameBuf, int nFrameSize, int nFreamType, long lTimestamp, uint64_t dwUser, void* lpUser);
	void onVideoUDPRecvCallback(Socket* sock, const char* buffer, int len, const NetAddr& addr);
	void onVideoCtrlUDPRecvCallback(Socket* sock, const char* buffer, int len, const NetAddr& addr);
	void onAudioUDPRecvCallback(Socket* sock, const char* buffer, int len, const NetAddr& addr);
	void onAudioCtrlUDPRecvCallback(Socket* sock, const char* buffer, int len, const NetAddr& addr);
	void onVideoAndAudioTCPRecvCallback(const char* buffer, int len, bool isvideo);
	void onProtocolStatusCallback(RTSPStatus_t status, const std::string& errorinfo);
	void onProtocolCallback(bool sender, const char* buffer, uint32_t len);

	/*构造RTP包分析器*/
	int creatRTPAnalyzer();
	void setFrameInfo(int nFrameType, int nFrameSize, int nWidth, int nHight, long lTimeStamp, FRAME_INFO& stFrameInfo);

	void onTimerProc(unsigned long);	
private:
	int							m_nFrameIndex;//帧序号
	bool						m_bFirstIFream;
	bool						m_bSendHeader;
	long						m_lPrevVideoTs;//前一帧视频的RTP时间戳
	long						m_lVideoTsDiff;
	long						m_lCurrentVideoTs;
};
