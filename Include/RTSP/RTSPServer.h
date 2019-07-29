#pragma once

#include "RTSP/Defs.h"
#include "RTSP/RTSPStructs.h"
#include "Network/Network.h"
#include "Base/Base.h"
#include "HTTP/HTTPParse.h"
using namespace Public::Base;
using namespace Public::Network;
using namespace Public::HTTP;

namespace Public {
namespace RTSP {

class RTSPServerSession;
class RTSPServerHandler;

class RTSP_API RTSPServer
{
public:
	struct RTSPServerInternal;
	typedef Function1<shared_ptr<RTSPServerHandler>, const shared_ptr<RTSPServerSession>&> ListenCallback;
public:
	RTSPServer(const shared_ptr<IOWorker>& worker, const std::string& useragent);
	virtual ~RTSPServer();

	bool run(uint32_t port, const ListenCallback& callback);
	bool stop();
	uint32_t listenPort() const;
private:
	RTSPServerInternal* internal;
};

class RTSP_API RTSPServerSession
{
	friend struct RTSPServer::RTSPServerInternal;
private:
	RTSPServerSession(const shared_ptr<IOWorker>& worker, const shared_ptr<Socket>& sock, const RTSPServer::ListenCallback& querycallback, const std::string& useragent);
	void initRTSPServerSessionPtr(const weak_ptr<RTSPServerSession>& session);
public:
	virtual ~RTSPServerSession();

	void setAuthenInfo(const std::string& username, const std::string& password);
	const URL& url() const;
	uint32_t sendListSize() const;
	uint64_t prevAlivetime() const;
	shared_ptr<RTSPServerHandler> handler();

	void sendOptionResponse(const shared_ptr<RTSPCommandInfo>& cmdinfo, const std::string& cmdstr = "");
	void sendDescribeResponse(const shared_ptr<RTSPCommandInfo>& cmdinfo, const MEDIA_INFO& mediainfo);
	void sendSetupResponse(const shared_ptr<RTSPCommandInfo>& cmdinfo, const TRANSPORT_INFO& transport);
	void sendPlayResponse(const shared_ptr<RTSPCommandInfo>& cmdinfo);
	void sendPauseResponse(const shared_ptr<RTSPCommandInfo>& cmdinfo);
	void sendTeradownResponse(const shared_ptr<RTSPCommandInfo>& cmdinfo);
	void sendGetparameterResponse(const shared_ptr<RTSPCommandInfo>& cmdinfo, const std::string& content);

	void sendErrorResponse(const shared_ptr<RTSPCommandInfo>& cmdinfo, int errcode, const std::string& errmsg);
	void sendMedia(bool isvideo, uint32_t timestmap, const String& data);
private:
	struct RTSPServerSessionInternal;
	RTSPServerSessionInternal* internal;
};

class RTSP_API RTSPServerHandler
{
public:
	RTSPServerHandler() {}
	virtual ~RTSPServerHandler() {}

	virtual void onOptionRequest(const shared_ptr<RTSPServerSession>& session, const shared_ptr<RTSPCommandInfo>& cmdinfo) { session->sendOptionResponse(cmdinfo); }
	virtual void onDescribeRequest(const shared_ptr<RTSPServerSession>& session, const shared_ptr<RTSPCommandInfo>& cmdinfo) = 0;
	virtual void onSetupRequest(const shared_ptr<RTSPServerSession>& session, const shared_ptr<RTSPCommandInfo>& cmdinfo, const TRANSPORT_INFO& transport) = 0;
	virtual void onPlayRequest(const shared_ptr<RTSPServerSession>& session, const shared_ptr<RTSPCommandInfo>& cmdinfo, const RANGE_INFO& range) = 0;
	virtual void onPauseRequest(const shared_ptr<RTSPServerSession>& session, const shared_ptr<RTSPCommandInfo>& cmdinfo) { session->sendErrorResponse(cmdinfo, 500, "NOT SUPPORT"); }
	virtual void onTeardownRequest(const shared_ptr<RTSPServerSession>& session, const shared_ptr<RTSPCommandInfo>& cmdinfo) = 0;
	virtual void onGetparameterRequest(const shared_ptr<RTSPServerSession>& session, const shared_ptr<RTSPCommandInfo>& cmdinfo, const std::string& content) { session->sendErrorResponse(cmdinfo, 500, "NOT SUPPORT"); }

	virtual void onClose(const shared_ptr<RTSPServerSession>& session) = 0;
	virtual void onMediaCallback(bool isvideo, uint32_t timestmap, const String& data) {}
};

}
}

