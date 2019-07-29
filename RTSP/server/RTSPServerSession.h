#pragma  once
#include "RTSP/RTSPServer.h"
#include "../common/rtspProtocol.h"
#include "../common/sdpParse.h"
#include "../common/rtspTransport.h"
#include "../common/rtspRange.h"
#include "../common/WWW_Authenticate.h"
#include "../common/rtpovertcp.h"
#include "../common/rtpoverudp.h"

using namespace Public::RTSP;

#define OPTIONCMDSTR "DESCRIBE,SETUP,TEARDOWN,PLAY,PAUSE,OPTIONS,ANNOUNCE,RECORD"


struct RTSPServerSession::RTSPServerSessionInternal:public RTSPProtocol
{
	shared_ptr<IOWorker>					worker;

	bool									socketdisconnected;
	RTSPServer::ListenCallback				queryhandlercallback;
	
	weak_ptr<RTSPServerSession>				sessionptr;

	URL										url;
	std::string								password;
	std::string								username;
	shared_ptr<RTSPServerHandler>	handler;
	std::string								useragent;

	std::string								sessionstr;

	shared_ptr<rtp>							rtp;
	RTSP_MEDIA_INFO							rtspmedia;

	RTSPServerSessionInternal(const shared_ptr<IOWorker>& _worker, const shared_ptr<Socket>& sock, const RTSPServer::ListenCallback& queryhandle,const std::string&  _useragent)
		:RTSPProtocol(sock, RTSPProtocol::CommandCallback(&RTSPServerSessionInternal::rtspCommandCallback, this),
			RTSPProtocol::DisconnectCallback(&RTSPServerSessionInternal::socketDisconnectCallback, this),true)
		,worker(_worker), socketdisconnected(false), queryhandlercallback(queryhandle),useragent(_useragent)
	{
		memset(&rtspmedia, 0, sizeof(RTSP_MEDIA_INFO));
	}
	~RTSPServerSessionInternal() 
	{
		rtp = NULL;
		handler = NULL;
		if (m_sock) m_sock->disconnect();
	}

	void rtspCommandCallback(const shared_ptr<RTSPCommandInfo>& cmdinfo)
	{
		if (cmdinfo->method == "" || cmdinfo->cseq == 0) return sendErrorResponse(cmdinfo, 400, "Bad Request");
		if(strcasecmp(cmdinfo->verinfo.protocol.c_str(),"rtsp") != 0 || cmdinfo->verinfo.version != "1.0")
		{
			return sendErrorResponse(cmdinfo, 505, "RTSP Version Not Supported");
		}	

		shared_ptr<RTSPServerSession> session = sessionptr.lock();
		if (session == NULL)
		{
			socketdisconnected = true;
			return;
		}

		if (handler == NULL)
		{
			url = cmdinfo->url;

			handler = queryhandlercallback(session);
		}

		if (handler == NULL)
		{
			sendErrorResponse(cmdinfo, 500, "NOT SUPPORT");
			socketdisconnected = true;
			return;
		}

		if (username != "" || password != "")
		{
			std::string authenstr = cmdinfo->header("Authorization").readString();
			if(authenstr == "") return sendErrorResponse(cmdinfo, 401, "No Authorization");

			if (!WWW_Authenticate::checkAuthenticate(cmdinfo->method, username, password, authenstr))
			{
				return sendErrorResponse(cmdinfo, 403, "Forbidden");
			}
		}
		
		if (strcasecmp(cmdinfo->method.c_str(), "OPTIONS") == 0)
		{
			handler->onOptionRequest(session, cmdinfo);
		}
		else if (strcasecmp(cmdinfo->method.c_str(), "DESCRIBE") == 0)
		{
			handler->onDescribeRequest(session, cmdinfo);
		}
		else if (strcasecmp(cmdinfo->method.c_str(), "SETUP") == 0)
		{
			if(sessionstr == "") sessionstr = cmdinfo->header("Session").readString();
			std::string tranportstr = cmdinfo->header("Transport").readString();
			TRANSPORT_INFO transport;

			rtsp_header_parse_transport(cmdinfo->body.c_str(), &transport);
			handler->onSetupRequest(session, cmdinfo, transport);
		}
		else if (strcasecmp(cmdinfo->method.c_str(), "PLAY") == 0)
		{
			std::string rangestr = cmdinfo->header("Range").readString();
			
			RANGE_INFO range;
			rtsp_header_range(rangestr.c_str(), &range);
			handler->onPlayRequest(session, cmdinfo, range);
		}
		else if (strcasecmp(cmdinfo->method.c_str(), "PAUSE") == 0)
		{
			handler->onPauseRequest(session, cmdinfo);
		}
		else if (strcasecmp(cmdinfo->method.c_str(), "GET_PARAMETER"))
		{
			handler->onGetparameterRequest(session, cmdinfo, cmdinfo->body);
		}
		else if (strcasecmp(cmdinfo->method.c_str(), "TERADOWN") == 0)
		{
			handler->onTeardownRequest(session, cmdinfo);
		}
		else 
		{
			sendErrorResponse(cmdinfo, 500, "NOT SUPPORT");
		}
	}
	void rtpDataCallback(bool isvideo, uint32_t timestmap, const String& data)
	{
		if (handler) handler->onMediaCallback(isvideo, timestmap, data);
	}
	void socketDisconnectCallback()
	{
		socketdisconnected = true;
	}

	void sendOptionResponse(const shared_ptr<RTSPCommandInfo>& cmdinfo, const std::string& cmdstr)
	{
		std::string cmdstrtmp = cmdstr;
		if (cmdstrtmp.length() == 0) cmdstrtmp = OPTIONCMDSTR;

		HTTPParse::Header header;
		header.headers["Public"] = cmdstrtmp;

		sendProtocol(cmdinfo, header);
	}
	void sendDescribeResponse(const shared_ptr<RTSPCommandInfo>& cmdinfo, const MEDIA_INFO& mediainfo)
	{
		rtspmedia.media = mediainfo;

		sendProtocol(cmdinfo, HTTPParse::Header(), BuildSdp(mediainfo),RTSPCONENTTYPESDP);
	}
	void sendSetupResponse(const shared_ptr<RTSPCommandInfo>& cmdinfo, const TRANSPORT_INFO& transport)
	{
		TRANSPORT_INFO transporttmp = transport;
		transporttmp.ssrc = (uint32_t)Time::getCurrentTime().makeTime();

		HTTPParse::Header header;
		header.headers["Transport"] = rtsp_header_build_transport(transport);
		sendProtocol(cmdinfo, header);


		//build socket
		{
			if (transport.transport == TRANSPORT_INFO::TRANSPORT_RTP_TCP)
			{
				rtspmedia.videoTransport = rtspmedia.audioTransport = transport;

				//rtpOverTcp(bool _isserver, const shared_ptr<RTSPProtocol>& _protocol, const RTSP_MEDIA_INFO& _rtspmedia, const RTPDataCallback& _datacallback)
				rtp = make_shared<rtpOverTcp>(true, this, rtspmedia, rtp::RTPDataCallback(&RTSPServerSessionInternal::rtpDataCallback, this));
			}
			else if (rtspmedia.media.bHasVideo && String::indexOfByCase(cmdinfo->url, rtspmedia.media.stStreamVideo.szTrackID) != 0)
			{
				rtspmedia.videoTransport = transport;
			}
			else if (rtspmedia.media.bHasAudio && String::indexOfByCase(cmdinfo->url, rtspmedia.media.stStreamAudio.szTrackID) != 0)
			{
				rtspmedia.audioTransport = transport;
			}

			if (!(transport.transport != TRANSPORT_INFO::TRANSPORT_RTP_UDP ||
				(rtspmedia.media.bHasVideo && rtspmedia.videoTransport.transport == TRANSPORT_INFO::TRANSPORT_NONE) ||
				(rtspmedia.media.bHasAudio && rtspmedia.audioTransport.transport == TRANSPORT_INFO::TRANSPORT_NONE)))
			{
				//rtpOverUdp(bool _isserver, const shared_ptr<IOWorker>& work,const std::string& _dstaddr,const RTSP_MEDIA_INFO& _rtspmedia, const RTPDataCallback& _datacallback)
				rtp = make_shared<rtpOverUdp>(true,worker,m_sock->getOtherAddr().getIP(),rtspmedia, rtp::RTPDataCallback(&RTSPServerSessionInternal::rtpDataCallback, this));
			}
		}
	}
	void sendPlayResponse(const shared_ptr<RTSPCommandInfo>& cmdinfo)
	{
		sendProtocol(cmdinfo, HTTPParse::Header());
	}
	void sendPauseResponse(const shared_ptr<RTSPCommandInfo>& cmdinfo)
	{
		sendProtocol(cmdinfo, HTTPParse::Header());
	}
	void sendTeradownResponse(const shared_ptr<RTSPCommandInfo>& cmdinfo)
	{
		sendProtocol(cmdinfo, HTTPParse::Header());
	}
	void sendGetparameterResponse(const shared_ptr<RTSPCommandInfo>& cmdinfo, const std::string& content)
	{
		sendProtocol(cmdinfo, HTTPParse::Header(),content);
	}

	void sendErrorResponse(const shared_ptr<RTSPCommandInfo>& cmdinfo, int errcode, const std::string& errmsg)
	{
		HTTPParse::Header header;
		header.statuscode = errcode;
		header.statusmsg = errmsg;
		if (errcode == 401)
		{
			header.headers["WWW-Authenticate"] = WWW_Authenticate::buildWWWAuthenticate(cmdinfo->method, username, password);
		}
		
		sendProtocol(cmdinfo, header);
	}
	void sendProtocol(const shared_ptr<RTSPCommandInfo>& cmdinfo, HTTPParse::Header& header,const std::string& body = "",const std::string& contentype = "")
	{
		if (header.statuscode == 200) header.statusmsg = "OK";
		if (body.length() > 0)
		{
			header.headers[Content_Length] = body.length();
		}
		if (contentype.length() > 0)
		{
			header.headers[Content_Type] = contentype;
		}
		if (useragent.length() > 0) header.headers["User-Agent"] = useragent;
		if (cmdinfo != NULL && cmdinfo->cseq != 0) header.headers["CSeq"] = cmdinfo->cseq;
		if (sessionstr.length() > 0) header.headers["Session"] = sessionstr;

		std::string responsestr = std::string("RTSP/1.0 ") + Value(header.statuscode).readString() + " " + header.statusmsg + HTTPSEPERATOR;
		
		for (std::map<std::string, Value>::iterator iter = header.headers.begin(); iter != header.headers.end(); iter++)
		{
			responsestr += iter->first + ": " + iter->second.readString() + HTTPSEPERATOR;
		}
		responsestr += HTTPSEPERATOR;
		if (body.length() > 0) responsestr += body;

		RTSPProtocol::sendProtocolData(responsestr);
	}
};

RTSPServerSession::RTSPServerSession(const shared_ptr<IOWorker>& worker, const shared_ptr<Socket>& sock, const RTSPServer::ListenCallback& querycallback, const std::string& useragent)
{
	internal = new RTSPServerSessionInternal(worker,sock, querycallback,useragent);
	internal->useragent = useragent;
}
void RTSPServerSession::initRTSPServerSessionPtr(const weak_ptr<RTSPServerSession>& session)
{
	internal->sessionptr = session;
}

RTSPServerSession::~RTSPServerSession()
{
	SAFE_DELETE(internal);
}

void RTSPServerSession::setAuthenInfo(const std::string& username, const std::string& password)
{
	internal->username = username;
	internal->password = password;
}
const URL& RTSPServerSession::url() const
{
	return internal->url;
}
uint32_t RTSPServerSession::sendListSize() const
{
	return internal->sendListSize();
}
uint64_t RTSPServerSession::prevAlivetime() const
{
	if (internal->socketdisconnected) return 0;

	return internal->prevalivetime();
}
shared_ptr<RTSPServerHandler> RTSPServerSession::handler()
{
	return internal->handler;
}
void RTSPServerSession::sendOptionResponse(const shared_ptr<RTSPCommandInfo>& cmdinfo, const std::string& cmdstr)
{
	internal->sendOptionResponse(cmdinfo, cmdstr);
}
void RTSPServerSession::sendDescribeResponse(const shared_ptr<RTSPCommandInfo>& cmdinfo, const MEDIA_INFO& mediainfo)
{
	internal-> sendDescribeResponse(cmdinfo, mediainfo);
}
void RTSPServerSession::sendSetupResponse(const shared_ptr<RTSPCommandInfo>& cmdinfo, const TRANSPORT_INFO& transport)
{
	internal->sendSetupResponse(cmdinfo, transport);
}
void RTSPServerSession::sendPlayResponse(const shared_ptr<RTSPCommandInfo>& cmdinfo)
{
	internal->sendPlayResponse(cmdinfo);
}
void RTSPServerSession::sendPauseResponse(const shared_ptr<RTSPCommandInfo>& cmdinfo)
{
	internal->sendPauseResponse(cmdinfo);
}
void RTSPServerSession::sendTeradownResponse(const shared_ptr<RTSPCommandInfo>& cmdinfo)
{
	internal->sendTeradownResponse(cmdinfo);
}
void RTSPServerSession::sendGetparameterResponse(const shared_ptr<RTSPCommandInfo>& cmdinfo, const std::string& content)
{
	internal->sendGetparameterResponse(cmdinfo, content);
}

void RTSPServerSession::sendErrorResponse(const shared_ptr<RTSPCommandInfo>& cmdinfo, int errcode, const std::string& errmsg)
{
	internal->sendErrorResponse(cmdinfo, errcode, errmsg);
}
void RTSPServerSession::sendMedia(bool isvideo, uint32_t timestmap, const String& data)
{
	shared_ptr<rtp> rtptmp = internal->rtp;
	if (rtptmp)
	{
		rtptmp->sendData(isvideo, timestmap, data);
	}
}