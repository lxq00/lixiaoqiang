#ifndef __ONVIFCLIENT_H__
#define __ONVIFCLIENT_H__
#include "Base/Base.h"
#include "Network/Network.h"
#include "OnvifClientDefs.h"
using namespace Public::Base;
using namespace Public::Network;

#ifdef WIN32
#ifdef ONVIFCLIENT_EXPORTS
#define ONVIFCLIENT_API __declspec(dllexport)
#else
#define ONVIFCLIENT_API __declspec(dllimport)
#endif
#else
#define ONVIFCLIENT_API
#endif


namespace Public {
namespace Onvif {

//Onvif客户端
class OnvifClientManager;
class ONVIFCLIENT_API OnvifClient
{
	friend class OnvifClientManager;
	OnvifClient(const shared_ptr<IOWorker>& worker, const URL& url,const std::string& useragent);
public:
	~OnvifClient();

	shared_ptr<OnvifClientDefs::Info> getInfo(int timeoutms = 10000);			//获取设备信息，错误信息使用XM_GetLastError捕获
	shared_ptr<OnvifClientDefs::Capabilities> getCapabities(int timeoutms = 10000);	//获取设备能力集合，错误信息使用XM_GetLastError捕获
//	shared_ptr<OnvifClientDefs::Scopes> getScopes(int timeoutms = 10000); //获取描述信息，错误信息使用XM_GetLastError捕获
	shared_ptr<OnvifClientDefs::Profiles> getProfiles(int timeoutms = 10000); //获取配置信息，错误信息使用XM_GetLastError捕获
	shared_ptr<OnvifClientDefs::StreamUrl> getStreamUrl(const OnvifClientDefs::ProfileInfo& info, int timeoutms = 10000); //获取六信息,错误信息使用XM_GetLastError捕获
	shared_ptr<OnvifClientDefs::SnapUrl> getSnapUrl(const OnvifClientDefs::ProfileInfo& info, int timeoutms = 10000);	//获取截图信息，错误信息使用XM_GetLastError捕获
	shared_ptr<OnvifClientDefs::NetworkInterfaces> getNetworkInterfaces(int timeoutms = 10000);//网络信息，错误信息使用XM_GetLastError捕获
//	shared_ptr<OnvifClientDefs::VideoEncoderConfigurations> getVideoEncoderConfigurations(int timeoutms = 10000); //获取视频编码信息，错误信息使用XM_GetLastError捕获
	shared_ptr<OnvifClientDefs::ContinuousMove> getContinuousMove(const OnvifClientDefs::ProfileInfo& info, int timeoutms = 10000); //错误信息使用XM_GetLastError捕获
	shared_ptr<OnvifClientDefs::AbsoluteMove> getAbsoluteMove(const OnvifClientDefs::ProfileInfo& info, int timeoutms = 10000); //错误信息使用XM_GetLastError捕获
	shared_ptr<OnvifClientDefs::PTZConfig> getConfigurations(int timeoutms = 10000); //错误信息使用XM_GetLastError捕获
	shared_ptr<OnvifClientDefs::ConfigurationOptions> getConfigurationOptions(const shared_ptr<OnvifClientDefs::PTZConfig>& ptzcfg,int timeoutms = 10000); //错误信息使用XM_GetLastError捕获
	shared_ptr<Time> GetSystemDatetime(int timeoutms = 10000); //错误信息使用XM_GetLastError捕获
	bool SetSystemDatetime(const Time& time, int timeoutms = 10000); //错误信息使用XM_GetLastError捕获
	bool SystemReboot(int timeoutms = 10000);//错误信息使用XM_GetLastError捕获
	shared_ptr<OnvifClientDefs::StartRecvAlarm> startRecvAlarm(const shared_ptr<OnvifClientDefs::Capabilities>& capabilities,int timeoutms = 10000);
	bool recvAlarm(const shared_ptr<OnvifClientDefs::StartRecvAlarm>& alarminfo,int timeoutms = 2*60000);
	bool stopRecvAlarm();
private:
	struct OnvifClientInternal;
	OnvifClientInternal* internal;
};


//OnvifClient管理器
class ONVIFCLIENT_API OnvifClientManager
{
	friend OnvifClient;
public:
	//userContent 用户描述信息,threadNum 线程数，根据RTSP的用户量决定
	OnvifClientManager(const shared_ptr<IOWorker>& worker,const std::string& userContent);
	~OnvifClientManager();

	//根据onvif设备地址创建一个设备,url 为设备地址，包括IP，端口，用户名，密码等信息,onvif默认请求路径为 "/onvif/device_service"
	//如:admin:admin@192.168.13.33
	shared_ptr<OnvifClient> create(const URL& url);
private:
	struct OnvifClientManagerInternal;
	OnvifClientManagerInternal* internal;
};


}
}



#endif //__ONVIFCLIENT_H__
