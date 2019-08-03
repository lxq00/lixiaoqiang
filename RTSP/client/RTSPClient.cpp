#pragma once
#include "RTSP/RTSPClient.h"
#include "../common/rtspSession.h"

namespace Public {
namespace RTSP {

struct RTSPClient::RTSPClientInternal:public RTSPSession
{
	shared_ptr<RTSPClientHandler>	handler;

	bool						socketconnected;	

	uint32_t					connecttimeout;
	uint64_t					startconnecttime;

	shared_ptr<Timer>			pooltimer;

	uint64_t					prevheartbeattime;
	
	Mutex						mutex;
	std::list<shared_ptr<CommandInfo> >	sendcmdlist;

	uint32_t				tcpinterval;
	RTSPClientInternal(const shared_ptr<RTSPClientHandler>& _handler, const AllockUdpPortCallback& allockport, const shared_ptr<IOWorker>& worker, const RTSPUrl& url, const std::string& _useragent)
		:handler(_handler), socketconnected(false), connecttimeout(10000), tcpinterval(0)
	{
		ioworker = worker;
		rtspurl = url;
		useragent = _useragent;
		allockportcallback = allockport;

		prevheartbeattime = Time::getCurrentMilliSecond();
	}
	~RTSPClientInternal()
	{
		stop();
	}

	bool start(uint32_t timeout)
	{
		connecttimeout = timeout;
		startconnecttime = Time::getCurrentMilliSecond();
		socket = TCPClient::create(ioworker);
		socket->setSocketTimeout(timeout, timeout);
		socket->async_connect(NetAddr(rtspurl.serverip, rtspurl.serverport), Socket::ConnectedCallback(&RTSPClientInternal::socketconnectcallback, this));
		pooltimer = make_shared<Timer>("RTSPClientInternal");
		pooltimer->start(Timer::Proc(&RTSPClientInternal::onpooltimerproc, this), 0, 1000);

		return RTSPSession::start(timeout);
	}

	bool stop()
	{
		RTSPSession::stop();

		handler = NULL;
		pooltimer = NULL;
		
		return true;
	}
private:
	void onSendRequestCallback(const shared_ptr<CommandInfo>& cmd)
	{
		Guard locker(mutex);

		sendcmdlist.push_back(cmd);
	}
	void onpooltimerproc(unsigned long)
	{
		uint64_t nowtime = Time::getCurrentMilliSecond();

		if (!socketconnected && socket != NULL && nowtime > startconnecttime && nowtime - startconnecttime > connecttimeout)
		{
			handler->onConnectResponse(false, "connect timeout");
			socket = NULL;
		}

		{
			shared_ptr<CommandInfo> cmdinfo;
			{
				Guard locker(mutex);
				if (sendcmdlist.size() > 0 && sendcmdlist.front()->cmd->cseq == protocolstartcseq)
				{
					cmdinfo = sendcmdlist.front();
				}

				if (cmdinfo && nowtime > cmdinfo->starttime && nowtime - cmdinfo->starttime > cmdinfo->timeout)
				{
					sendcmdlist.pop_front();
				}
				else
				{
					cmdinfo = NULL;
				}
			}
			if (cmdinfo)
			{
				if (handler) handler->onErrorResponse(cmdinfo->cmd, 406, "Command Timeout");
			}
		}

		if(socketconnected && sessionstr.length() > 0 && rtspmedia)
		{
			uint64_t nowtime = Time::getCurrentMilliSecond();
			if (nowtime > prevheartbeattime && nowtime - prevheartbeattime > 10000)
			{
				std::list<shared_ptr< STREAM_TRANS_INFO> > transinfos = rtspmedia->infos;

				bool havesendhearbeat = true;
				
				for (std::list<shared_ptr< STREAM_TRANS_INFO> >::iterator iter = transinfos.begin(); iter != transinfos.end(); iter++)
				{
					shared_ptr<RTPSession> rtpsession = (*iter)->rtpsession;

					if (rtpsession && !rtpsession->onPoolHeartbeat()) havesendhearbeat = false;
				}

				if (!havesendhearbeat)
				{
					sendOptionsRequest();
				}

				prevheartbeattime = nowtime;
			}
		}
	}
	void socketconnectcallback(const weak_ptr<Socket>& sock,bool status,const std::string& errmsg)
	{
		if (!status)
		{
			handler->onClose(std::string("connect error "+errmsg));
			return;
		}

		//const shared_ptr<Socket>& sock, const CommandCallback& cmdcallback, const ExternDataCallback& datacallback, const DisconnectCallback& disconnectCallback,bool server
		socketconnected = true;
		handler->onConnectResponse(true, "OK");

		shared_ptr<Socket> socktmp = sock.lock();
		if (socktmp)
		{
			protocol = make_shared<RTSPProtocol>(socktmp, RTSPProtocol::CommandCallback(&RTSPClientInternal::rtspCommandCallback, this),
				RTSPProtocol::DisconnectCallback(&RTSPClientInternal::socketDisconnectCallback, this), false);
		}	

		sendOptionsRequest();
	}

	void checkRecvSetupResponse(const shared_ptr<CommandInfo>& cmdinfo, const std::string& tranportstr)
	{
		TRANSPORT_INFO transport;
		rtsp_header_parse_transport(tranportstr.c_str(), &transport);

		if (transport.transport == TRANSPORT_INFO::TRANSPORT_NONE) return;

		shared_ptr<STREAM_TRANS_INFO> transportinfo;

		
		for (std::list<shared_ptr<STREAM_TRANS_INFO> >::const_iterator iter = rtspmedia->infos.begin(); iter != rtspmedia->infos.end(); iter++)
		{
			if (String::indexOfByCase(cmdinfo->cmd->url, (*iter)->streaminfo.szTrackID) != -1)
			{
				transportinfo = *iter;

				transportinfo->transportinfo = transport;

				break;
			}
		}
		
		if(transportinfo)
			handler->onSetupResponse(cmdinfo->cmd, transportinfo);
	}
	shared_ptr<STREAM_TRANS_INFO> checkAndBuildNotSetupTransport()
	{
		shared_ptr<STREAM_TRANS_INFO> transportinfo;

		for (std::list<shared_ptr<STREAM_TRANS_INFO> >::const_iterator iter = rtspmedia->infos.begin(); iter != rtspmedia->infos.end(); iter++)
		{
			if ((*iter)->transportinfo.transport == TRANSPORT_INFO::TRANSPORT_NONE)
			{
				transportinfo = *iter;
				break;
			}
		}

		if (!transportinfo) return shared_ptr<STREAM_TRANS_INFO>();

		if (transportbytcp)
		{
			transportinfo->transportinfo.transport = TRANSPORT_INFO::TRANSPORT_RTP_TCP;
			transportinfo->transportinfo.rtp.t.dataChannel = tcpinterval;
			transportinfo->transportinfo.rtp.t.contorlChannel = tcpinterval + 1;

			tcpinterval += 2;
		}
		else
		{
			uint32_t startport = allockportcallback();

			transportinfo->transportinfo.transport = TRANSPORT_INFO::TRANSPORT_RTP_UDP;
			transportinfo->transportinfo.rtp.u.client_port1 = startport;
			transportinfo->transportinfo.rtp.u.client_port2 = startport + 1;
		}

		return transportinfo;
	}
	void rtspCommandCallback(const shared_ptr<RTSPCommandInfo>& cmdheader)
	{
		if (strcasecmp(cmdheader->verinfo.protocol.c_str(), "rtsp") != 0 || cmdheader->verinfo.version != "1.0" || cmdheader->method.length() != 0)
		{
			assert(0);
			return;
		}

		shared_ptr<CommandInfo> cmdinfo;
		{
			Guard locker(mutex);
			if (sendcmdlist.size() <= 0) return;

			cmdinfo = sendcmdlist.front();
			if (cmdinfo->cmd->cseq != cmdheader->cseq)
			{
				assert(0);
				return;
			}
			sendcmdlist.pop_front();
		}

		if (cmdheader->statuscode == 401)
		{
			if (rtspurl.username == "" || rtspurl.password == "")
			{
				handler->onErrorResponse(cmdinfo->cmd, cmdheader->statuscode, cmdheader->statusmsg);
				handler->onClose("no authenticate info");
			}
			else
			{
				std::string wwwauthen = cmdheader->header("WWW-Authenticate").readString();
				cmdinfo->wwwauthen = wwwauthen;

				sendRequest(cmdinfo);
			}
		}
		else if (cmdheader->statuscode != 200)
		{
			handler->onErrorResponse(cmdinfo->cmd, cmdheader->statuscode, cmdheader->statusmsg);
		}
		else if (strcasecmp(cmdinfo->cmd->method.c_str(), "OPTIONS") == 0)
		{
			//没建立会话
			if (rtspmedia == NULL)
			{
				sendDescribeRequest();
			}	
		}
		else if (strcasecmp(cmdinfo->cmd->method.c_str(), "DESCRIBE") == 0)
		{
			rtspmedia = make_shared<MEDIA_INFO>();

			rtsp_header_parse_sdp(cmdheader->body.c_str(), rtspmedia.get());

			handler->onDescribeResponse(cmdinfo->cmd, rtspmedia);

			shared_ptr<STREAM_TRANS_INFO> transportinfo = checkAndBuildNotSetupTransport();
			if (transportinfo)
			{
				sendSetupRequest(transportinfo);
			}
		}
		else if (strcasecmp(cmdinfo->cmd->method.c_str(), "SETUP") == 0)
		{
			if (sessionstr.length() <= 0)
			{
				const std::string sessionstrtmp = cmdheader->header("Session").readString();
				const char* sessionptraddr = strchr(sessionstrtmp.c_str(), ';');
				if (sessionptraddr != NULL) sessionstr = std::string(sessionstrtmp.c_str(), sessionptraddr - sessionstrtmp.c_str());
				else sessionstr = sessionstrtmp;
			}
			std::string tranportstr = cmdheader->header("Transport").readString();

			//检查接收回来的setup回复
			checkRecvSetupResponse(cmdinfo, tranportstr);

			//发送失败说明setup已经发送全部
			shared_ptr<STREAM_TRANS_INFO> transportinfo = checkAndBuildNotSetupTransport();
			if (transportinfo)
			{
				sendSetupRequest(transportinfo);
			}
			else
			{
				prevheartbeattime = Time::getCurrentMilliSecond();
				buildRtpSession(false);				

				sendPlayRequest(RANGE_INFO());
			}
		}
		else if (strcasecmp(cmdinfo->cmd->method.c_str(), "PLAY") == 0)
		{
			if (cmdinfo->waitsem == NULL)
				handler->onPlayResponse(cmdinfo->cmd);
		}
		else if (strcasecmp(cmdinfo->cmd->method.c_str(), "PAUSE") == 0)
		{
			if (cmdinfo->waitsem == NULL)
				handler->onPlayResponse(cmdinfo->cmd);
		}
		else if (strcasecmp(cmdinfo->cmd->method.c_str(), "GET_PARAMETER"))
		{
			if (cmdinfo->waitsem == NULL)
				handler->onGetparameterResponse(cmdinfo->cmd, cmdheader->body);
		}
		else if (strcasecmp(cmdinfo->cmd->method.c_str(), "TERADOWN") == 0)
		{
			if (cmdinfo->waitsem == NULL)
				handler->onTeradownResponse(cmdinfo->cmd);
		}

		if (cmdinfo->waitsem != NULL)
		{
			cmdinfo->responseheader = cmdheader;
			cmdinfo->waitsem->post();
		}
	}
	void onContorlDataCallback(const shared_ptr<STREAM_TRANS_INFO>& transinfo, const char* buffer, uint32_t len)
	{
		handler->onContorlPackageCallback(transinfo, buffer, len);
	}
	void onMediaDataCallback(const shared_ptr<STREAM_TRANS_INFO>& transinfo, uint32_t timestmap, const char* buffer, uint32_t len, bool mark)
	{
		handler->onMediaPackageCallback(transinfo, timestmap, buffer, len, mark);
	}
	void socketDisconnectCallback()
	{
		socketconnected = false;
		if (handler) handler->onClose("socket disconnected");
	}
};

RTSPClient::RTSPClient(const std::shared_ptr<IOWorker>& work, const shared_ptr<RTSPClientHandler>& handler, const AllockUdpPortCallback& allockport, const RTSPUrl& rtspUrl, const std::string& useragent)
{
	internal = new RTSPClientInternal(handler, allockport, work, rtspUrl, useragent);
}
RTSPClient::~RTSPClient()
{
	stop();
	SAFE_DELETE(internal);
}
bool RTSPClient::initRTPOverTcpType()
{
	internal->transportbytcp = true;

	return true;
}
bool RTSPClient::initRTPOverUdpType()
{
	internal->transportbytcp = false;

	return true;
}
bool RTSPClient::start(uint32_t timeout)
{
	return internal->start(timeout);
}

bool RTSPClient::stop()
{
	return internal->stop();
}

shared_ptr<RTSPCommandInfo> RTSPClient::sendPlayRequest(const RANGE_INFO& range)
{
	return internal->sendPlayRequest(range)->cmd;
}

bool RTSPClient::sendPlayRequest(const RANGE_INFO& range, uint32_t timeout)
{
	shared_ptr<RTSPClientInternal::CommandInfo> cmdinfo = internal->sendPlayRequest(range, timeout);
	if (!cmdinfo->waitsem->pend(timeout) || cmdinfo->responseheader == NULL)
	{
		return false;
	}

	return cmdinfo->responseheader->statuscode == 200;
}

shared_ptr<RTSPCommandInfo> RTSPClient::sendPauseRequest()
{
	return internal->sendPauseRequest()->cmd;
}
bool RTSPClient::sendPauseRequest(uint32_t timeout)
{
	shared_ptr<RTSPClientInternal::CommandInfo> cmdinfo = internal->sendPauseRequest(timeout);
	if (!cmdinfo->waitsem->pend(timeout) || cmdinfo->responseheader == NULL)
	{
		return false;
	}

	return cmdinfo->responseheader->statuscode == 200;
}

shared_ptr<RTSPCommandInfo> RTSPClient::sendGetparameterRequest(const std::string& body)
{
	return internal->sendGetparameterRequest(body)->cmd;
}
bool RTSPClient::sendGetparameterRequest(const std::string& body, uint32_t timeout)
{
	shared_ptr<RTSPClientInternal::CommandInfo> cmdinfo = internal->sendGetparameterRequest(body, timeout);
	if (!cmdinfo->waitsem->pend(timeout) || cmdinfo->responseheader == NULL)
	{
		return false;
	}

	return cmdinfo->responseheader->statuscode == 200;
}
shared_ptr<RTSPCommandInfo> RTSPClient::sendTeradownRequest()
{
	return internal->sendTeardownRequest()->cmd;
}
bool RTSPClient::sendTeradownRequest(uint32_t timeout)
{
	shared_ptr<RTSPClientInternal::CommandInfo> cmdinfo = internal->sendTeardownRequest(timeout);
	if (!cmdinfo->waitsem->pend(timeout) || cmdinfo->responseheader == NULL)
	{
		return false;
	}
	return cmdinfo->responseheader->statuscode == 200;
}

bool RTSPClient::sendMediaPackage(const shared_ptr<STREAM_TRANS_INFO> mediainfo, uint32_t timestmap, const char*  buffer, uint32_t bufferlen, bool mark)
{
	shared_ptr<RTPSession> rtpsession = mediainfo->rtpsession;

	if (mediainfo == NULL || rtpsession == NULL || buffer == NULL || bufferlen <= 0) return false;

	rtpsession->sendMediaData(mediainfo, timestmap, buffer, bufferlen, mark);

	return true;
}
bool RTSPClient::sendContorlPackage(const shared_ptr<STREAM_TRANS_INFO> mediainfo, const char*  buffer, uint32_t bufferlen)
{
	shared_ptr<RTPSession> rtpsession = mediainfo->rtpsession;

	if (mediainfo == NULL || rtpsession == NULL || buffer == NULL || bufferlen <= 0) return false;

	rtpsession->sendContorlData(mediainfo, buffer, bufferlen);

	return true;
}

}
}