#include "RTSP/RTSP.h"
using namespace Public::RTSP;

#if 1

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

	virtual void onClose(const std::string& errmsg)
	{
		int a = 0;
	}
	virtual void onMediaCallback(bool isvideo, uint32_t timestmap, const RTSPBuffer& buffer, bool mark)
	{
		int a = 0;
	}
};

#define MAXTESTRTSPCLIENT		1

struct RTSPClientInfo
{
	shared_ptr<RTSPClientHandler> handler;
	shared_ptr<RTSPClient> client;
};

string rtspaddr[] = {
	"rtsp://admin:ms123456@192.168.7.104:554/main",
	"rtsp://admin:ms123456@192.168.3.135:554/main",
	"rtsp://192.168.9.230:554/cam/realmonitor?channel=1&subtype=0&unicast=true&proto=Onvif",
	"rtsp://admin:support2019@192.168.9.205:554/Streaming/Channels/102",
	"rtsp://admin:ms123456@192.168.10.230:554/main",
//	"rtsp://admin:ms123456@192.168.9.230:554/main",
	"rtsp://admin:ms123456@192.168.4.150:554/main",
	//"rtsp://admin:ms123456@192.168.4.111:554/main",
	//"rtsp://admin:ms123456@192.168.2.172:554/main",
	//"rtsp://admin:ms123456@192.168.4.105:554/main",
	//"rtsp://admin:ms123456@192.168.10.236:554/main",
//	"rtsp://192.168.2.46:5554/111",
};

int main()
{

	//uint16_t startcode = 0;
	//uint16_t prevcode = 0;

	//uint16_t sn = 0;
	//uint16_t sn1 = 65533;

	//uint16_t sn2 = sn - sn1;

	//while (1)
	//{
	//	startcode++;

	//	if (prevcode == 0) prevcode = startcode;
	//	else if ((uint16_t)(prevcode + 1) != startcode)
	//	{
	//		int a = 0;
	//	}
	//	if (startcode == 0)
	//	{
	//		int b = 0;
	//	}

	//	prevcode = startcode;
	//}








	shared_ptr<IOWorker>	worker = make_shared<IOWorker>(4);
	shared_ptr<RTSPClientManager> manager = make_shared<RTSPClientManager>(worker,"test");

	std::list< RTSPClientInfo> clientlist;

	uint32_t rtspaddrsize = sizeof(rtspaddr) / sizeof(string);

	uint32_t openrtspaddrindex = 0;

	while (1)
	{
		if (clientlist.size() >= rtspaddrsize* MAXTESTRTSPCLIENT)
		{
			Thread::sleep(10000);
			clientlist.pop_front();
		}		

		Thread::sleep(1000);

		RTSPClientInfo info;
		info.handler = make_shared<RTSPSessiontmp>();
		info.client = manager->create(info.handler, RTSPUrl(rtspaddr[openrtspaddrindex % rtspaddrsize]));
	//	info.client->initRTPOverUdpType();
		info.client->start(10000);

		clientlist.push_back(info);

		openrtspaddrindex++;
	}

	getchar();

	return 0;
}

#endif