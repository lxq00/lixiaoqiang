#include "RTSPProtocol.h"
#include "../StringParse/sdpParse.h"
#include "../StringParse/spsParse.h"

#define MAXTCPSOCKETBUFFERLEN		65*1024
#define CMDTIMEOUT					30000

RTSPProtocol::RTSPProtocol(const std::string& rtspUrl)
	:m_videoSdp(0),m_audioSdp(2), m_nSpsLen(0), m_nPpsLen(0), m_nFrameRate(25), m_nRecvType(RTP_RECV_TYPE_UDP)
{
	m_rtspInfo = make_shared<RTSPUrlInfo>();
	m_rtspInfo->parse(rtspUrl);

	m_tcprecvBufferLen = 0;
	m_tcprecvBuffer = new char[MAXTCPSOCKETBUFFERLEN + 100];

	memset(&m_udpRecvPort, 0, sizeof(m_udpRecvPort));
}
RTSPProtocol::~RTSPProtocol()
{
	m_tcpsocket = NULL;
	SAFE_DELETEARRAY(m_tcprecvBuffer);
}

bool RTSPProtocol::startProtocol(uint32_t timeout)
{
	m_tcpsocket = make_shared<TCPClient>(m_ioworker);
	m_tcpsocket->setSocketTimeout(timeout, timeout);

	bool connectret = m_tcpsocket->connect(NetAddr(m_rtspInfo->m_szServerIp, m_rtspInfo->m_nServerPort));
	if (!connectret) return false;

	m_tcpsocket->setDisconnectCallback(Socket::DisconnectedCallback(&RTSPProtocol::_tcpSocketDisconnectCallback, this));
	m_tcpsocket->async_recv(m_tcprecvBuffer + m_tcprecvBufferLen,MAXTCPSOCKETBUFFERLEN - m_tcprecvBufferLen,Socket::ReceivedCallback(&RTSPProtocol::_tcpRecvCallback,this));

	addFlowList(shared_ptr<CMDBuilder>(new CMDBuilder_OPTIONS()));
	doFlowList();

	return true;
}

bool RTSPProtocol::stopProcol()
{
	addFlowList(shared_ptr<CMDBuilder>(new CMDBuilder_TEARDOWN()));
	doFlowList();
	
	Thread::sleep(100);

	return true;
}

void RTSPProtocol::_tcpSocketDisconnectCallback(Socket* sock, const char* errrstr)
{
	m_statusCallback(RTSPStatus_DisConnect,std::string("rtsp socket disconnect"));
}

void RTSPProtocol::_tcpRecvCallback(Socket* , const char* , int len)
{
	uint32_t backlen = m_tcprecvBufferLen;
	if (len > 0) m_tcprecvBufferLen += len;

	m_tcprecvBuffer[m_tcprecvBufferLen] = 0;
	const char* buffertmp = m_tcprecvBuffer;
	uint32_t buffertmplen = m_tcprecvBufferLen;

	while (buffertmplen > strlen(RTSPHEADERFLAG))
	{
		//是头数据
		if (strncasecmp(buffertmp, RTSPHEADERFLAG, strlen(RTSPHEADERFLAG)) == 0)
		{
			//有头，开始找尾巴
			const char* cmdend = strstr(buffertmp, RTSPENDFLAG1);
			if (cmdend != NULL)
			{
				cmdend += strlen(RTSPENDFLAG1);
			}
			else
			{
				cmdend = strstr(buffertmp, RTSPENDFLAG2);
				if (cmdend != NULL)
				{
					cmdend += strlen(RTSPENDFLAG2);
				}
			}
			//没找到尾巴，下次再来
			if (cmdend == NULL)
			{
				break;
			}
			std::string cmdresp(buffertmp, cmdend - buffertmp);
			RTSP_RESPONSE response;
			ParseResponse(cmdresp, response);
			if (response.content_length != 0)
			{
				//数据不够
				if (buffertmp + buffertmplen - cmdend < response.content_length)
				{
					break;
				}
				cmdend += response.content_length;
				cmdresp = std::string(buffertmp, cmdend - buffertmp);
				ParseResponse(cmdresp, response);
			}

			m_protocolCallback(false, cmdresp.c_str(), cmdresp.length());

			doDispatcherCMD(response);

			buffertmplen -= (cmdend - buffertmp);
			buffertmp = cmdend;
		}
		else
		{
			//不是数据魔术
			if (*buffertmp != 0x24)
			{
				buffertmp++;
				buffertmplen--;
				continue;
			}

			INTERLEAVEDFRAME* frame = (INTERLEAVEDFRAME*)buffertmp;
			uint32_t nNeedRecvLen = ntohs(frame->len);

			//数据不够
			if (nNeedRecvLen > buffertmplen - sizeof(INTERLEAVEDFRAME))
			{
				break;
			}
			const char* videobuffer = buffertmp + sizeof(INTERLEAVEDFRAME);
			m_dataCallback(videobuffer, nNeedRecvLen, frame->type == m_videoSdp.channel);

			buffertmp = videobuffer + nNeedRecvLen;
			buffertmplen -= (sizeof(INTERLEAVEDFRAME) + nNeedRecvLen);
		}		
	}

	if (buffertmplen > 0 && buffertmp != m_tcprecvBuffer)
	{
		memmove(m_tcprecvBuffer, buffertmp, buffertmplen);
	}
	m_tcprecvBufferLen = buffertmplen;

	
	m_tcpsocket->async_recv(m_tcprecvBuffer + m_tcprecvBufferLen, MAXTCPSOCKETBUFFERLEN - m_tcprecvBufferLen, Socket::ReceivedCallback(&RTSPProtocol::_tcpRecvCallback, this));
}

void RTSPProtocol::onTimerProc()
{
	FlowNode node;
	{
		Guard locker(mutex);
		if (flowList.size() == 0) return;
		node = flowList.front();
	}

	uint64_t nowtime = Time::getCurrentMilliSecond();
	if (nowtime < node.startTime) node.startTime = nowtime;

	if (nowtime > node.startTime && nowtime - node.startTime >= CMDTIMEOUT)
	{
		m_statusCallback(RTSPStatus_Timeout, std::string("cmd ") + node.cmdbuilder->cmd + " recv timeout");
		return;
	}
}
void RTSPProtocol::addHeatbeat()
{
	bool haveNoCmd = false;
	{
		Guard locker(mutex);
		haveNoCmd = flowList.size() == 0;
	}

	addFlowList(shared_ptr<CMDBuilder>(new CMDBuilder_GET_PARAMETER()));
	if (!haveNoCmd)
	{
		doFlowList();
	}
}
void RTSPProtocol::addFlowList(const shared_ptr<CMDBuilder>& builder)
{
	FlowNode node;

	Guard locker(mutex);
	if (builder != NULL)
	{
		node.cmdbuilder = builder;
		flowList.push_back(node);
	}
}
void RTSPProtocol::doFlowList()
{
	FlowNode node;

	{
		Guard locker(mutex);
		if (flowList.size() <= 0)
		{
			return;
		}
		node = flowList.front();
	}
	std::string senddata = node.cmdbuilder->build(m_rtspInfo);

	shared_ptr<TCPClient> socket = m_tcpsocket;
	if (socket != NULL)
	{
		m_protocolCallback(true, senddata.c_str(), senddata.length());

		socket->send(senddata.c_str(), senddata.length());
	}
}

void RTSPProtocol::doDispatcherCMD(const RTSP_RESPONSE& repose)
{
	FlowNode node;
	int flowlistsize = 0;
	{
		Guard locker(mutex);
		if (flowList.size() <= 0 || repose.cseq != flowList.front().cmdbuilder->m_nCSeq)
		{
			return;
		}
		node = flowList.front();
		flowList.pop_front();
		flowlistsize = flowList.size();
	}

	uint32_t errorcode = atoi(repose.retcode);

	if (errorcode == 401 && node.cmdbuilder->cmdType == RTSPCmd_OPTIONS)
	{
		m_rtspInfo->parseAuthenString(repose.www_authenticate);

		addFlowList(shared_ptr<CMDBuilder>(new CMDBuilder_OPTIONS()));
		doFlowList();
	}
	else if (errorcode == 401 && node.cmdbuilder->cmdType == RTSPCmd_DESCRIBE)
	{
		m_rtspInfo->parseAuthenString(repose.www_authenticate);

		addFlowList(shared_ptr<CMDBuilder>(new CMDBuilder_DESCRIBE()));
		doFlowList();
	}
	else if (errorcode == 200)
	{
		if (node.cmdbuilder->cmdType == RTSPCmd_OPTIONS)
		{
			addFlowList(shared_ptr<CMDBuilder>(new CMDBuilder_DESCRIBE()));
		}
		else if (node.cmdbuilder->cmdType == RTSPCmd_DESCRIBE)
		{
			//分析sdp, 得到音视频信息
			bool nbParse = ParseSDP(repose.body, &m_stMediaInfo);
			if (true == nbParse)
			{
				int nWidth = 0;
				int nHeight = 0;

				//解析sps信息，获取视频尺寸
				int nRet = DecodeSpsPpsInfo(m_stMediaInfo.stStreamVideo.szSpsPps, nWidth, nHeight, m_szSpsBuffer, m_nSpsLen, m_szPpsBuffer, m_nPpsLen, m_nFrameRate);
				if (0 != nRet)
				{
					//解码视频尺寸失败, 不作退出处理, 只需要记录警告日志
				}
				else
				{
					m_stMediaInfo.stStreamVideo.nWidth = nWidth;
					m_stMediaInfo.stStreamVideo.nHight = nHeight;
				}
			}

			//构建视频setup任务
			{
				m_videoSdp.clientPort = m_udpRecvPort[0];
				m_videoSdp.clientCtrolPort = m_udpRecvPort[1];
					
				addFlowList(shared_ptr<CMDBuilder>(new CMDBuilder_SETUP(
					m_stMediaInfo.stStreamVideo.szTrackID,
					m_videoSdp.build(m_nRecvType),
					true
				)));
			}

			//构建音频setup任务
			if(m_stMediaInfo.bHasAudio)
			{
				m_audioSdp.clientPort = m_udpRecvPort[2];
				m_audioSdp.clientCtrolPort = m_udpRecvPort[3];
				
				addFlowList(shared_ptr<CMDBuilder>(new CMDBuilder_SETUP(
					m_stMediaInfo.stStreamAudio.szTrackID,
					m_audioSdp.build(m_nRecvType),
					false
				)));
			}
		}
		else if (node.cmdbuilder->cmdType == RTSPCmd_SETUP)
		{
			if (((CMDBuilder_SETUP*)node.cmdbuilder.get())->isVideo)
			{
				m_videoSdp.parse(repose.transport);
				if (NULL != repose.session)
				{
					m_rtspInfo->m_szSession = repose.session;
				}
			}
			else
			{
				m_audioSdp.parse(repose.transport);
			}

			//setup完了，需要play了
			if (flowlistsize == 0)
			{
				m_statusCallback(RTSPStatus_ConnectSuccess, "Setup Success Start Play");

				addFlowList(shared_ptr<CMDBuilder>(new CMDBuilder_PLAY(play_type,play_startTime,play_stopTime)));
			}
		}

		doFlowList();
	}
	else
	{
		m_statusCallback(RTSPStatus_CMDError,std::string("recv cmd ") + node.cmdbuilder->cmd + "error code " + repose.retcode);
	}
}
