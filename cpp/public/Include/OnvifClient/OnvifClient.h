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

//Onvif�ͻ���
class OnvifClientManager;
class ONVIFCLIENT_API OnvifClient
{
	friend class OnvifClientManager;
	OnvifClient(const URI& url,const shared_ptr<IOWorker>& worker,const std::string& useragent);
public:
	//@AlarmState 0:δ֪״̬��1:����������2:������ʧ
	//void,int alarmType, int alarmState, uint32_t& alarmCount, Time alarmTime
	typedef Function4<void, int, int, uint32_t, Time> AlarmCallback;
public:
	~OnvifClient();

	shared_ptr<OnvifClientDefs::Info> getInfo(int timeoutms = 10000);			//��ȡ�豸��Ϣ��������Ϣʹ��XM_GetLastError����
	shared_ptr<OnvifClientDefs::Capabilities> getCapabities(int timeoutms = 10000);	//��ȡ�豸�������ϣ�������Ϣʹ��XM_GetLastError����
	shared_ptr<OnvifClientDefs::Scopes> getScopes(int timeoutms = 10000); //��ȡ������Ϣ��������Ϣʹ��XM_GetLastError����
	shared_ptr<OnvifClientDefs::Profiles> getProfiles(int timeoutms = 10000); //��ȡ������Ϣ��������Ϣʹ��XM_GetLastError����
	std::string getStreamUrl(const std::string& streamtoken, int timeoutms = 10000); //��ȡ����Ϣ,������Ϣʹ��XM_GetLastError����
	std::string getSnapUrl(const std::string& snaptoken, int timeoutms = 10000);	//��ȡ��ͼ��Ϣ��������Ϣʹ��XM_GetLastError����
	shared_ptr<OnvifClientDefs::NetworkInterfaces> getNetworkInterfaces(int timeoutms = 10000);//������Ϣ��������Ϣʹ��XM_GetLastError����
	shared_ptr<OnvifClientDefs::VideoEncoderConfigurations> getVideoEncoderConfigurations(int timeoutms = 10000); //��ȡ��Ƶ������Ϣ��������Ϣʹ��XM_GetLastError����
	shared_ptr<OnvifClientDefs::ContinuousMove> getContinuousMove(int timeoutms = 10000); //������Ϣʹ��XM_GetLastError����
	shared_ptr<OnvifClientDefs::AbsoluteMove> getAbsoluteMove(int timeoutms = 10000); //������Ϣʹ��XM_GetLastError����
	shared_ptr<OnvifClientDefs::_PTZConfig> getConfigurations(int timeoutms = 10000); //������Ϣʹ��XM_GetLastError����
	shared_ptr<OnvifClientDefs::ConfigurationOptions> getConfigurationOptions(int timeoutms = 10000); //������Ϣʹ��XM_GetLastError����
	shared_ptr<Time> GetSystemDatetime(int timeoutms = 10000); //������Ϣʹ��XM_GetLastError����
	bool SetSystemDatetime(const Time& time, int timeoutms = 10000); //������Ϣʹ��XM_GetLastError����
	bool SystemReboot(int timeoutms = 10000);//������Ϣʹ��XM_GetLastError����
	bool startRecvAlarm(/*const AlarmCallback& callback*/);
	bool stopRecvAlarm();
private:
	struct OnvifClientInternal;
	OnvifClientInternal* internal;
};


//OnvifClient������
class ONVIFCLIENT_API OnvifClientManager
{
	friend OnvifClient;
public:
	//userContent �û�������Ϣ,threadNum �߳���������RTSP���û�������
	OnvifClientManager(const std::string& userContent, const IOWorker::ThreadNum& threadNum = IOWorker::ThreadNum(1, 4, 16));
	OnvifClientManager(const std::string& userContent, const shared_ptr<IOWorker>& worker);
	~OnvifClientManager();

	//����onvif�豸��ַ����һ���豸,url Ϊ�豸��ַ������IP���˿ڣ��û������������Ϣ,onvifĬ������·��Ϊ "/onvif/device_service"
	shared_ptr<OnvifClient> create(const URI& url);
private:
	struct OnvifClientManagerInternal;
	OnvifClientManagerInternal* internal;
};


}
}



#endif //__ONVIFCLIENT_H__
