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
	OnvifClient(const URI& url,const shared_ptr<IOWorker>& worker,const std::string& useragent);
public:
	//@AlarmState 0:未知状态、1:报警发生、2:报警消失
	//void,int alarmType, int alarmState, uint32_t& alarmCount, Time alarmTime
	typedef Function4<void, int, int, uint32_t, Time> AlarmCallback;
public:
	~OnvifClient();

	shared_ptr<OnvifClientDefs::Info> getInfo(int timeoutms = 10000);			//获取设备信息，错误信息使用XM_GetLastError捕获
	shared_ptr<OnvifClientDefs::Capabilities> getCapabities(int timeoutms = 10000);	//获取设备能力集合，错误信息使用XM_GetLastError捕获
	shared_ptr<OnvifClientDefs::Scopes> getScopes(int timeoutms = 10000); //获取描述信息，错误信息使用XM_GetLastError捕获
	shared_ptr<OnvifClientDefs::Profiles> getProfiles(int timeoutms = 10000); //获取配置信息，错误信息使用XM_GetLastError捕获
	std::string getStreamUrl(const std::string& streamtoken, int timeoutms = 10000); //获取六信息,错误信息使用XM_GetLastError捕获
	std::string getSnapUrl(const std::string& snaptoken, int timeoutms = 10000);	//获取截图信息，错误信息使用XM_GetLastError捕获
	shared_ptr<OnvifClientDefs::NetworkInterfaces> getNetworkInterfaces(int timeoutms = 10000);//网络信息，错误信息使用XM_GetLastError捕获
	shared_ptr<OnvifClientDefs::VideoEncoderConfigurations> getVideoEncoderConfigurations(int timeoutms = 10000); //获取视频编码信息，错误信息使用XM_GetLastError捕获
	shared_ptr<OnvifClientDefs::ContinuousMove> getContinuousMove(int timeoutms = 10000); //错误信息使用XM_GetLastError捕获
	shared_ptr<OnvifClientDefs::AbsoluteMove> getAbsoluteMove(int timeoutms = 10000); //错误信息使用XM_GetLastError捕获
	shared_ptr<OnvifClientDefs::_PTZConfig> getConfigurations(int timeoutms = 10000); //错误信息使用XM_GetLastError捕获
	shared_ptr<OnvifClientDefs::ConfigurationOptions> getConfigurationOptions(int timeoutms = 10000); //错误信息使用XM_GetLastError捕获
	shared_ptr<Time> GetSystemDatetime(int timeoutms = 10000); //错误信息使用XM_GetLastError捕获
	bool SetSystemDatetime(const Time& time, int timeoutms = 10000); //错误信息使用XM_GetLastError捕获
	bool SystemReboot(int timeoutms = 10000);//错误信息使用XM_GetLastError捕获
	bool startRecvAlarm(/*const AlarmCallback& callback*/);
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
	OnvifClientManager(const std::string& userContent, const IOWorker::ThreadNum& threadNum = IOWorker::ThreadNum(1, 4, 16));
	OnvifClientManager(const std::string& userContent, const shared_ptr<IOWorker>& worker);
	~OnvifClientManager();

	//根据onvif设备地址创建一个设备,url 为设备地址，包括IP，端口，用户名，密码等信息,onvif默认请求路径为 "/onvif/device_service"
	shared_ptr<OnvifClient> create(const URI& url);
private:
	struct OnvifClientManagerInternal;
	OnvifClientManagerInternal* internal;
};


}
}



#endif //__ONVIFCLIENT_H__
