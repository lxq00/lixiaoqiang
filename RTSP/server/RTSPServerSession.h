#pragma  once
#include "RTSP/RTSPServer.h"
#include "../common/rtspProtocol.h"
#include "../common/sdpParse.h"
#include "../common/rtspTransport.h"
#include "../common/rtspRange.h"
#include "../common/rtspCMD.h"
#include "../common/rtpovertcp.h"
#include "../common/rtpoverudp.h"

using namespace Public::RTSP;

#define OPTIONCMDSTR "DESCRIBE,SETUP,TEARDOWN,PLAY,PAUSE,OPTIONS,ANNOUNCE,RECORD"


struct RTSPServerSession::RTSPServerSessionInternal:public RTSPProtocol
{
	shared_ptr<IOWorker>					worker;

	bool									socketdisconnected;
	RTSPServer::ListenCallback				queryhandlercallback;
	
	weak_ptr<RTSPServerSession>					sessionptr;

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

	void rtspCommandCallback(const shared_ptr<HTTPParse::Header>& cmdheader, const std::string& cmdbody)
	{
		shared_ptr<RTSPCommandInfo> cmdinfo = make_shared<RTSPCommandInfo>();
		*(HTTPParse::Header*)cmdinfo.get() = *cmdheader.get();
		cmdinfo->CSeq = cmdheader->header("CSeq").readInt();
		cmdinfo->body = cmdbody;
		
		if (cmdheader->method == "" || cmdinfo->CSeq == 0) return sendErrorResponse(cmdinfo, 400, "Bad Request");
		if(strcasecmp(cmdheader->verinfo.protocol.c_str(),"rtsp") != 0 || cmdheader->verinfo.version != "1.0") 
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
			url = cmdheader->url;

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
			std::string authenstr = cmdheader->header("Authorization").readString();
			if(authenstr == "") return sendErrorResponse(cmdinfo, 401, "No Authorization");

			if (!RTSPUrlInfo::checkAuthen(cmdheader->method, username, password, authenstr))
			{
				return sendErrorResponse(cmdinfo, 403, "Forbidden");
			}
		}
		
		if (strcasecmp(cmdheader->method.c_str(), "OPTIONS") == 0)
		{
			handler->onOptionRequest(session, cmdinfo);
		}
		else if (strcasecmp(cmdheader->method.c_str(), "DESCRIBE") == 0)
		{
			handler->onDescribeRequest(session, cmdinfo);
		}
		else if (strcasecmp(cmdheader->method.c_str(), "SETUP") == 0)
		{
			if(sessionstr == "") sessionstr = cmdheader->header("Session").readString();
			std::string tranportstr = cmdheader->header("Transport").readString();
			TRANSPORT_INFO transport;

			cmdinfo->session = sessionstr;
			rtsp_header_transport(cmdbody.c_str(), &transport);
			handler->onSetupRequest(session, cmdinfo, transport);
		}
		else if (strcasecmp(cmdheader->method.c_str(), "PLAY") == 0)
		{
			cmdinfo->session = sessionstr;

			std::string rangestr = cmdheader->header("Range").readString();
			
			RANGE_INFO range;
			rtsp_header_range(cmdbody.c_str(), &range);
			handler->onPlayRequest(session, cmdinfo, range);
		}
		else if (strcasecmp(cmdheader->method.c_str(), "PAUSE") == 0)
		{
			cmdinfo->session = sessionstr;

			handler->onPauseRequest(session, cmdinfo);
		}
		else if (strcasecmp(cmdheader->method.c_str(), "GET_PARAMETER"))
		{
			cmdinfo->session = sessionstr;

			handler->onGetparameterRequest(session, cmdinfo,cmdbody);
		}
		else if (strcasecmp(cmdheader->method.c_str(), "TERADOWN") == 0)
		{
			cmdinfo->session = sessionstr;

			handler->onTeardownRequest(session, cmdinfo);
		}
		else 
		{
			sendErrorResponse(cmdinfo, 500, "NOT SUPPORT");
		}
	}
	void rtpDataCallback(bool isvideo, uint32_t timestmap, const char* buffer, uint32_t bufferlen)
	{
		if (handler) handler->onMediaCallback(isvideo, timestmap, buffer, bufferlen);
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
		header.headers["Transport"] = buildTransport(transport);
		sendProtocol(cmdinfo, header);


		//build socket
		{
			if (transport.transport == TRANSPORT_INFO::TRANSPORT_RTP_TCP)
			{
				rtspmedia.videoTransport = rtspmedia.audioTransport = transport;

				//rtpOverTcp(bool _isserver, const shared_ptr<RTSPProtocol>& _protocol, const RTSP_MEDIA_INFO& _rtspmedia, const RTPDataCallback& _datacallback)
				rtp = make_shared<rtpOverTcp>(true, this, rtspmedia, rtp::RTPDataCallback(&RTSPServerSessionInternal::rtpDataCallback, this));
			}
			else if (rtspmedia.media.bHasVideo && String::indexOfByCase(cmdinfo->url.href(), rtspmedia.media.stStreamVideo.szTrackID) != 0)
			{
				rtspmedia.videoTransport = transport;
			}
			else if (rtspmedia.media.bHasAudio && String::indexOfByCase(cmdinfo->url.href(), rtspmedia.media.stStreamAudio.szTrackID) != 0)
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
			header.headers["WWW-Authenticate"] = RTSPUrlInfo::buildWWWAuthen(cmdinfo->method, username, password);
		}
		
		sendProtocol(cmdinfo, header);
	}
	void sendMedia(bool video, const FRAME_INFO& info, const char* buffer, uint32_t bufferlen)
	{

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
		if (cmdinfo != NULL && cmdinfo->CSeq != 0) header.headers["CSeq"] = cmdinfo->CSeq;
		if (cmdinfo->session.length() > 0) header.headers["Session"] = cmdinfo->session;

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
void RTSPServerSession::sendMedia(bool isvideo, uint32_t timestmap, const char* buffer, uint32_t bufferlen)
{
	shared_ptr<rtp> rtptmp = internal->rtp;
	if (rtptmp)
	{
		rtptmp->sendData(isvideo, timestmap, buffer, bufferlen);
	}
}