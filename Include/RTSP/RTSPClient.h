
#pragma once
#include "Defs.h"
#include "Base/Base.h"
#include "RTSPStructs.h"
#include "Network/Network.h"
using namespace Public::Base;
using namespace Public::Network;

namespace Public {
namespace RTSP {

class RTSPClientManager;
class RTSPClientHandler;
class RTSP_API RTSPClient
{
	friend class RTSPClientManager;
	struct RTSPClientInternal;

	RTSPClient(const std::shared_ptr<IOWorker>& work, const shared_ptr<RTSPClientHandler>& handler, const std::string& rtspUrl,const std::string& useragent);
public:
	~RTSPClient();

	/*����RTP���ݽ��շ�ʽ 0:TCP��1:UDP  Ĭ��UDP*/
	bool initRTPOverTcpType();
	bool initRTPOverUdpType(uint32_t startport = 40000, uint32_t stopport = 4100);

	/*׼�����ݽ��գ������������ݽ����̣߳������߳�*/
	//timeout ���ӳ�ʱʱ�䣬
	//reconnect �Ƿ���������
	bool start(uint32_t timeout = 10000);

	bool stop();

	//�첽���ʹ��RTSPClientHandler->onPlayResponse���ս��
	shared_ptr<RTSPCommandInfo> sendPlayRequest(const RANGE_INFO& range);
	//ͬ�����ͬ������
	bool sendPlayRequest(const RANGE_INFO& range, uint32_t timeout);

	//�첽���ʹ��RTSPClientHandler->onPauseResponse���ս��
	shared_ptr<RTSPCommandInfo> sendPauseRequest();
	//ͬ������,ͬ������
	bool sendPauseRequest(uint32_t timeout);


	//�첽���ʹ��RTSPClientHandler->onGetparameterResponse���ս��
	shared_ptr<RTSPCommandInfo> sendGetparameterRequest(const std::string& body);
	//ͬ������,ͬ������
	bool sendGetparameterRequest(const std::string& body, uint32_t timeout);

	//�첽���ʹ��RTSPClientHandler->onTeradownResponse���ս��
	shared_ptr<RTSPCommandInfo> sendTeradownRequest();
	//ͬ������,ͬ������
	bool sendTeradownRequest(uint32_t timeout);
private:
	RTSPClientInternal *internal;
};

class RTSP_API RTSPClientHandler
{
public:
	RTSPClientHandler() {}
	virtual ~RTSPClientHandler() {}

	virtual void onConnectResponse(bool success, const std::string& errmsg) {}

	virtual void onDescribeResponse(const shared_ptr<RTSPCommandInfo>& cmdinfo, const MEDIA_INFO& info) {}
	virtual void onSetupResponse(const shared_ptr<RTSPCommandInfo>& cmdinfo, bool isVideo,const TRANSPORT_INFO& transport) {}
	virtual void onPlayResponse(const shared_ptr<RTSPCommandInfo>& cmdinfo) {}
	virtual void onPauseResponse(const shared_ptr<RTSPCommandInfo>& cmdinfo) {}
	virtual void onGetparameterResponse(const shared_ptr<RTSPCommandInfo>& cmdinfo, const std::string& content) {}
	virtual void onTeradownResponse(const shared_ptr<RTSPCommandInfo>& cmdinfo) {}

	virtual void onErrorResponse(const shared_ptr<RTSPCommandInfo>& cmdinfo,int statuscode,const std::string& errmsg) {}

	virtual void onClose() = 0;
	virtual void onMediaCallback(bool video, const FRAME_INFO& info, const char* buffer, uint32_t bufferlen) {}
};

class RTSP_API RTSPClientManager
{
	struct RTSPClientManagerInternal;
public:
	//userContent �û�������Ϣ,threadNum �߳���������RTSP���û�������
	RTSPClientManager(const shared_ptr<IOWorker>& iowrker,const std::string& useragent);
	~RTSPClientManager();


	//����һ������
	shared_ptr<RTSPClient> create(const shared_ptr<RTSPClientHandler>& handler, const std::string& pRtspUrl);


	/*���url�Ƿ�Ϸ�������Ϸ�*/
	static bool CheckUrl(const std::string& pRtspUrl);

	/*��ȡ�û������룬������url�а����ķ�������ַ���˿ڣ��û���������*/
	static bool GetUserInfo(const std::string& pRtspUrl, std::string& pUserName, std::string& pPassWord);
private:
	RTSPClientManagerInternal * internal;
};


}
}
