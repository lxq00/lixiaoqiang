
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

	//��ʼ���ص���Ϣ
	bool initCallback(const RTSPClient_DataCallBack& dataCallback, const RTSPClient_StatusCallback& statusCallback,const RTSPClient_MediaCallback& mediainfo,const RTSPClient_ProtocolCallback& protocolCallback,void* userData);
	
	//��ʼ���ص���Ϣ
	bool initCallback(const RTSPClient_RTPDataCallBack& dataCallback, const RTSPClient_StatusCallback& statusCallback, const RTSPClient_MediaCallback& mediainfo, const RTSPClient_ProtocolCallback& protocolCallback, void* userData);

	/*����RTP���ݽ��շ�ʽ 0:TCP��1:UDP  Ĭ��UDP*/
	bool initRecvType(RTP_RECV_Type nRecvType);

	bool initUserInfo(const std::string& username, const std::string& passwd);

	bool initPlayInfo(const std::string& type, const std::string& starttime, const std::string& stoptime);
	
	/*׼�����ݽ��գ������������ݽ����̣߳������߳�*/
	//timeout ���ӳ�ʱʱ�䣬
	//reconnect �Ƿ���������
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
	//userContent �û�������Ϣ,threadNum �߳���������RTSP���û�������
	RTSPClientManager(const std::string& userContent, const IOWorker::ThreadNum& threadNum = IOWorker::ThreadNum(1, 4, 16));
	~RTSPClientManager();
	
	//UDP�ķ�ʽ �˿ڷ�Χ,�ڲ����ⷶΧ���Զ�����
	bool initUDPRecvTypeStartPort(uint32_t startport = 40000, uint32_t stopport = 41000);

	//����һ������
	shared_ptr<RTSPClient> create(const std::string& pRtspUrl);


	/*���url�Ƿ�Ϸ�������Ϸ���������url�а����ķ�������ַ���˿ڣ��û���������*/
	static bool CheckUrl(const std::string& pRtspUrl);

	/*��ȡ�û�������*/
	static bool GetUserInfo(const std::string& pRtspUrl, std::string& pUserName, std::string& pPassWord);
private:
	RTSPClientManagerInternal * internal;
};


}
}
