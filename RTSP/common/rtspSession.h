#pragma once
#include "RTSP/RTSPClient.h"
#include "rtspProtocol.h"
#include "rtspHeaderSdp.h"
#include "rtspHeaderTransport.h"
#include "rtspHeaderRange.h"
#include "wwwAuthenticate.h"
#include "rtpovertcp.h"
#include "rtpoverudp.h"

using namespace Public::RTSP;

#define OPTIONCMDSTR "DESCRIBE,SETUP,TEARDOWN,PLAY,PAUSE,OPTIONS,ANNOUNCE,RECORD"

class RTSPSession
{
public:
	struct CommandInfo
	{
		shared_ptr<RTSPCommandInfo> cmd;
		uint64_t					starttime;
		uint64_t					timeout;

		std::string					wwwauthen;

		shared_ptr<Semaphore>		waitsem;
		shared_ptr<RTSPCommandInfo> responseheader;;

		CommandInfo(uint32_t _timeout = -1) :starttime(Time::getCurrentMilliSecond()), timeout(_timeout)
		{
			cmd = make_shared<RTSPCommandInfo>();
			if (_timeout != -1) waitsem = make_shared<Semaphore>();
		}
	};
public:
	shared_ptr<IOWorker>		ioworker;
	shared_ptr<Socket>			socket;
	std::string					useragent;
	shared_ptr<RTSPProtocol>	protocol;
	RTSPUrl						rtspurl;

	RTSP_MEDIA_INFO				rtspmedia;

	std::string					sessionstr;

	shared_ptr<rtp>				rtp;

	AllockUdpPortCallback		allockportcallback;

	uint32_t					protocolstartcseq;

	uint32_t					ssrc;

	RTSPSession()
	{
		protocolstartcseq = 0;
		rtspmedia.videoTransport.transport = TRANSPORT_INFO::TRANSPORT_RTP_TCP;
		rtspmedia.videoTransport.rtp.t.dataChannel = 0;
		rtspmedia.videoTransport.rtp.t.contorlChannel = 0;
		rtspmedia.audioTransport = rtspmedia.videoTransport;

		ssrc = (uint32_t)this;
	}
	~RTSPSession()
	{
		stop();
	}

	bool start(uint32_t timeout)
	{
		return true;
	}

	bool stop()
	{
		if (socket) socket->disconnect();
		protocol = NULL;
		rtp = NULL;
		socket = NULL;
		ioworker = NULL;

		return true;
	}
	shared_ptr<CommandInfo> sendOptionsRequest()
	{
		shared_ptr<CommandInfo> cmdinfo = make_shared<CommandInfo>();
		cmdinfo->cmd->method = "OPTIONS";
		cmdinfo->cmd->url = rtspurl.rtspurl;

		sendRequest(cmdinfo);

		return cmdinfo;
	}
	void sendOptionResponse(const shared_ptr<RTSPCommandInfo>& cmdinfo, const std::string& cmdstr)
	{
		std::string cmdstrtmp = cmdstr;
		if (cmdstrtmp.length() == 0) cmdstrtmp = OPTIONCMDSTR;

		HTTPParse::Header header;
		header.headers["Public"] = cmdstrtmp;

		sendResponse(cmdinfo, header);
	}
	shared_ptr<CommandInfo> sendPlayRequest(const RANGE_INFO& range, uint32_t timeout = -1)
	{
		shared_ptr<CommandInfo> cmdinfo = make_shared<CommandInfo>(timeout);
		cmdinfo->cmd->method = "PLAY";
		cmdinfo->cmd->url = rtspurl.rtspurl;
		cmdinfo->cmd->headers["Range"] = rtsp_header_build_range(range);

		sendRequest(cmdinfo);

		return cmdinfo;
	}
	void sendPlayResponse(const shared_ptr<RTSPCommandInfo>& cmdinfo)
	{
		sendResponse(cmdinfo, HTTPParse::Header());
	}
	shared_ptr<CommandInfo> sendPauseRequest(uint32_t timeout = -1)
	{
		shared_ptr<CommandInfo> cmdinfo = make_shared<CommandInfo>(timeout);
		cmdinfo->cmd->method = "PAUSE";
		cmdinfo->cmd->url = rtspurl.rtspurl;

		sendRequest(cmdinfo);

		return cmdinfo;
	}
	void sendPauseResponse(const shared_ptr<RTSPCommandInfo>& cmdinfo)
	{
		sendResponse(cmdinfo, HTTPParse::Header());
	}
	shared_ptr<CommandInfo> sendGetparameterRequest(const std::string& body, uint32_t timeout = -1)
	{
		shared_ptr<CommandInfo> cmdinfo = make_shared<CommandInfo>(timeout);
		cmdinfo->cmd->method = "GET_PARAMETER";
		cmdinfo->cmd->url = rtspurl.rtspurl;

		sendRequest(cmdinfo, body);

		return cmdinfo;
	}
	void sendGetparameterResponse(const shared_ptr<RTSPCommandInfo>& cmdinfo, const std::string& content)
	{
		sendResponse(cmdinfo, HTTPParse::Header(), content);
	}
	shared_ptr<CommandInfo> sendTeardownRequest(uint32_t timeout = -1)
	{
		shared_ptr<CommandInfo> cmdinfo = make_shared<CommandInfo>(timeout);
		cmdinfo->cmd->method = "TERADOWN";
		cmdinfo->cmd->url = rtspurl.rtspurl;

		sendRequest(cmdinfo);

		return cmdinfo;
	}
	void sendTeardownResponse(const shared_ptr<RTSPCommandInfo>& cmdinfo)
	{
		sendResponse(cmdinfo, HTTPParse::Header());
	}
	shared_ptr<CommandInfo> sendDescribeRequest()
	{
		shared_ptr<CommandInfo> cmdinfo = make_shared<CommandInfo>();
		cmdinfo->cmd->method = "DESCRIBE";
		cmdinfo->cmd->url = rtspurl.rtspurl;

		sendRequest(cmdinfo);

		return cmdinfo;
	}
	void sendDescribeResponse(const shared_ptr<RTSPCommandInfo>& cmdinfo, const MEDIA_INFO& mediainfo)
	{
		rtspmedia.media = mediainfo;
		rtspmedia.media.ssrc = ssrc;

		sendResponse(cmdinfo, HTTPParse::Header(), rtsp_header_build_sdp(rtspmedia.media), RTSPCONENTTYPESDP);
	}
	shared_ptr<CommandInfo> sendSetupRequest()
	{
		shared_ptr<CommandInfo> cmdinfo = make_shared<CommandInfo>();
		cmdinfo->cmd->method = "SETUP";

		if (rtspmedia.videoTransport.transport != TRANSPORT_INFO::TRANSPORT_RTP_TCP && rtspmedia.audioTransport.transport != TRANSPORT_INFO::TRANSPORT_RTP_TCP && rtspmedia.videoTransport.rtp.u.server_port1 == 0)
		{
			uint32_t startport = allockportcallback();

			rtspmedia.videoTransport.rtp.u.client_port1 = startport;
			rtspmedia.videoTransport.rtp.u.client_port2 = startport + 1;
			rtspmedia.audioTransport.rtp.u.client_port1 = startport + 2;
			rtspmedia.audioTransport.rtp.u.client_port2 = startport + 3;
		}

		if (rtspmedia.videoTransport.transport == TRANSPORT_INFO::TRANSPORT_RTP_TCP || rtspmedia.audioTransport.transport == TRANSPORT_INFO::TRANSPORT_RTP_TCP)
		{
			rtspmedia.videoTransport.rtp.t.dataChannel = 0;
			rtspmedia.videoTransport.rtp.t.contorlChannel = rtspmedia.videoTransport.rtp.t.dataChannel + 1;
			rtspmedia.audioTransport = rtspmedia.videoTransport;

			cmdinfo->cmd->url = rtspurl.rtspurl + "/" + (rtspmedia.media.bHasVideo ? rtspmedia.media.stStreamVideo.szTrackID : rtspmedia.media.stStreamAudio.szTrackID);
			cmdinfo->cmd->headers["Transport"] = rtsp_header_build_transport(rtspmedia.media.bHasVideo ? rtspmedia.videoTransport : rtspmedia.audioTransport);
		}
		else if (rtspmedia.media.bHasVideo && rtspmedia.videoTransport.rtp.u.server_port1 == 0)
		{
			cmdinfo->cmd->url = rtspurl.rtspurl + "/" + rtspmedia.media.stStreamVideo.szTrackID;
			cmdinfo->cmd->headers["Transport"] = rtsp_header_build_transport(rtspmedia.videoTransport);
		}
		else if (rtspmedia.media.bHasAudio && rtspmedia.audioTransport.rtp.u.server_port1 == 0)
		{
			cmdinfo->cmd->url = rtspurl.rtspurl + "/" + rtspmedia.media.stStreamAudio.szTrackID;
			cmdinfo->cmd->headers["Transport"] = rtsp_header_build_transport(rtspmedia.audioTransport);
		}
		else
		{
			return shared_ptr<CommandInfo>();
		}

		sendRequest(cmdinfo);

		return cmdinfo;
	}
	void sendSetupResponse(const shared_ptr<RTSPCommandInfo>& cmdinfo, const TRANSPORT_INFO& transporttmp)
	{
		TRANSPORT_INFO transport = transporttmp;
		transport.ssrc = ssrc;

		bool isVideo = true;

		//build socket
		{
			if (transport.transport == TRANSPORT_INFO::TRANSPORT_RTP_TCP)
			{
				if(protocol)
					protocol->setTCPInterleavedChannel(rtspmedia.videoTransport.rtp.t.dataChannel, rtspmedia.audioTransport.rtp.t.contorlChannel);

				rtspmedia.videoTransport = rtspmedia.audioTransport = transport;

				//rtpOverTcp(bool _isserver, const shared_ptr<RTSPProtocol>& _protocol, const RTSP_MEDIA_INFO& _rtspmedia, const RTPDataCallback& _datacallback)
				rtp = make_shared<rtpOverTcp>(true, protocol.get(), rtspmedia, rtp::RTPDataCallback(&RTSPSession::rtpDataCallback, this));
			}
			else if (rtspmedia.media.bHasVideo && String::indexOfByCase(cmdinfo->url, rtspmedia.media.stStreamVideo.szTrackID) != -1)
			{
				isVideo = true;
				rtspmedia.videoTransport.transport = TRANSPORT_INFO::TRANSPORT_RTP_UDP;
				rtspmedia.videoTransport.rtp.u.client_port1 = transport.rtp.u.client_port1;
				rtspmedia.videoTransport.rtp.u.client_port2 = transport.rtp.u.client_port2;
				rtspmedia.videoTransport.ssrc = transport.ssrc;
			}
			else if (rtspmedia.media.bHasAudio && String::indexOfByCase(cmdinfo->url, rtspmedia.media.stStreamAudio.szTrackID) != -1)
			{
				isVideo = false;
				rtspmedia.audioTransport.transport = TRANSPORT_INFO::TRANSPORT_RTP_UDP;
				rtspmedia.audioTransport.rtp.u.client_port1 = transport.rtp.u.client_port1;
				rtspmedia.audioTransport.rtp.u.client_port2 = transport.rtp.u.client_port2;
				rtspmedia.audioTransport.ssrc = transport.ssrc;
			}

			if (transport.transport != TRANSPORT_INFO::TRANSPORT_RTP_TCP && rtspmedia.videoTransport.rtp.u.server_port1 == 0)
			{
				uint32_t startport = allockportcallback();

				rtspmedia.videoTransport.rtp.u.server_port1 = startport;
				rtspmedia.videoTransport.rtp.u.server_port2 = startport + 1;
				rtspmedia.audioTransport.rtp.u.server_port1 = startport + 2;
				rtspmedia.audioTransport.rtp.u.server_port2 = startport + 2;
			}

			if (!(transport.transport != TRANSPORT_INFO::TRANSPORT_RTP_UDP ||
				(rtspmedia.media.bHasVideo && rtspmedia.videoTransport.rtp.u.client_port1 == 0) ||
				(rtspmedia.media.bHasAudio && rtspmedia.audioTransport.rtp.u.client_port1 == 0)))
			{
				//rtpOverUdp(bool _isserver, const shared_ptr<IOWorker>& work,const std::string& _dstaddr,const RTSP_MEDIA_INFO& _rtspmedia, const RTPDataCallback& _datacallback)
				rtp = make_shared<rtpOverUdp>(true, ioworker, socket->getOtherAddr().getIP(), rtspmedia, rtp::RTPDataCallback(&RTSPSession::rtpDataCallback, this));
			}
		}

		HTTPParse::Header header;
		header.headers["Transport"] = rtsp_header_build_transport(isVideo ? rtspmedia.videoTransport : rtspmedia.audioTransport);
		sendResponse(cmdinfo, header);
	}
	bool sendMedia(bool isvideo, uint32_t timestmap, const char* buffer,uint32_t len, bool mark)
	{
		if (!rtp) return false;

		rtp->sendData(isvideo, timestmap, buffer,len, mark);

		return true;
	}
	void sendErrorResponse(const shared_ptr<RTSPCommandInfo>& cmdinfo, int errcode, const std::string& errmsg)
	{
		HTTPParse::Header header;
		header.statuscode = errcode;
		header.statusmsg = errmsg;
		if (errcode == 401)
		{
			header.headers["WWW-Authenticate"] = WWW_Authenticate::buildWWWAuthenticate(cmdinfo->method, rtspurl.username, rtspurl.password);
		}

		sendResponse(cmdinfo, header);
	}
	void sendResponse(const shared_ptr<RTSPCommandInfo>& cmdinfo, HTTPParse::Header& header, const std::string& body = "", const std::string& contentype = "")
	{
		shared_ptr<RTSPCommandInfo> respcmd = make_shared<RTSPCommandInfo>();
		respcmd->statuscode = header.statuscode;
		respcmd->statusmsg = header.statusmsg;
		respcmd->headers = header.headers;
		respcmd->cseq = cmdinfo->cseq;

		_rtspBuildRtspCommand(respcmd, body, contentype);
	}

	void sendRequest(const shared_ptr<CommandInfo>& cmdinfo, const std::string& body = "", const std::string& contentype = "")
	{
		onSendRequestCallback(cmdinfo);

		if(cmdinfo->cmd->url.length() <= 0)	cmdinfo->cmd->url = rtspurl.rtspurl;
		if (cmdinfo->wwwauthen.length() > 0)
		{
			cmdinfo->cmd->headers["Authorization"] = WWW_Authenticate::buildAuthorization(cmdinfo->cmd->method, rtspurl.username, rtspurl.password, cmdinfo->cmd->url, cmdinfo->wwwauthen);
		}
		_rtspBuildRtspCommand(cmdinfo->cmd, body, contentype);
	}
private:
	virtual void rtpDataCallback(bool isvideo, uint32_t timestmap, const char* buffer,uint32_t bufferlen, bool mark) = 0;

	void _rtspBuildRtspCommand(const shared_ptr<RTSPCommandInfo>& cmd, const std::string& body = "", const std::string& contentype = "")
	{
		if (body.length() > 0)
		{
			cmd->headers[Content_Length] = body.length();
		}
		if (body.length() > 0 && contentype.length() > 0)
		{
			cmd->headers[Content_Type] = contentype;
		}
		if (useragent.length() > 0) cmd->headers["User-Agent"] = useragent;
		if (sessionstr.length() > 0) cmd->headers["Session"] = sessionstr;

		if (cmd->cseq == 0)
		{
			do
			{
				protocolstartcseq++;
			} while (protocolstartcseq == 0);
			cmd->cseq = protocolstartcseq;
		}

		cmd->headers["CSeq"] = cmd->cseq;		

		std::string cmdstr;
		
		if (cmd->url.length() > 0)
		{
			cmdstr = cmd->method + " " + cmd->url + " RTSP/1.0" + HTTPSEPERATOR;
		}
		else
		{
			if (cmd->statuscode == 200) cmd->statusmsg = "0K";
			cmdstr = std::string("RTSP/1.0 ") + Value(cmd->statuscode).readString() + " " + cmd->statusmsg + HTTPSEPERATOR;
		}
		for (std::map<std::string, Value>::iterator iter = cmd->headers.begin(); iter != cmd->headers.end(); iter++)
		{
			cmdstr += iter->first + ": " + iter->second.readString() + HTTPSEPERATOR;
		}
		cmdstr += HTTPSEPERATOR;
		if (body.length() > 0) cmdstr += body;


		shared_ptr<RTSPProtocol> protocoltmp = protocol;
		if (protocoltmp) protocol->sendProtocolData(cmdstr);
	}

	virtual void onSendRequestCallback(const shared_ptr<CommandInfo>& cmd){}
};
