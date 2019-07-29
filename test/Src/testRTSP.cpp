#include "RTSP/RTSP.h"
using namespace Public::RTSP;

class RTSPSessiontmp :public RTSPClientHandler
{
	virtual void onConnectResponse(bool success, const std::string& errmsg) 
	{
		int a = 0;
	}

	virtual void onDescribeResponse(const shared_ptr<RTSPCommandInfo>& cmdinfo, const MEDIA_INFO& info) 
	{
		int a = 0;
	}
	virtual void onSetupResponse(const shared_ptr<RTSPCommandInfo>& cmdinfo, bool isVideo, const TRANSPORT_INFO& transport)
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
	virtual void onMediaCallback(bool isvideo, uint32_t timestmap, const String& data)
	{
		int a = 0;
	}
};

int main()
{
	shared_ptr<IOWorker>	worker = make_shared<IOWorker>(4);
	shared_ptr<RTSPClientManager> manager = make_shared<RTSPClientManager>(worker,"test");
	shared_ptr<RTSPClientHandler> handler = make_shared<RTSPSessiontmp>();

	shared_ptr<RTSPClient> client = manager->create(handler, RTSPUrl("rtsp://admin:ms123456@192.168.7.104:554/main"));
	client->start(10000);


	getchar();

	return 0;
}