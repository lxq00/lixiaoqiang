#include "RTSP/RTSP.h"
using namespace Public::RTSP;

#if 0
class RTSPServerSessiontmp;

Mutex			mutex;
std::map< RTSPServerSession*, shared_ptr< RTSPServerSessiontmp> > serverlist;

struct RTSPBufferInfo
{
	bool		isvideo;
	RTSPBuffer	buffer;
	uint32_t	timestmap;
	bool		mark;
};

std::list< RTSPBufferInfo>		cache;
MEDIA_INFO						sourcemedia;

class RTSPServerSessiontmp :public RTSPServerHandler
{
	
public:
	RTSPServerSessiontmp():isplaying(false)
	{
	}
	~RTSPServerSessiontmp()
	{
		if (fd != NULL) fclose(fd);
		int a = 0;
	}

	shared_ptr<RTSPServerSession> session;
	bool						  isplaying;
	FILE* fd;

	virtual void onOptionRequest(const shared_ptr<RTSPServerSession>& session, const shared_ptr<RTSPCommandInfo>& cmdinfo)
	{
		session->sendOptionResponse(cmdinfo);
	}
	virtual void onDescribeRequest(const shared_ptr<RTSPServerSession>& session, const shared_ptr<RTSPCommandInfo>& cmdinfo)
	{
	//	MEDIA_INFO info;
	//	info.bHasVideo = true;
	//	info.bHasAudio = true;

		session->sendDescribeResponse(cmdinfo, sourcemedia);
	}
	virtual void onSetupRequest(const shared_ptr<RTSPServerSession>& session, const shared_ptr<RTSPCommandInfo>& cmdinfo, const TRANSPORT_INFO& transport)
	{
		session->sendSetupResponse(cmdinfo, transport);
	}
	virtual void onPlayRequest(const shared_ptr<RTSPServerSession>& session, const shared_ptr<RTSPCommandInfo>& cmdinfo, const RANGE_INFO& range)
	{
		session->sendPlayResponse(cmdinfo);
		isplaying = true;

		Guard locker(mutex);
		for (std::list< RTSPBufferInfo>::iterator iter = cache.begin(); iter != cache.end(); iter++)
		{
			session->sendMedia(iter->isvideo, iter->timestmap, iter->buffer, iter->mark);
		}
	}
	virtual void onPauseRequest(const shared_ptr<RTSPServerSession>& session, const shared_ptr<RTSPCommandInfo>& cmdinfo)
	{
		session->sendErrorResponse(cmdinfo, 500, "NOT SUPPORT");
	}
	virtual void onTeardownRequest(const shared_ptr<RTSPServerSession>& session, const shared_ptr<RTSPCommandInfo>& cmdinfo)
	{
		session->sendTeardownResponse(cmdinfo);
	}
	virtual void onGetparameterRequest(const shared_ptr<RTSPServerSession>& session, const shared_ptr<RTSPCommandInfo>& cmdinfo, const std::string& content) { session->sendErrorResponse(cmdinfo, 500, "NOT SUPPORT"); }

	virtual void onClose(const shared_ptr<RTSPServerSession>& session)
	{
		Guard locker(mutex);
		serverlist.erase(session.get());
	}
	virtual void onMediaCallback(bool isvideo, uint32_t timestmap, const RTSPBuffer& buffer, bool mark)
	{
		int a = 0;
	}
};
class RTSPSessiontmp :public RTSPClientHandler
{
	virtual void onConnectResponse(bool success, const std::string& errmsg) 
	{
		int a = 0;
	}

	virtual void onDescribeResponse(const shared_ptr<RTSPCommandInfo>& cmdinfo, const MEDIA_INFO& info) 
	{
		sourcemedia = info;
		int a = 0;
	}
	virtual void onSetupResponse(const shared_ptr<RTSPCommandInfo>& cmdinfo, const MEDIA_INFO& info, const TRANSPORT_INFO& transport)
	{
		int a = 0;
	}
	virtual void onPlayResponse(const shared_ptr<RTSPCommandInfo>& cmdinfo)
	{
		int a = 0;
	}
	virtual void onPauseResponse(const shared_ptr<RTSPCommandInfo>& cmdinfo)
	{
		int a = 0;
	}
	virtual void onGetparameterResponse(const shared_ptr<RTSPCommandInfo>& cmdinfo, const std::string& content)
	{
		int a = 0;
	}
	virtual void onTeradownResponse(const shared_ptr<RTSPCommandInfo>& cmdinfo)
	{
		int a = 0;
	}

	virtual void onErrorResponse(const shared_ptr<RTSPCommandInfo>& cmdinfo, int statuscode, const std::string& errmsg)
	{
		int a = 0;
	}

	virtual void onClose()
	{
		int a = 0;
	}
	virtual void onMediaCallback(bool isvideo, uint32_t timestmap, const RTSPBuffer& buffer, bool mark)
	{
		std::map< RTSPServerSession*, shared_ptr< RTSPServerSessiontmp> > sendlist;
		{
			Guard locker(mutex);
			sendlist = serverlist;

			if (mark) cache.clear();

			RTSPBufferInfo info;
			info.isvideo = isvideo;
			info.buffer = buffer;
			info.timestmap = timestmap;
			info.mark = mark;

			cache.push_back(info);
		}

		for (std::map< RTSPServerSession*, shared_ptr< RTSPServerSessiontmp> >::iterator iter = sendlist.begin(); iter != sendlist.end(); iter++)
		{
			shared_ptr< RTSPServerSessiontmp> tmp = iter->second;
			if(tmp == NULL || !tmp->isplaying) continue;

			shared_ptr<RTSPServerSession> session = tmp->session;
			if(session == NULL) continue;

			session->sendMedia(isvideo, timestmap, buffer, mark);
		}

		int a = 0;
	}
};


struct RTSPClientInfo
{
	shared_ptr<RTSPClientHandler> handler;
	shared_ptr<RTSPClient> client;
};

static shared_ptr<RTSPServerHandler> rtspAceeptCallback(const shared_ptr<RTSPServerSession>& server)
{
	Guard locker(mutex);

	shared_ptr< RTSPServerSessiontmp> servertmp = make_shared<RTSPServerSessiontmp>();
	servertmp->session = server;

	serverlist[server.get()] = servertmp;


	return servertmp;
}


int main()
{
	shared_ptr<IOWorker>	worker = make_shared<IOWorker>(32);
	shared_ptr<RTSPClientManager> manager = make_shared<RTSPClientManager>(worker,"test");

	std::list< RTSPClientInfo> clientlist;

	{
		RTSPClientInfo info;
		info.handler = make_shared<RTSPSessiontmp>();
		info.client = manager->create(info.handler, RTSPUrl("rtsp://192.168.9.230:554/cam/realmonitor?channel=1&subtype=0&unicast=true&proto=Onvif"));
	//	info.client->initRTPOverUdpType();
		info.client->start(10000);

		clientlist.push_back(info);
	}

	shared_ptr<RTSPServer> servermanager = make_shared<RTSPServer>(worker, "test");
	servermanager->run(5554, rtspAceeptCallback);

	getchar();

	return 0;
}

#endif