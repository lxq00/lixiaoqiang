#include "RTSP/RTSPClient.h"
#include "../common/rtspProtocol.h"
#include "../common/sdpParse.h"
#include "../common/rtspTransport.h"
#include "../common/rtspRange.h"
#include "../common/rtspCMD.h"
#include "../common/rtpovertcp.h"
#include "../common/rtpoverudp.h"

namespace Public {
namespace RTSP {

struct RTSPClient::RTSPClientInternal
{
	struct CommandInfo
	{
		shared_ptr<RTSPCommandInfo> cmd;
		uint64_t				starttime;
		uint64_t				timeout;

		std::string				wwwauthen;

		shared_ptr<Semaphore>	waitsem;
		shared_ptr<HTTPParse::Header> responseheader;;

		CommandInfo(uint32_t _timeout) :starttime(Time::getCurrentMilliSecond()),timeout(_timeout)
		{
			cmd = make_shared<RTSPCommandInfo>(); 
			if (_timeout != -1) waitsem = make_shared<Semaphore>();
		}
	};


	shared_ptr<RTSPClientHandler>	handler;

	bool						socketconnected;
	shared_ptr<IOWorker>		ioworker;
	shared_ptr<Socket>			socket;
	std::string					useragent;
	shared_ptr<RTSPProtocol>	protocol;
	shared_ptr<RTSPUrlInfo>		rtspurl;

	RTSP_MEDIA_INFO				rtspmedia;


	uint32_t					connecttimeout;
	uint64_t					startconnecttime;

	uint32_t					protocolstartcseq;

	Mutex						mutex;
	std::list<shared_ptr<CommandInfo> >	cmdlist;

	std::string					sessionstr;
	shared_ptr<Timer>			pooltimer;

	shared_ptr<rtp>				rtp;

	AllockUdpPortCallback		allockportcallback;

	RTSPClientInternal(const shared_ptr<RTSPClientHandler>& _handler,const shared_ptr<IOWorker>& worker, const std::string& url, const std::string& _useragent)
		:handler(_handler),socketconnected(false), ioworker(worker), useragent(_useragent),connecttimeout(10000), protocolstartcseq(0)
	{
		rtspurl = make_shared<RTSPUrlInfo>();
		rtspurl->parse(url);

		rtspmedia.videoTransport.transport = TRANSPORT_INFO::TRANSPORT_RTP_TCP;
		rtspmedia.videoTransport.rtp.t.videointerleaved = 0;
		rtspmedia.videoTransport.rtp.t.audiointerleaved = 1;
		rtspmedia.audioTransport = rtspmedia.videoTransport;
	}
	~RTSPClientInternal() 
	{
		stop();
	}
	
	bool start(uint32_t timeout = 10000)
	{
		connecttimeout = timeout;
		startconnecttime = Time::getCurrentMilliSecond();
		socket = TCPClient::create(ioworker);
		socket->setSocketTimeout(timeout, timeout);
		socket->async_connect(NetAddr(rtspurl->m_szServerIp, rtspurl->m_nServerPort), Socket::ConnectedCallback(&RTSPClientInternal::socketconnectcallback, this));
		pooltimer = make_shared<Timer>("RTSPClientInternal");
		pooltimer->start(Timer::Proc(&RTSPClientInternal::onpooltimerproc, this), 0, 1000);

		return true;
	}

	bool stop()
	{
		pooltimer = NULL;
		if (socket) socket->disconnect();
		protocol = NULL;
		rtp = NULL;

		return true;
	}

	shared_ptr<CommandInfo> sendPlayRequest(const RANGE_INFO& range,uint32_t timeout = -1)
	{
		shared_ptr<CommandInfo> cmdinfo = make_shared<CommandInfo>(timeout);
		cmdinfo->cmd->method = "PLAY";
		cmdinfo->cmd->url = rtspurl->toUrl();
		cmdinfo->cmd->headers["Range"] = rtspBuildRange(range);
		
		rtspBuildRtspCommand(cmdinfo);

		return cmdinfo;
	}
	shared_ptr<CommandInfo> sendPauseRequest(uint32_t timeout = -1)
	{
		shared_ptr<CommandInfo> cmdinfo = make_shared<CommandInfo>(timeout);
		cmdinfo->cmd->method = "PAUSE";
		cmdinfo->cmd->url = rtspurl->toUrl();

		rtspBuildRtspCommand(cmdinfo);

		return cmdinfo;
	}
	shared_ptr<CommandInfo> sendGetparameterRequest(const std::string& body, uint32_t timeout = -1)
	{
		shared_ptr<CommandInfo> cmdinfo = make_shared<CommandInfo>(timeout);
		cmdinfo->cmd->method = "GET_PARAMETER";
		cmdinfo->cmd->url = rtspurl->toUrl();

		rtspBuildRtspCommand(cmdinfo,body);

		return cmdinfo;
	}
	shared_ptr<CommandInfo> sendTeradownRequest(uint32_t timeout = -1)
	{
		shared_ptr<CommandInfo> cmdinfo = make_shared<CommandInfo>(timeout);
		cmdinfo->cmd->method = "TERADOWN";
		cmdinfo->cmd->url = rtspurl->toUrl();

		rtspBuildRtspCommand(cmdinfo);

		return cmdinfo;
	}
	shared_ptr<CommandInfo> sendDescribeRequest()
	{
		shared_ptr<CommandInfo> cmdinfo = make_shared<CommandInfo>();
		cmdinfo->cmd->method = "DESCRIBE";
		cmdinfo->cmd->url = rtspurl->toUrl();

		rtspBuildRtspCommand(cmdinfo);

		return cmdinfo;
	}
	shared_ptr<CommandInfo> sendSetupRequest()
	{
		shared_ptr<CommandInfo> cmdinfo = make_shared<CommandInfo>();
		cmdinfo->cmd->method = "SETUP";

		if(sessionstr.length() == 0) sessionstr = Value(Time::getCurrentMilliSecond()).readString();
		
		if (rtspmedia.videoTransport.transport == TRANSPORT_INFO::TRANSPORT_RTP_TCP || rtspmedia.audioTransport.transport == TRANSPORT_INFO::TRANSPORT_RTP_TCP)
		{
			cmdinfo->cmd->url = rtspurl->toUrl() + "/" + (rtspmedia.media.bHasVideo ? rtspmedia.media.stStreamVideo.szTrackID : rtspmedia.media.stStreamAudio.szTrackID);
			cmdinfo->cmd->headers["Transport"] = buildTransport(rtspmedia.media.bHasVideo ? rtspmedia.videoTransport : rtspmedia.audioTransport);
		}
		else if (rtspmedia.media.bHasVideo && rtspmedia.videoTransport.rtp.u.server_port1 == 0)
		{
			if (rtspmedia.videoTransport.rtp.u.client_port1 == 0)
			{
				uint32_t rtpport = allockportcallback();

				rtspmedia.videoTransport.rtp.u.client_port1 = rtpport;
				rtspmedia.videoTransport.rtp.u.client_port2 = rtpport + 1;
				rtspmedia.audioTransport.rtp.u.client_port1 = rtpport + 2;
				rtspmedia.audioTransport.rtp.u.client_port2 = rtpport + 3;
			}
			cmdinfo->cmd->url = rtspurl->toUrl() + "/" + rtspmedia.media.stStreamVideo.szTrackID;
			cmdinfo->cmd->headers["Transport"] = buildTransport(rtspmedia.videoTransport);
		}
		else if (rtspmedia.media.bHasAudio && rtspmedia.audioTransport.rtp.u.server_port1 == 0)
		{
			if (rtspmedia.audioTransport.rtp.u.client_port1 == 0)
			{
				uint32_t rtpport = allockportcallback();

				rtspmedia.videoTransport.rtp.u.client_port1 = rtpport;
				rtspmedia.videoTransport.rtp.u.client_port2 = rtpport + 1;
				rtspmedia.audioTransport.rtp.u.client_port1 = rtpport + 2;
				rtspmedia.audioTransport.rtp.u.client_port2 = rtpport + 3;
			}
			cmdinfo->cmd->url = rtspurl->toUrl() + "/" + rtspmedia.media.stStreamAudio.szTrackID;
			cmdinfo->cmd->headers["Transport"] = buildTransport(rtspmedia.audioTransport);
		}

		rtspBuildRtspCommand(cmdinfo);

		return cmdinfo;
	}

	bool sendMedia(bool isvideo, uint32_t timestmap, const char* buffer, uint32_t bufferlen)
	{
		if (!rtp) return false;

		rtp->sendData(isvideo, timestmap, buffer, bufferlen);

		return true;
	}
private:
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
				if (cmdlist.size() > 0 && cmdlist.front()->cmd->CSeq == protocolstartcseq)
				{
					cmdinfo = cmdlist.front();
				}

				if (cmdinfo && nowtime > cmdinfo->starttime && nowtime - cmdinfo->starttime > cmdinfo->timeout)
				{
					cmdlist.pop_front();
				}
				else
				{
					cmdinfo = NULL;
				}
			}
			if (cmdinfo)
			{
				if (handler) handler->onErrorResponse(cmdinfo->cmd,406,"Command Timeout");

				checkAndSendCommand(shared_ptr<CommandInfo>());
			}
		}
	}
	void socketconnectcallback(const weak_ptr<Socket>& sock)
	{
		//const shared_ptr<Socket>& sock, const CommandCallback& cmdcallback, const ExternDataCallback& datacallback, const DisconnectCallback& disconnectCallback,bool server
		socketconnected = true;
		handler->onConnectResponse(true, "OK");

		protocol = make_shared<RTSPProtocol>(sock, RTSPProtocol::CommandCallback(&RTSPClientInternal::rtspCommandCallback, this),
			RTSPProtocol::DisconnectCallback(&RTSPClientInternal::socketDisconnectCallback, this), false);
	}
	void rtspBuildRtspCommand(const shared_ptr<CommandInfo>& cmdinfo, const std::string& body = "", const std::string& contentype = "")
	{
		if (body.length() > 0)
		{
			cmdinfo->cmd->headers[Content_Length] = body.length();
		}
		if (contentype.length() > 0)
		{
			cmdinfo->cmd->headers[Content_Type] = contentype;
		}
		if (useragent.length() > 0) cmdinfo->cmd->headers["User-Agent"] = useragent;
		if (sessionstr.length()> 0) cmdinfo->cmd->headers["Session"] = sessionstr;
		cmdinfo->cmd->body = body;

		{
			Guard locker(mutex);
			cmdlist.push_back(cmdinfo);
		}

		checkAndSendCommand(shared_ptr<CommandInfo>());
	}
	void checkAndSendCommand(const shared_ptr<CommandInfo>& cmdptr)
	{
		shared_ptr<CommandInfo> cmdinfo = cmdptr;

		{
			Guard locker(mutex);
			//no cmd and return
			if (cmdlist.size() <= 0) return;

			cmdinfo = cmdlist.front();

			//first cmd cseq == now cseq ,this cmd is send and wait response
			if (cmdptr == NULL && cmdinfo->cmd->CSeq == protocolstartcseq)
			{
				return;
			}
			//send cmd not first cmd and return
			else if (cmdptr && cmdptr.get() != cmdinfo.get())
			{
				return;
			}

			if (cmdinfo->cmd->CSeq != protocolstartcseq)
			{
				do
				{
					protocolstartcseq++;
				} while (protocolstartcseq == 0);

				cmdinfo->cmd->CSeq = protocolstartcseq;
				cmdinfo->cmd->headers["CSeq"] = cmdinfo->cmd->CSeq;
			}
		}

		if (cmdinfo->wwwauthen.length() > 0)
		{
			cmdinfo->cmd->headers["Authorization"] = rtspurl->buildAuthorization(cmdinfo->cmd->method, cmdinfo->cmd->url.href(), cmdinfo->wwwauthen);
		}

		std::string cmdstr = cmdinfo->cmd->method + " " + cmdinfo->cmd->url.href() + " RTSP/1.0" + HTTPSEPERATOR;
		for (std::map<std::string, Value>::iterator iter = cmdinfo->cmd->headers.begin(); iter != cmdinfo->cmd->headers.end(); iter++)
		{
			cmdstr += iter->first + ": " + iter->second.readString() + HTTPSEPERATOR;
		}
		cmdstr += HTTPSEPERATOR;
		if (cmdinfo->cmd->body.length() > 0) cmdstr += cmdinfo->cmd->body;


		if(protocol) protocol->sendProtocolData(cmdstr);
	}
	void rtspCommandCallback(const shared_ptr<HTTPParse::Header>& cmdheader, const std::string& cmdbody)
	{
		uint32_t CSeq = cmdheader->header("CSeq").readInt();
		
		if (strcasecmp(cmdheader->verinfo.protocol.c_str(), "rtsp") != 0 || cmdheader->verinfo.version != "1.0")
		{
			return;
		}

		shared_ptr<CommandInfo> cmdinfo;
		{
			Guard locker(mutex);
			if (cmdlist.size() <= 0) return;

			cmdinfo = cmdlist.front();
			if (cmdinfo->cmd->CSeq != CSeq) return;
		}

		if (cmdheader->statuscode != 200)
		{
			handler->onErrorResponse(cmdinfo->cmd,cmdheader->statuscode,cmdheader->statusmsg);
		}
		else if (cmdheader->statuscode == 401)
		{
			std::string wwwauthen = cmdheader->header("WWW-Authenticate").readString();
			cmdinfo->wwwauthen = wwwauthen;

			checkAndSendCommand(cmdinfo);
		}
		else if (strcasecmp(cmdinfo->cmd->method.c_str(), "OPTIONS") == 0)
		{
			if (!mediainfo.bHasAudio && !mediainfo.bHasVideo)
			{
				sendDescribeRequest();
			}
		}
		else if (strcasecmp(cmdinfo->cmd->method.c_str(), "DESCRIBE") == 0)
		{
			ParseSDP(cmdbody.c_str(), &rtspmedia.media);

			if(cmdinfo->waitsem == NULL)
				handler->onDescribeResponse(cmdinfo->cmd, rtspmedia.media);

			sendSetupRequest();
		}
		else if (strcasecmp(cmdinfo->cmd->method.c_str(), "SETUP") == 0)
		{
			std::string tranportstr = cmdheader->header("Transport").readString();

			TRANSPORT_INFO transport;
			rtsp_header_transport(tranportstr.c_str(), &transport);

			//build socket
			{
				if (transport.transport == TRANSPORT_INFO::TRANSPORT_RTP_TCP)
				{
					rtspmedia.videoTransport = rtspmedia.audioTransport = transport;

					//rtpOverTcp(bool _isserver, const shared_ptr<RTSPProtocol>& _protocol, const RTSP_MEDIA_INFO& _rtspmedia, const RTPDataCallback& _datacallback)
					rtp = make_shared<rtpOverTcp>(false, this, rtspmedia, rtp::RTPDataCallback(&RTSPClientInternal::rtpDataCallback, this));
				}
				else if (rtspmedia.media.bHasVideo && String::indexOfByCase(cmdinfo->cmd->url.href(), rtspmedia.media.stStreamVideo.szTrackID) != 0)
				{
					rtspmedia.videoTransport = transport;
				}
				else if (rtspmedia.media.bHasAudio && String::indexOfByCase(cmdinfo->cmd->url.href(), rtspmedia.media.stStreamAudio.szTrackID) != 0)
				{
					rtspmedia.audioTransport = transport;
				}

				if (!(transport.transport != TRANSPORT_INFO::TRANSPORT_RTP_UDP ||
					(rtspmedia.media.bHasVideo && rtspmedia.videoTransport.transport == TRANSPORT_INFO::TRANSPORT_NONE) ||
					(rtspmedia.media.bHasAudio && rtspmedia.audioTransport.transport == TRANSPORT_INFO::TRANSPORT_NONE)))
				{
					//rtpOverUdp(bool _isserver, const shared_ptr<IOWorker>& work,const std::string& _dstaddr,const RTSP_MEDIA_INFO& _rtspmedia, const RTPDataCallback& _datacallback)
					rtp = make_shared<rtpOverUdp>(false, ioworker, rtspurl->m_szServerIp, rtspmedia, rtp::RTPDataCallback(&RTSPClientInternal::rtpDataCallback, this));
				}
			}

			if ((transport.transport != TRANSPORT_INFO::TRANSPORT_RTP_UDP ||
				(rtspmedia.media.bHasVideo && rtspmedia.videoTransport.transport == TRANSPORT_INFO::TRANSPORT_NONE) ||
				(rtspmedia.media.bHasAudio && rtspmedia.audioTransport.transport == TRANSPORT_INFO::TRANSPORT_NONE)))
			{
				sendSetupRequest();
			}
			else
			{
				sendPlayRequest(RANGE_INFO());
			}
		}
		else if (strcasecmp(cmdheader->method.c_str(), "PLAY") == 0)
		{
			if (cmdinfo->waitsem == NULL)
				handler->onPlayResponse(cmdinfo->cmd);
		}
		else if (strcasecmp(cmdheader->method.c_str(), "PAUSE") == 0)
		{
			if (cmdinfo->waitsem == NULL)
				handler->onPlayResponse(cmdinfo->cmd);
		}
		else if (strcasecmp(cmdheader->method.c_str(), "GET_PARAMETER"))
		{
			if (cmdinfo->waitsem == NULL)
				handler->onGetparameterResponse(cmdinfo->cmd, cmdbody);
		}
		else if (strcasecmp(cmdheader->method.c_str(), "TERADOWN") == 0)
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
	void rtpDataCallback(bool isvideo, uint32_t timestmap, const char* buffer, uint32_t bufferlen)
	{
		if (handler) handler->onMediaCallback(isvideo, timestmap, buffer, bufferlen);
	}
	void socketDisconnectCallback()
	{
		socketconnected = false;
		if (handler) handler->onClose();
	}
};


RTSPClient::RTSPClient(const std::shared_ptr<IOWorker>& work, const shared_ptr<RTSPClientHandler>& handler, const std::string& rtspUrl, const std::string& useragent)
{
	internal = new RTSPClientInternal(handler, work, rtspUrl, useragent);
}
RTSPClient::~RTSPClient()
{
	stop();
	SAFE_DELETE(internal);
}
bool RTSPClient::initRTPOverTcpType()
{
	internal->rtspmedia.videoTransport.transport = TRANSPORT_INFO::TRANSPORT_RTP_TCP;
	internal->rtspmedia.videoTransport.rtp.t.videointerleaved = 0;
	internal->rtspmedia.videoTransport.rtp.t.audiointerleaved = 1;
	internal->rtspmedia.audioTransport = internal->rtspmedia.videoTransport;


	return true;
}
bool RTSPClient::initRTPOverUdpType(const AllockUdpPortCallback& allockport)
{
	internal->rtspmedia.videoTransport.transport = TRANSPORT_INFO::TRANSPORT_RTP_UDP;
	internal->rtspmedia.audioTransport.transport = TRANSPORT_INFO::TRANSPORT_RTP_UDP;

	internal->allockportcallback = allockport;

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
	return internal->sendTeradownRequest()->cmd;
}
bool RTSPClient::sendTeradownRequest(uint32_t timeout)
{
	shared_ptr<RTSPClientInternal::CommandInfo> cmdinfo = internal->sendTeradownRequest(timeout);
	if (!cmdinfo->waitsem->pend(timeout) || cmdinfo->responseheader == NULL)
	{
		return false;
	}

	return cmdinfo->responseheader->statuscode == 200;
}

}
}