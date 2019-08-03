#pragma once
#include "RTSP/RTSPClient.h"
#include "rtspProtocol.h"
#include "StringParse/rtspHeaderSdp.h"
#include "StringParse/rtspHeaderTransport.h"
#include "StringParse/rtspHeaderRange.h"
#include "wwwAuthenticate.h"
#include "rtp/rtpovertcpSession.h"
#include "rtp/rtpoverudpSession.h"

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

	shared_ptr<MEDIA_INFO>		rtspmedia;

	std::string					sessionstr;	

	AllockUdpPortCallback		allockportcallback;

	uint32_t					protocolstartcseq;

	uint32_t					ssrc;

	bool						transportbytcp;

	bool						isserver;
public:
	RTSPSession(bool server):isserver(server)
	{
		protocolstartcseq = 0;
		transportbytcp = true;
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
		socket = NULL;
		ioworker = NULL;
		rtspmedia = NULL;

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
		rtspmedia = shared_ptr<MEDIA_INFO>(new MEDIA_INFO(mediainfo));
		rtspmedia->ssrc = Value(ssrc).readString();

		sendResponse(cmdinfo, HTTPParse::Header(), rtsp_header_build_sdp(*rtspmedia.get()), RTSPCONENTTYPESDP);
	}
	shared_ptr<CommandInfo> sendSetupRequest(const shared_ptr<STREAM_TRANS_INFO>& transport)
	{
		shared_ptr<CommandInfo> cmdinfo = make_shared<CommandInfo>();
		cmdinfo->cmd->method = "SETUP";
		cmdinfo->cmd->url = rtspurl.rtspurl + "/" + transport->streaminfo.szTrackID;
		cmdinfo->cmd->headers["Transport"] = rtsp_header_build_transport(transport->transportinfo);

		sendRequest(cmdinfo);

		return cmdinfo;
	}
	void sendSetupResponse(const shared_ptr<RTSPCommandInfo>& cmdinfo, const shared_ptr<STREAM_TRANS_INFO>& transporttmp)
	{
		transporttmp->transportinfo.ssrc = (int)(ssrc | Time::getCurrentMilliSecond());
		if (transporttmp->transportinfo.transport == TRANSPORT_INFO::TRANSPORT_RTP_UDP)
		{
			uint32_t startport = allockportcallback();
			transporttmp->transportinfo.rtp.u.server_port1 = startport;
			transporttmp->transportinfo.rtp.u.server_port2 = startport + 1;
		}

		HTTPParse::Header header;
		header.headers["Transport"] = rtsp_header_build_transport(transporttmp->transportinfo);
		sendResponse(cmdinfo, header);
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
protected:
	void buildRtpSession(bool isserver)
	{
		protocol->setMediaTransportInfo(rtspmedia);
		
		for (std::list< shared_ptr<STREAM_TRANS_INFO> >::iterator iter = rtspmedia->infos.begin(); iter != rtspmedia->infos.end(); iter++)
		{
			shared_ptr<STREAM_TRANS_INFO> transport = *iter;
			
			if (transport->rtpsession) break;

			if (transport->transportinfo.transport == TRANSPORT_INFO::TRANSPORT_RTP_TCP)
			{
				//rtpOverTcpSesssion(const shared_ptr<RTSPProtocol>& _protocol, const shared_ptr<STREAM_TRANS_INFO>& _transport, const MediaDataCallback& _datacallback, const ContorlDataCallback& _contorlcallback)
				transport->rtpsession = make_shared<rtpOverTcpSesssion>(transport,
					rtpOverTcpSesssion::SendMediaDataCallback(&RTSPProtocol::sendMedia,protocol),rtpOverTcpSesssion::SendContrlDataCallback(&RTSPProtocol::sendContrlData,protocol),
					RTPSession::MediaDataCallback(&RTSPSession::onMediaDataCallback,this),RTPSession::ContorlDataCallback(&RTSPSession::onContorlDataCallback,this));
			}
			else if (transport->transportinfo.transport == TRANSPORT_INFO::TRANSPORT_RTP_UDP)
			{
				std::string dstaddr;
				if (isserver)
				{
					dstaddr = socket->getOtherAddr().getIP();
				}
				else
				{
					dstaddr = rtspurl.serverip;
				}

				//rtpOverUdpSession(bool _isserver, const shared_ptr<IOWorker>& ioworker,const std::string& _dstaddr, const shared_ptr<STREAM_TRANS_INFO>& _transport, const RTPSession::MediaDataCallback& _datacallback, const RTPSession::ContorlDataCallback& _contorlcallback)
				transport->rtpsession = make_shared<rtpOverUdpSession>(isserver,ioworker, dstaddr,transport,
					RTPSession::MediaDataCallback(&RTSPSession::onMediaDataCallback, this),
					RTPSession::ContorlDataCallback(&RTSPSession::onContorlDataCallback, this));
			}
			else
			{
				continue;
			}
		}
	}
private:
	virtual void onContorlDataCallback(const shared_ptr<STREAM_TRANS_INFO>& transinfo, const char* buffer, uint32_t len) = 0;
	virtual void onMediaDataCallback(const shared_ptr<STREAM_TRANS_INFO>& transinfo, const RTPHEADER& rtpheader, const char* buffer, uint32_t len) = 0;
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
		cmd->url = cmd->url;

		std::string cmdstr = HTTPBuild::build(!isserver,*(HTTPHeader*)cmd.get());
		if (body.length() > 0) cmdstr += body;


		shared_ptr<RTSPProtocol> protocoltmp = protocol;
		if (protocoltmp) protocol->sendProtocolData(cmdstr);
	}

	virtual void onSendRequestCallback(const shared_ptr<CommandInfo>& cmd){}
};
