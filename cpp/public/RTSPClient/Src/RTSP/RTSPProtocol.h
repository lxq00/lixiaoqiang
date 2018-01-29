#ifndef __RTSPPROTOCOL_H__
#define __RTSPPROTOCOL_H__
#include "RTSPClient/RTSPClient.h"
using namespace Public::RTSPClient;
#include "RTSPCMD.h"
#include "../StringParse/rtspResponseParse.h"

struct INTERLEAVEDFRAME
{
	uint8_t flag;
	uint8_t type;
	unsigned short len;
};

class RTSPProtocol
{
	struct FlowNode
	{
		FlowNode():startTime(Time::getCurrentMilliSecond()){}

		shared_ptr<CMDBuilder>	cmdbuilder;
		uint64_t				startTime;
	};
protected:
	typedef Function3<void, const char*, int,bool>				MediaDataCallback;
	typedef Function2<void, RTSPStatus_t,const std::string&>	StatusCallback;
	typedef Function3<void, bool /*send*/, const char*, uint32_t> ProtocolCallback;
protected:
	RTSPProtocol(const std::string& rtspUrl);
	~RTSPProtocol();

	bool startProtocol(uint32_t timeout);
	bool stopProcol();

	void onTimerProc();
	void addHeatbeat();
private:
	void addFlowList(const shared_ptr<CMDBuilder>& builder);
	void doFlowList();
	void doDispatcherCMD(const RTSP_RESPONSE& repose);
	void _tcpRecvCallback(const shared_ptr<Socket>& sock, const char* buf, int len);
	void _tcpSocketDisconnectCallback(const shared_ptr<Socket>& sock, const std::string& errrstr);
protected:
	MediaDataCallback			m_dataCallback;
	StatusCallback				m_statusCallback;
	ProtocolCallback			m_protocolCallback;
public:
	RTP_RECV_Type				m_nRecvType;//流传输方式 0-TCP, 1-UDP
	MEDIA_INFO					m_stMediaInfo;
	shared_ptr<RTSPUrlInfo>		m_rtspInfo;

	uint32_t					m_udpRecvPort[4];

	int							m_nFrameRate;

	std::string					play_type;
	std::string					play_startTime;
	std::string					play_stopTime;
protected:
	shared_ptr<AsyncIOWorker>	m_ioworker;
	shared_ptr<TCPClient>		m_tcpsocket;

	char						m_szSpsBuffer[512];
	int							m_nSpsLen;
	char						m_szPpsBuffer[512];
	int							m_nPpsLen;	
protected:
	char*						m_tcprecvBuffer;
	uint32_t					m_tcprecvBufferLen;

	SDPInfo						m_videoSdp;
	SDPInfo						m_audioSdp;
	Mutex						mutex;
	std::list<FlowNode>			flowList;
};


#endif //__RTSPPROTOCOL_H__
