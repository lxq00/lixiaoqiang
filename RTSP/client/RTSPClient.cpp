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

	RTSPClientInternal(const shared_ptr<RTSPClientHandler>& _handler, const AllockUdpPortCallback& allockport, const shared_ptr<IOWorker>& worker, const RTSPUrl& url, const std::string& _useragent)
		:handler(_handler), socketconnected(false), connecttimeout(10000)
	{
		ioworker = worker;
		rtspurl = url;
		useragent = _useragent;
		allockportcallback = allockport;

		rtspmedia.videoTransport.transport = TRANSPORT_INFO::TRANSPORT_RTP_TCP;
		rtspmedia.videoTransport.rtp.t.dataChannel = 0;
		rtspmedia.videoTransport.rtp.t.contorlChannel = rtspmedia.videoTransport.rtp.t.dataChannel + 1;
		rtspmedia.audioTransport = rtspmedia.videoTransport;

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

		if(socketconnected && sessionstr.length() > 0)
		{
			uint64_t nowtime = Time::getCurrentMilliSecond();
			if (nowtime > prevheartbeattime && nowtime - prevheartbeattime > 10000)
			{
				if (rtspmedia.videoTransport.transport == TRANSPORT_INFO::TRANSPORT_RTP_TCP)
				{
					if (rtp) rtp->onPoolHeartbeat();
				}
				else
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
			if(sessionstr.length() == 0)
				sendDescribeRequest();
		}
		else if (strcasecmp(cmdinfo->cmd->method.c_str(), "DESCRIBE") == 0)
		{
			rtsp_header_parse_sdp(cmdheader->body.c_str(), &rtspmedia.media);

			if (cmdinfo->waitsem == NULL)
				handler->onDescribeResponse(cmdinfo->cmd, rtspmedia.media);

			sendSetupRequest();
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

			TRANSPORT_INFO transport;
			rtsp_header_parse_transport(tranportstr.c_str(), &transport);

			if (handler) handler->onSetupResponse(cmdinfo->cmd, rtspmedia.media, transport);

			//build socket
			{
				if (transport.transport == TRANSPORT_INFO::TRANSPORT_RTP_TCP)
				{
					rtspmedia.videoTransport = rtspmedia.audioTransport = transport;

					protocol->setTCPInterleavedChannel(rtspmedia.videoTransport.rtp.t.dataChannel, rtspmedia.audioTransport.rtp.t.contorlChannel);

					//rtpOverTcp(bool _isserver, const shared_ptr<RTSPProtocol>& _protocol, const RTSP_MEDIA_INFO& _rtspmedia, const RTPDataCallback& _datacallback)
					rtp = make_shared<rtpOverTcp>(false, protocol.get(), rtspmedia, rtp::RTPDataCallback(&RTSPClientInternal::rtpDataCallback, this));
				}
				else if (rtspmedia.media.bHasVideo && String::indexOfByCase(cmdinfo->cmd->url, rtspmedia.media.stStreamVideo.szTrackID) != -1)
				{
					rtspmedia.videoTransport = transport;
				}
				else if (rtspmedia.media.bHasAudio && String::indexOfByCase(cmdinfo->cmd->url, rtspmedia.media.stStreamAudio.szTrackID) != -1)
				{
					rtspmedia.audioTransport = transport;
				}
			}

			bool haveSetupFinsh = true;
			if ((transport.transport == TRANSPORT_INFO::TRANSPORT_RTP_UDP && rtspmedia.media.bHasVideo && rtspmedia.videoTransport.rtp.u.server_port1 == 0) ||
				(transport.transport == TRANSPORT_INFO::TRANSPORT_RTP_UDP && rtspmedia.media.bHasAudio && rtspmedia.audioTransport.rtp.u.server_port1 == 0))
			{
				haveSetupFinsh = false;
			}

			if (haveSetupFinsh)
			{
				if (transport.transport == TRANSPORT_INFO::TRANSPORT_RTP_UDP)
				{
					//rtpOverUdp(bool _isserver, const shared_ptr<IOWorker>& work,const std::string& _dstaddr,const RTSP_MEDIA_INFO& _rtspmedia, const RTPDataCallback& _datacallback)
					rtp = make_shared<rtpOverUdp>(false, ioworker, rtspurl.serverip, rtspmedia, rtp::RTPDataCallback(&RTSPClientInternal::rtpDataCallback, this));

				}
				
				sendPlayRequest(RANGE_INFO());
			}
			else
			{
				sendSetupRequest();
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
	void rtpDataCallback(bool isvideo, uint32_t timestmap, const RTSPBuffer& data, bool mark,const RTPHEADER* header)
	{
		if (handler) handler->onMediaCallback(isvideo, timestmap, data, mark, header);
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
	internal->rtspmedia.videoTransport.transport = TRANSPORT_INFO::TRANSPORT_RTP_TCP;
	internal->rtspmedia.videoTransport.rtp.t.dataChannel = 0;
	internal->rtspmedia.videoTransport.rtp.t.contorlChannel = internal->rtspmedia.videoTransport.rtp.t.dataChannel + 1;
	internal->rtspmedia.audioTransport = internal->rtspmedia.videoTransport;


	return true;
}
bool RTSPClient::initRTPOverUdpType()
{
	internal->rtspmedia.videoTransport.transport = TRANSPORT_INFO::TRANSPORT_RTP_UDP;
	internal->rtspmedia.audioTransport.transport = TRANSPORT_INFO::TRANSPORT_RTP_UDP;
	internal->rtspmedia.videoTransport.rtp.u.client_port1 = 0;
	internal->rtspmedia.videoTransport.rtp.u.client_port2 = 0;
	internal->rtspmedia.videoTransport.rtp.u.server_port1 = 0;
	internal->rtspmedia.videoTransport.rtp.u.server_port2 = 0;
	internal->rtspmedia.audioTransport.rtp.u.client_port1 = 0;
	internal->rtspmedia.audioTransport.rtp.u.client_port2 = 0;
	internal->rtspmedia.audioTransport.rtp.u.server_port1 = 0;
	internal->rtspmedia.audioTransport.rtp.u.server_port2 = 0;

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

bool RTSPClient::sendMedia(bool isvideo, uint32_t timestmap, const RTSPBuffer& buffer, bool mark, const RTPHEADER* header)
{
	shared_ptr<rtp> rtptmp = internal->rtp;
	if (rtptmp) rtptmp->sendData(isvideo, timestmap, buffer, mark,header);

	return true;
}
}
}