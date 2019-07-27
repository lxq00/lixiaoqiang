#pragma once

#include "RTSP/Defs.h"
#include "RTSP/RTSPStructs.h"
#include "Network/Network.h"
#include "Base/Base.h"
using namespace Public::Base;
using namespace Public::Network;

#define OPTIONCMDSTR "DESCRIBE,SETUP,TEARDOWN,PLAY,PAUSE,OPTIONS,ANNOUNCE,RECORD"

struct RTSP_API RTSPCommandInfo
{
	std::string		method;
	URL				url;
	uint32_t		code;
	std::string		session;
};
class RTSPCommandRequestHandler;
class RTSP_API RTSPSession
{
public:
	typedef Function1<shared_ptr<RTSPCommandRequestHandler>, Socket*> QueryHandlerCallback;
	typedef Function1<shared_ptr<RTSPSession>, Socket*> QuerySessionCallback;
public:
	RTSPSession(const shared_ptr<Socket>& sock,const QueryHandlerCallback& querycallback);
	virtual ~RTSPSession();

	void setPassword(const std::string& password);
	const URL& url() const;
	uint32_t sendListSize() const;
	uint64_t prevAlivetime() const;

	void sendOptionResponse(const shared_ptr<RTSPCommandInfo>& cmdinfo, const std::string& cmdstr = OPTIONCMDSTR);
	void sendDescribeResponse(const shared_ptr<RTSPCommandInfo>& cmdinfo, const MEDIA_INFO& mediainfo);
	void sendSetupResponse(const shared_ptr<RTSPCommandInfo>& cmdinfo, const TRANSPORT_INFO& transport);
	void sendPlayResponse(const shared_ptr<RTSPCommandInfo>& cmdinfo);
	void sendPauseResponse(const shared_ptr<RTSPCommandInfo>& cmdinfo);
	void sendTeradownResponse(const shared_ptr<RTSPCommandInfo>& cmdinfo);
	void sendGetparameterResponse(const shared_ptr<RTSPCommandInfo>& cmdinfo, const std::string& content);

	void sendErrorResponse(const shared_ptr<RTSPCommandInfo>& cmdinfo, int errcode, const std::string& errmsg);
	void sendMedia(bool video,const FRAME_INFO& info,const char* buffer,uint32_t bufferlen);
private:
	struct RTSPSessionInternal;
	RTSPSessionInternal* internal;
};

class RTSP_API RTSPCommandRequestHandler
{
public:
	RTSPCommandRequestHandler() {}
	virtual ~RTSPCommandRequestHandler() {}

	virtual void onOptionRequest(const shared_ptr<RTSPSession>& session, const shared_ptr<RTSPCommandInfo>& cmdinfo) { session->sendOptionResponse(cmdinfo); }
	virtual void onDescribeRequest(const shared_ptr<RTSPSession>& session, const shared_ptr<RTSPCommandInfo>& cmdinfo) = 0;
	virtual void onSetupRequest(const shared_ptr<RTSPSession>& session, const shared_ptr<RTSPCommandInfo>& cmdinfo, const TRANSPORT_INFO& transport) = 0;
	virtual void onPlayRequest(const shared_ptr<RTSPSession>& session, const shared_ptr<RTSPCommandInfo>& cmdinfo,const RANGE_INFO& range) = 0;
	virtual void onPauseRequest(const shared_ptr<RTSPSession>& session, const shared_ptr<RTSPCommandInfo>& cmdinfo) { session->sendErrorResponse(cmdinfo, 500, "NOT SUPPORT"); }
	virtual void onTeardownRequest(const shared_ptr<RTSPSession>& session, const shared_ptr<RTSPCommandInfo>& cmdinfo) = 0;
	virtual void onGetparameterRequest(const shared_ptr<RTSPSession>& session, const shared_ptr<RTSPCommandInfo>& cmdinfo,const std::string& content) { session->sendErrorResponse(cmdinfo, 500, "NOT SUPPORT"); }
	
	virtual void onClose(const shared_ptr<RTSPSession>& session) = 0;
	virtual void onMediaCallback(bool video, const FRAME_INFO& info, const char* buffer, uint32_t bufferlen) {}
};

class RTSP_API RTSPServer
{
public:
	typedef Function1<shared_ptr<RTSPCommandRequestHandler>, const shared_ptr<RTSPSession>&> ListenCallback;
public:
	RTSPServer(const shared_ptr<IOWorker>& worker, const std::string& useragent);
	virtual ~RTSPServer();

	bool run(uint32_t port, const ListenCallback& callback);
	bool stop();
private:
	struct RTSPServerInternal;
	RTSPServerInternal* internal;
};