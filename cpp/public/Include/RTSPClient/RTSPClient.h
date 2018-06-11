
#pragma once
#include "RTSPClientSdk.h"
#include "Base/Base.h"
#include "Network/Network.h"
#include "RTSPClientErrorCode.h"
using namespace Public::Base;
using namespace Public::Network;

namespace Public {
namespace RTSPClient {

typedef Function4<void, const char*, long, LPFRAME_INFO, void*> RTSPClient_DataCallBack;
typedef Function4<void,bool, const char*, long, void*> RTSPClient_RTPDataCallBack;
typedef Function2<void, RTSPStatus_t, void*> RTSPClient_StatusCallback;
typedef Function2<void, const MEDIA_PARAMETER&, void*> RTSPClient_MediaCallback;
typedef Function4<void, bool /*send*/, const char*, uint32_t, void*> RTSPClient_ProtocolCallback;
class RTSPClientManager;

class RTSPCLIENT_API RTSPClient
{
	friend class RTSPClientManager;
	struct RTSPClientInternal;

	RTSPClient(RTSPClientManager* manager, const std::string& rtspUrl,const std::string& usercontext);
public:
	~RTSPClient();

	//初始化回调信息
	bool initCallback(const RTSPClient_DataCallBack& dataCallback, const RTSPClient_StatusCallback& statusCallback,const RTSPClient_MediaCallback& mediainfo,const RTSPClient_ProtocolCallback& protocolCallback,void* userData);
	
	//初始化回调信息
	bool initCallback(const RTSPClient_RTPDataCallBack& dataCallback, const RTSPClient_StatusCallback& statusCallback, const RTSPClient_MediaCallback& mediainfo, const RTSPClient_ProtocolCallback& protocolCallback, void* userData);

	/*设置RTP数据接收方式 0:TCP，1:UDP  默认UDP*/
	bool initRecvType(RTP_RECV_Type nRecvType);

	bool initUserInfo(const std::string& username, const std::string& passwd);

	bool initPlayInfo(const std::string& type, const std::string& starttime, const std::string& stoptime);
	
	/*准备数据接收，包括启动数据接收线程，心跳线程*/
	//timeout 连接超时时间，
	//reconnect 是否启用重连
	bool start(uint32_t timeout = 10000);

	bool stop();

	bool getMediaInfo(LPMEDIA_PARAMETER param);
private:
	RTSPClientInternal *internal;
};


class RTSPCLIENT_API RTSPClientManager
{
	friend class RTSPClient;
	struct RTSPClientManagerInternal;
public:
	//userContent 用户描述信息,threadNum 线程数，根据RTSP的用户量决定
	RTSPClientManager(const std::string& userContent, const IOWorker::ThreadNum& threadNum = IOWorker::ThreadNum(1, 4, 16));
	~RTSPClientManager();
	
	//UDP的方式 端口范围,内部在这范围内自动分配
	bool initUDPRecvTypeStartPort(uint32_t startport = 40000, uint32_t stopport = 41000);

	//创建一个对象
	shared_ptr<RTSPClient> create(const std::string& pRtspUrl);


	/*检查url是否合法，如果合法，解析出url中包含的服务器地址，端口，用户名，密码*/
	static bool CheckUrl(const std::string& pRtspUrl);

	/*获取用户名密码*/
	static bool GetUserInfo(const std::string& pRtspUrl, std::string& pUserName, std::string& pPassWord);
private:
	RTSPClientManagerInternal * internal;
};


}
}
