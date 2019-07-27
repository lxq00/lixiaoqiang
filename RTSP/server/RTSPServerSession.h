#pragma  once
#include "RTSP/RTSPServer.h"
#include "../common/rtspProtocol.h"
#include "../common/sdpParse.h"
#include "../common/rtspTransport.h"
#include "../common/rtspRange.h"
#include "../common/rtspCMD.h"

struct RTSPSession::RTSPSessionInternal:public RTSPProtocol
{
	bool									socketdisconnected;
	QueryHandlerCallback					queryhandlercallback;
	QuerySessionCallback					querysessioncallback;

	URL										url;
	std::string								password;
	std::string								username;
	shared_ptr<RTSPCommandRequestHandler>	handler;
	std::string								useragent;

	std::string								sessionstr;

	RTSPSessionInternal(const shared_ptr<Socket>& sock, const QueryHandlerCallback& queryhandle,const QuerySessionCallback& querysession)
		:RTSPProtocol(sock, RTSPProtocol::CommandCallback(&RTSPSessionInternal::rtspCommandCallback, this),
			RTSPProtocol::ExternDataCallback(&RTSPSessionInternal::rtpovertcpCallback, this),
			RTSPProtocol::DisconnectCallback(&RTSPSessionInternal::socketDisconnectCallback, this),true)
		, socketdisconnected(false), queryhandlercallback(queryhandle),querysessioncallback(querysession)
	{

	}
	~RTSPSessionInternal() {}

	void rtspCommandCallback(const shared_ptr<HTTPParse::Header>& cmdheader, const std::string& cmdbody)
	{
		shared_ptr<RTSPCommandInfo> cmdinfo = make_shared<RTSPCommandInfo>();
		cmdinfo->code = cmdheader->header("CSeq").readInt();
		cmdinfo->method = cmdheader->method;
		cmdinfo->url = cmdheader->url;

		if (cmdheader->method == "" || cmdinfo->code == 0) return sendErrorResponse(cmdinfo, 400, "Bad Request");
		if(strcasecmp(cmdheader->verinfo.protocol.c_str(),"rtsp") != 0 || cmdheader->verinfo.version != "1.0") 
		{
			return sendErrorResponse(cmdinfo, 505, "RTSP Version Not Supported");
		}	

		if (handler == NULL)
		{
			url = cmdheader->url;

			handler = queryhandlercallback(m_sock.get());
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
		shared_ptr<RTSPSession> session = querysessioncallback(m_sock.get());

		if (strcasecmp(cmdheader->method.c_str(), "OPTIONS") == 0 && handler != NULL && session != NULL)
		{
			handler->onOptionRequest(session, cmdinfo);
		}
		else if (strcasecmp(cmdheader->method.c_str(), "DESCRIBE") == 0 && handler != NULL && session != NULL)
		{
			handler->onDescribeRequest(session, cmdinfo);
		}
		else if (strcasecmp(cmdheader->method.c_str(), "SETUP") == 0 && handler != NULL && session != NULL)
		{
			if(sessionstr == "") sessionstr = cmdheader->header("Session").readString();
			std::string tranportstr = cmdheader->header("Transport").readString();
			TRANSPORT_INFO transport;

			cmdinfo->session = sessionstr;
			rtsp_header_transport(cmdbody.c_str(), &transport);
			handler->onSetupRequest(session, cmdinfo, transport);
		}
		else if (strcasecmp(cmdheader->method.c_str(), "PLAY") == 0 && handler != NULL && session != NULL)
		{
			cmdinfo->session = sessionstr;

			std::string rangestr = cmdheader->header("Range").readString();
			
			RANGE_INFO range;
			rtsp_header_range(cmdbody.c_str(), &range);
			handler->onPlayRequest(session, cmdinfo, range);
		}
		else if (strcasecmp(cmdheader->method.c_str(), "PAUSE") == 0 && handler != NULL && session != NULL)
		{
			cmdinfo->session = sessionstr;

			handler->onPauseRequest(session, cmdinfo);
		}
		else if (strcasecmp(cmdheader->method.c_str(), "GET_PARAMETER") == 0 && handler != NULL && session != NULL)
		{
			cmdinfo->session = sessionstr;

			handler->onGetparameterRequest(session, cmdinfo,cmdbody);
		}
		else if (strcasecmp(cmdheader->method.c_str(), "TERADOWN") == 0 && handler != NULL && session != NULL)
		{
			cmdinfo->session = sessionstr;

			handler->onTeardownRequest(session, cmdinfo);
		}
		else 
		{
			sendErrorResponse(cmdinfo, 500, "NOT SUPPORT");
			socketdisconnected = true;
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
		header.headers["WWW-Authenticate"] = RTSPUrlInfo::buildWWWAuthen(cmdinfo->method, username, password);

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
		if (cmdinfo != NULL && cmdinfo->code != 0) header.headers["CSeq"] = cmdinfo->code;
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