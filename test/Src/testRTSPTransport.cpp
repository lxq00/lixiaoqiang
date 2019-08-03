#include "RTSP/RTSP.h"
using namespace Public::RTSP;

#if 1
class RTSPServerSessiontmp;

Mutex			mutex;
std::map< RTSPServerSession*, shared_ptr< RTSPServerSessiontmp> > serverlist;

struct RTSPBufferInfo
{
	String		buffer;
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
		int a = 0;
	}

	shared_ptr<RTSPServerSession> session;
	bool						  isplaying;
	shared_ptr<MEDIA_INFO>		 mediainfo;

	virtual void onOptionRequest(const shared_ptr<RTSPServerSession>& session, const shared_ptr<RTSPCommandInfo>& cmdinfo)
	{
		session->sendOptionResponse(cmdinfo);
	}
	virtual void onDescribeRequest(const shared_ptr<RTSPServerSession>& session, const shared_ptr<RTSPCommandInfo>& cmdinfo)
	{
		MEDIA_INFO info = sourcemedia;
		//info.bHasVideo = true;
		//info.bHasAudio = true;

		session->sendDescribeResponse(cmdinfo, info);
	}
	virtual void onPlayRequest(const shared_ptr<RTSPServerSession>& session, const shared_ptr<RTSPCommandInfo>& cmdinfo, const shared_ptr<MEDIA_INFO>& _mediainfo, const RANGE_INFO& range)
	{
		session->sendPlayResponse(cmdinfo);
		isplaying = true;
		mediainfo = _mediainfo;

		Guard locker(mutex);
		for (std::list< RTSPBufferInfo>::iterator iter = cache.begin(); iter != cache.end(); iter++)
		{
			session->sendMediaPackage(mediainfo->videoStreamInfo(), iter->timestmap, iter->buffer.c_str(),iter->buffer.length(), iter->mark);
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

	virtual void onClose(const shared_ptr<RTSPServerSession>& session,const std::string& errmsg)
	{
		shared_ptr< RTSPServerSessiontmp> tmp;
		{
			Guard locker(mutex);
			std::map< RTSPServerSession*, shared_ptr< RTSPServerSessiontmp> >::iterator iter = serverlist.find(session.get());
			if (iter != serverlist.end())
			{
				tmp = iter->second;
				serverlist.erase(iter);
			}
		}		
	}
};
class RTSPSessiontmp :public RTSPClientHandler
{
	virtual void onConnectResponse(bool success, const std::string& errmsg) 
	{
		int a = 0;
	}

	virtual void onDescribeResponse(const shared_ptr<RTSPCommandInfo>& cmdinfo, const shared_ptr<MEDIA_INFO>& info)
	{
		sourcemedia = info->cloneStreamInfo();
		int a = 0;
	}
	virtual void onSetupResponse(const shared_ptr<RTSPCommandInfo>& cmdinfo, const shared_ptr<STREAM_TRANS_INFO>& transport)
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

	virtual void onClose(const std::string& errmsg)
	{
		int a = 0;
	}
	virtual void onMediaPackageCallback(const shared_ptr<STREAM_TRANS_INFO> mediainfo, const RTPHEADER& rtpheader, const char*  buffer, uint32_t bufferlen)
	{
		std::map< RTSPServerSession*, shared_ptr< RTSPServerSessiontmp> > sendlist;
		
		if (strcasecmp(mediainfo->streaminfo.szMediaName.c_str(), "video") != 0) return;

		RTSPBufferInfo info;
		info.buffer = String(buffer, bufferlen);
		info.timestmap = ntohl(rtpheader.ts);
		info.mark = rtpheader.m;

		if(1)
		{
			Guard locker(mutex);
			sendlist = serverlist;

			if (rtpheader.m) cache.clear();			

			cache.push_back(info);
		}

		for (std::map< RTSPServerSession*, shared_ptr< RTSPServerSessiontmp> >::iterator iter = sendlist.begin(); iter != sendlist.end(); iter++)
		{
			shared_ptr< RTSPServerSessiontmp> tmp = iter->second;
			if(tmp == NULL || !tmp->isplaying) continue;

			shared_ptr<RTSPServerSession> session = tmp->session;
			if(session == NULL) continue;

			session->sendMediaPackage(tmp->mediainfo->videoStreamInfo(), info.timestmap, buffer, bufferlen, info.mark);
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