#pragma  once
#include "RTSP/RTSPServer.h"
#include "../common/rtspSession.h"

using namespace Public::RTSP;


struct RTSPServerSession::RTSPServerSessionInternal:public RTSPSession
{
	bool									socketdisconnected;
	RTSPServer::ListenCallback				queryhandlercallback;
	
	shared_ptr<RTSPServerHandler>			handler;
	
	bool									sessionHaveAuthen;

	weak_ptr<RTSPServerSession>				sessionptr;

	RTSPServerSessionInternal(const shared_ptr<IOWorker>& _worker, const shared_ptr<Socket>& sock, const RTSPServer::ListenCallback& queryhandle, const AllockUdpPortCallback& allocport, const std::string&  _useragent)
		:socketdisconnected(false), queryhandlercallback(queryhandle),sessionHaveAuthen(false)
	{
		ssrc = (uint32_t)Time::getCurrentTime().makeTime();
		ioworker = _worker;
		socket = sock;
		allockportcallback = allocport;

		protocol = make_shared<RTSPProtocol>(sock, RTSPProtocol::CommandCallback(&RTSPServerSessionInternal::rtspCommandCallback, this),
			RTSPProtocol::DisconnectCallback(&RTSPServerSessionInternal::socketDisconnectCallback, this), true);
	}
	~RTSPServerSessionInternal() 
	{
		handler = NULL;
		RTSPSession::stop();
	}

	void rtspCommandCallback(const shared_ptr<RTSPCommandInfo>& cmdinfo)
	{
		if (cmdinfo->method == "" || cmdinfo->header("CSeq").empty()) return sendErrorResponse(cmdinfo, 400, "Bad Request");
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
			rtspurl = cmdinfo->url;

			handler = queryhandlercallback(session);
		}

		if (handler == NULL)
		{
			sendErrorResponse(cmdinfo, 500, "NOT SUPPORT");
			socketdisconnected = true;
			return;
		}

		std::string nowsession = cmdinfo->header("Session").readString();
		if (sessionstr.length() > 0 && sessionstr != nowsession)
		{
			sendErrorResponse(cmdinfo, 451, "Session Error");
			return;
		}

		//check authen info
		if (rtspurl.username != "" || rtspurl.password != "")
		{
			std::string authenstr = cmdinfo->header("Authorization").readString();
			if (authenstr.length() <= 0 && !sessionHaveAuthen)
			{
				return sendErrorResponse(cmdinfo, 401, "No Authorization");
			}
			else if (authenstr.length() > 0)
			{
				if (!WWW_Authenticate::checkAuthenticate(cmdinfo->method, rtspurl.username, rtspurl.password, authenstr))
				{
					return sendErrorResponse(cmdinfo, 403, "Forbidden");
				}
				sessionHaveAuthen = true;
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
			if (sessionstr.length() <= 0)
			{
				sessionstr = Value(Time::getCurrentMilliSecond()).readString() + Value(Time::getCurrentTime().makeTime()).readString();
			}

			std::string tranportstr = cmdinfo->header("Transport").readString();

			TRANSPORT_INFO transport;
			rtsp_header_parse_transport(tranportstr.c_str(), &transport);

			shared_ptr<STREAM_TRANS_INFO> transportinfo;
			for (std::list<shared_ptr<STREAM_TRANS_INFO> >::const_iterator iter = rtspmedia->infos.begin(); iter != rtspmedia->infos.end(); iter++)
			{
				if (String::indexOfByCase(cmdinfo->url, (*iter)->streaminfo.szTrackID) != -1)
				{
					transportinfo = *iter;
					break;
				}
			}

			if (transportinfo == NULL)
			{
				sendErrorResponse(cmdinfo, 501, "NO THIS TRACK");
			}
			else
			{
				transportinfo->transportinfo = transport;

				handler->onSetupRequest(session, cmdinfo, transportinfo);
			}
		}
		else if (strcasecmp(cmdinfo->method.c_str(), "PLAY") == 0)
		{
			if (rtplist.size() == 0) buildRtpSession(true);


			std::string rangestr = cmdinfo->header("Range").readString();
			
			RANGE_INFO range;
			rtsp_header_parse_range(rangestr.c_str(), &range);
			handler->onPlayRequest(session, cmdinfo, range);

			buildRtpSession(true);
		}
		else if (strcasecmp(cmdinfo->method.c_str(), "PAUSE") == 0)
		{
			handler->onPauseRequest(session, cmdinfo);
		}
		else if (strcasecmp(cmdinfo->method.c_str(), "GET_PARAMETER") == 0)
		{
			handler->onGetparameterRequest(session, cmdinfo, cmdinfo->body);
		}
		else if (strcasecmp(cmdinfo->method.c_str(), "TEARDOWN") == 0)
		{
			handler->onTeardownRequest(session, cmdinfo);
		}
		else 
		{
			sendErrorResponse(cmdinfo, 500, "NOT SUPPORT");
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
		socketdisconnected = true;
	}
};

RTSPServerSession::RTSPServerSession(const shared_ptr<IOWorker>& worker, const shared_ptr<Socket>& sock, const RTSPServer::ListenCallback& querycallback, const AllockUdpPortCallback& allocport, const std::string& useragent)
{
	internal = new RTSPServerSessionInternal(worker,sock, querycallback,allocport,useragent);
}
void RTSPServerSession::initRTSPServerSessionPtr(const weak_ptr<RTSPServerSession>& session)
{
	internal->sessionptr = session;
}

RTSPServerSession::~RTSPServerSession()
{
	SAFE_DELETE(internal);
}

void RTSPServerSession::disconnect()
{
	internal->stop();
	internal->handler = NULL;
	internal->rtplist.clear();
}

void RTSPServerSession::setAuthenInfo(const std::string& username, const std::string& password)
{
	internal->rtspurl.username = username;
	internal->rtspurl.password = password;
}
const std::string& RTSPServerSession::url() const
{
	return internal->rtspurl.rtspurl;
}
uint32_t RTSPServerSession::sendListSize() const
{
	if (!internal->protocol) return 0;

	return internal->protocol->sendListSize();
}
uint64_t RTSPServerSession::prevAlivetime() const
{
	if (internal->socketdisconnected) return 0;

	if (!internal->protocol) return 0;

	return internal->protocol->prevalivetime();
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
void RTSPServerSession::sendSetupResponse(const shared_ptr<RTSPCommandInfo>& cmdinfo, const shared_ptr<STREAM_TRANS_INFO>& transport)
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
void RTSPServerSession::sendTeardownResponse(const shared_ptr<RTSPCommandInfo>& cmdinfo)
{
	internal->sendTeardownResponse(cmdinfo);
}
void RTSPServerSession::sendGetparameterResponse(const shared_ptr<RTSPCommandInfo>& cmdinfo, const std::string& content)
{
	internal->sendGetparameterResponse(cmdinfo, content);
}

void RTSPServerSession::sendErrorResponse(const shared_ptr<RTSPCommandInfo>& cmdinfo, int errcode, const std::string& errmsg)
{
	internal->sendErrorResponse(cmdinfo, errcode, errmsg);
}
bool RTSPServerSession::sendMediaPackage(const shared_ptr<STREAM_TRANS_INFO> mediainfo, uint32_t timestmap, const char*  buffer, uint32_t bufferlen, bool mark)
{
	std::map<STREAM_TRANS_INFO*, shared_ptr<RTPSession> >::iterator iter = internal->rtplist.find(mediainfo.get());
	if (iter == internal->rtplist.end()) return false;

	shared_ptr<RTPSession> rtpsession = iter->second;
	if (rtpsession)
	{
		rtpsession->sendMediaData(mediainfo, timestmap, buffer, bufferlen, mark);
	}

	return (bool)rtpsession;
}
bool RTSPServerSession::sendContorlPackage(const shared_ptr<STREAM_TRANS_INFO> mediainfo, const char*  buffer, uint32_t bufferlen)
{
	std::map<STREAM_TRANS_INFO*, shared_ptr<RTPSession> >::iterator iter = internal->rtplist.find(mediainfo.get());
	if (iter == internal->rtplist.end()) return false;

	shared_ptr<RTPSession> rtpsession = iter->second;
	if (rtpsession)
	{
		rtpsession->sendContorlData(mediainfo, buffer, bufferlen);
	}

	return (bool)rtpsession;
}