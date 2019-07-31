#include "RTSP/RTSP.h"
using namespace Public::RTSP;

#if 0

class RTSPServerSessiontmp;


class RTSPServerSessiontmp :public RTSPServerHandler,public Thread
{
public:
	RTSPServerSessiontmp():Thread("RTSPServerSessiontmp"){}
	~RTSPServerSessiontmp()
	{
		int a = 0;
	}

	shared_ptr<RTSPServerSession> session;

	virtual void onOptionRequest(const shared_ptr<RTSPServerSession>& session, const shared_ptr<RTSPCommandInfo>& cmdinfo) 
	{ 
		session->sendOptionResponse(cmdinfo); 
	}
	virtual void onDescribeRequest(const shared_ptr<RTSPServerSession>& session, const shared_ptr<RTSPCommandInfo>& cmdinfo)
	{
		MEDIA_INFO info;
		info.bHasVideo = true;
		info.bHasAudio = true;

		session->sendDescribeResponse(cmdinfo,info);
	}
	virtual void onSetupRequest(const shared_ptr<RTSPServerSession>& session, const shared_ptr<RTSPCommandInfo>& cmdinfo, const TRANSPORT_INFO& transport)
	{
		session->sendSetupResponse(cmdinfo,transport);
	}
	virtual void onPlayRequest(const shared_ptr<RTSPServerSession>& session, const shared_ptr<RTSPCommandInfo>& cmdinfo, const RANGE_INFO& range)
	{
		session->sendPlayResponse(cmdinfo);
		createThread();
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
		int a = 0;
	}
	virtual void onMediaCallback(bool isvideo, uint32_t timestmap, const RTSPBuffer& buffer, bool mark)
	{
		int a = 0;
	}

private:
	void threadProc()
	{
#define MAXBUFFERSIZE	26*1024

		RTSPBuffer buffer;
		buffer.alloc(MAXBUFFERSIZE);
		buffer.resize(MAXBUFFERSIZE);

		uint32_t timestmap = 0;
		while (looping())
		{
			session->sendMedia(true, timestmap += 25, buffer, true);

			Thread::sleep(25);
		}
	}
};

shared_ptr<RTSPServerHandler> rtspAceeptCallback1(const shared_ptr<RTSPServerSession>& server)
{
	//Guard locker(mutex);

	shared_ptr< RTSPServerSessiontmp> servertmp = make_shared<RTSPServerSessiontmp>();
	servertmp->session = server;

	//sessionlist[server.get()] = servertmp;


	return servertmp;
}


int main2()
{
	shared_ptr<IOWorker>	worker = make_shared<IOWorker>(16);
	shared_ptr<RTSPServer> manager = make_shared<RTSPServer>(worker,"test");

	manager->run(5554, rtspAceeptCallback1);


	while (1)
	{
		Thread::sleep(1000);

	/*	std::list<shared_ptr< RTSPServerSessiontmp> > freelisttmp;
		{
			Guard locker(mutex);
			freelisttmp = freelist;
			freelist.clear();
		}*/
	}

	getchar();

	return 0;
}

#endif