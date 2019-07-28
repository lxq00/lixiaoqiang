#pragma  once
#include "RTSP/RTSPServer.h"
#include "../common/rtspProtocol.h"
#include "../common/sdpParse.h"
#include "../common/rtspTransport.h"
#include "../common/rtspRange.h"
#include "../common/rtspCMD.h"

using namespace Public::RTSP;

struct RTSPSession::RTSPSessionInternal:public RTSPProtocol
{
	bool									socketdisconnected;
	RTSPServer::ListenCallback				queryhandlercallback;
	
	weak_ptr<RTSPSession>					sessionptr;

	URL										url;
	std::string								password;
	std::string								username;
	shared_ptr<RTSPCommandRequestHandler>	handler;
	std::string								useragent;

	std::string								sessionstr;

	RTSPSessionInternal(const shared_ptr<Socket>& sock, const RTSPServer::ListenCallback& queryhandle,const std::string&  _useragent)
		:RTSPProtocol(sock, RTSPProtocol::CommandCallback(&RTSPSessionInternal::rtspCommandCallback, this),
			RTSPProtocol::ExternDataCallback(&RTSPSessionInternal::rtpovertcpCallback, this),
			RTSPProtocol::DisconnectCallback(&RTSPSessionInternal::socketDisconnectCallback, this),true)
		, socketdisconnected(false), queryhandlercallback(queryhandle),useragent(_useragent)
	{

	}
	~RTSPSessionInternal() {}

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

		shared_ptr<RTSPSession> session = sessionptr.lock();
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
	void rtpovertcpCallback(bool isvideo, const char* data, uint32_t len)
	{

	}
	void socketDisconnectCallback()
	{
		socketdisconnected = true;
	}

	void sendOptionResponse(const shared_ptr<RTSPCommandInfo>& cmdinfo, const std::string& cmdstr)
	{
		HTTPParse::Header header;
		header.headers["Public"] = cmdstr;

		sendProtocol(cmdinfo, header);
	}
	void sendDescribeResponse(const shared_ptr<RTSPCommandInfo>& cmdinfo, const MEDIA_INFO& mediainfo)
	{
		sendProtocol(cmdinfo, HTTPParse::Header(), BuildSdp(mediainfo));
	}
	void sendSetupResponse(const shared_ptr<RTSPCommandInfo>& cmdinfo, const TRANSPORT_INFO& transport)
	{
		//build socket

		HTTPParse::Header header;
		header.headers["Transport"] = buildTransport(transport);
		sendProtocol(cmdinfo, header);
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

	void sendProtocol(const shared_ptr<RTSPCommandInfo>& cmdinfo, HTTPParse::Header& header,const std::string& body = "")
	{
		if (header.statuscode == 200) header.statusmsg = "OK";
		if (body.length() > 0)
		{
			header.headers[Content_Length] = body.length();
			header.headers[Content_Type] = "application/sdp";
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

RTSPSession::RTSPSession(const shared_ptr<Socket>& sock, const RTSPServer::ListenCallback& querycallback, const std::string& useragent)
{
	internal = new RTSPSessionInternal(sock, querycallback,useragent);
	internal->useragent = useragent;
}
void RTSPSession::initRTSPSessionPtr(const weak_ptr<RTSPSession>& session)
{
	internal->sessionptr = session;
}

RTSPSession::~RTSPSession()
{
	SAFE_DELETE(internal);
}

void RTSPSession::setAuthenInfo(const std::string& username, const std::string& password)
{
	internal->username = username;
	internal->password = password;
}
const URL& RTSPSession::url() const
{
	return internal->url;
}
uint32_t RTSPSession::sendListSize() const
{
	return internal->sendListSize();
}
uint64_t RTSPSession::prevAlivetime() const
{
	if (internal->socketdisconnected) return 0;

	return internal->prevalivetime();
}
shared_ptr<RTSPCommandRequestHandler> RTSPSession::handler()
{
	return internal->handler;
}
void RTSPSession::sendOptionResponse(const shared_ptr<RTSPCommandInfo>& cmdinfo, const std::string& cmdstr)
{
	internal->sendOptionResponse(cmdinfo, cmdstr);
}
void RTSPSession::sendDescribeResponse(const shared_ptr<RTSPCommandInfo>& cmdinfo, const MEDIA_INFO& mediainfo)
{
	internal-> sendDescribeResponse(cmdinfo, mediainfo);
}
void RTSPSession::sendSetupResponse(const shared_ptr<RTSPCommandInfo>& cmdinfo, const TRANSPORT_INFO& transport)
{
	internal->sendSetupResponse(cmdinfo, transport);
}
void RTSPSession::sendPlayResponse(const shared_ptr<RTSPCommandInfo>& cmdinfo)
{
	internal->sendPlayResponse(cmdinfo);
}
void RTSPSession::sendPauseResponse(const shared_ptr<RTSPCommandInfo>& cmdinfo)
{
	internal->sendPauseResponse(cmdinfo);
}
void RTSPSession::sendTeradownResponse(const shared_ptr<RTSPCommandInfo>& cmdinfo)
{
	internal->sendTeradownResponse(cmdinfo);
}
void RTSPSession::sendGetparameterResponse(const shared_ptr<RTSPCommandInfo>& cmdinfo, const std::string& content)
{
	internal->sendGetparameterResponse(cmdinfo, content);
}

void RTSPSession::sendErrorResponse(const shared_ptr<RTSPCommandInfo>& cmdinfo, int errcode, const std::string& errmsg)
{
	internal->sendErrorResponse(cmdinfo, errcode, errmsg);
}
void RTSPSession::sendMedia(bool video, const FRAME_INFO& info, const char* buffer, uint32_t bufferlen)
{
	internal->sendMedia(video, info, buffer, bufferlen);
}