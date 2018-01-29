#include "RTSPClient/RTSPClientSdk.h"
#include "RTSPClient/RTSPClient.h"
#include "RTSPClient/RTSPClientErrorCode.h"

using namespace Public::RTSPClient;

//........................global............................
static Mutex	clientmutex;
static long		clientindex = 0;

struct WAITMEDIAINFO
{
	MEDIA_PARAMETER		mediainfo;
	Semaphore			mediasem;
};

#define STARTRTSPTIMEOUT		60000*3

struct RTSPUserInfo
{
	shared_ptr<RTSPClient> client;

	RtspClientDataCallBackFun datacallback;
	long dataUserParam1;
	void* dataUserParam2;

	RTSP_CLIENT_CONNECT_CALLBACK_FUNCTION statusFunc;
	long statusUserParam1;
	void* statuspUserParam2;

	shared_ptr<WAITMEDIAINFO>	waitmedia;
	

	RTSPUserInfo()
	{
		datacallback = NULL;
		dataUserParam1 = 0;
		dataUserParam2 = NULL;

		statusFunc = NULL;
		statusUserParam1 = 0;
		statuspUserParam2 = NULL;
	}
};

static std::map<long, shared_ptr<RTSPUserInfo> > clientMap;
static shared_ptr<RTSPClientManager> clientManager;

void _RTSPClientDataCallaback(const char* buf, long len, LPFRAME_INFO frame, void*userdata)
{
	shared_ptr<RTSPUserInfo>  info;
	{
		std::map<long, shared_ptr<RTSPUserInfo> >::iterator iter = clientMap.find((long)userdata);
		if (iter == clientMap.end())
		{
			return;
		}

		info = iter->second;
	}

	if (info->datacallback != NULL)
	{
		info->datacallback((const unsigned char*)buf, len, frame, info->dataUserParam1, info->dataUserParam2);
	}
}

void _RTSPClientStatusCallback(RTSPStatus_t status, void* userdata)
{
	shared_ptr<RTSPUserInfo>  info;
	{
		std::map<long, shared_ptr<RTSPUserInfo> >::iterator iter = clientMap.find((long)userdata);
		if (iter == clientMap.end())
		{
			return;
		}

		info = iter->second;
	}

	if (info != NULL && info->statusFunc != NULL)
	{
		info->statusFunc(status, info->dataUserParam1, info->dataUserParam2);
	}
}

void _RTSPClientMediaCallback(const MEDIA_PARAMETER& mediainfo, void* userdata)
{
	shared_ptr<RTSPUserInfo>  info;
	{
		std::map<long, shared_ptr<RTSPUserInfo> >::iterator iter = clientMap.find((long)userdata);
		if (iter == clientMap.end())
		{
			return;
		}

		info = iter->second;
	}

	shared_ptr<WAITMEDIAINFO> waitinfo = info->waitmedia;
	if (waitinfo != NULL)
	{
		waitinfo->mediainfo = mediainfo;
		waitinfo->mediasem.post();
	}
}

int CALLBACK XM_RTSP_Init()
{
	Guard locker(clientmutex);
	if (clientManager == NULL)
	{
		BaseSystem::init();
		NetworkSystem::init();
		clientManager = new RTSPClientManager("Public RTSP Client");
	}
	return RTSP_ERR_NO_ERROR;
}

int CALLBACK XM_RTSP_UnInit()
{
	Guard locker(clientmutex);
	clientindex = 0;
	clientMap.clear();
	clientManager = NULL;

	NetworkSystem::uninit();
	BaseSystem::uninit();

	return RTSP_ERR_NO_ERROR;
}

int CALLBACK XM_RTSP_AddTask(char* pUrl, long* nTaskHandle )
{
	Guard locker(clientmutex);
	if (clientManager == NULL)
	{
		return RTSP_ERR_UNINIT;
	}
	if (pUrl == NULL || nTaskHandle == NULL)
	{
		return RTSP_ERR_INVLID_PARAM;
	}
	if (!clientManager->CheckUrl(pUrl))
	{
		return RTSP_ERR_URL_CHECK_FAILED;
	}

	shared_ptr<RTSPClient> client = clientManager->create(pUrl);
	if (client == NULL)
	{
		return RTSP_ERR_CREATECLIENT_FAILED;
	}

	*nTaskHandle = ++clientindex;

	client->initCallback(_RTSPClientDataCallaback, _RTSPClientStatusCallback, _RTSPClientMediaCallback,NULL,(void*)*nTaskHandle);

	shared_ptr<RTSPUserInfo>  info(new RTSPUserInfo());
	info->client = client;
	
	clientMap[*nTaskHandle] = info;
	
	return RTSP_ERR_NO_ERROR;
}

int CALLBACK XM_RTSP_DelTask( long nTaskHandle )
{
	shared_ptr<RTSPClient> client;
	{
		Guard locker(clientmutex);
		if (clientManager == NULL)
		{
			return RTSP_ERR_UNINIT;
		}

		std::map<long, shared_ptr<RTSPUserInfo> >::iterator iter = clientMap.find(nTaskHandle);
		if (iter != clientMap.end())
		{
			client = iter->second->client;
			clientMap.erase(iter);
		}
	}

	return RTSP_ERR_NO_ERROR;
}
int CALLBACK XM_RTSP_SetStreamCallbackFunction(long nTaskHandle, RtspClientDataCallBackFun pFunc, long nUserParam1, void* pUserParam2)
{
	shared_ptr<RTSPUserInfo>  info;
	{
		Guard locker(clientmutex);
		if (clientManager == NULL)
		{
			return RTSP_ERR_UNINIT;
		}

		std::map<long, shared_ptr<RTSPUserInfo> >::iterator iter = clientMap.find(nTaskHandle);
		if (iter == clientMap.end())
		{
			return RTSP_ERR_INVLID_HANDLE;
		}
		info = iter->second;
	}
	info->datacallback = pFunc;
	info->dataUserParam1 = nUserParam1;
	info->dataUserParam2 = pUserParam2;

	return RTSP_ERR_NO_ERROR;
}

int CALLBACK XM_RTSP_SetConnectionCallbackFunction(long nTaskHandle, RTSP_CLIENT_CONNECT_CALLBACK_FUNCTION pFunc,long nUserParam1, void* pUserParam2)
{
	shared_ptr<RTSPUserInfo>  info;
	{
		Guard locker(clientmutex);
		if (clientManager == NULL)
		{
			return RTSP_ERR_UNINIT;
		}

		std::map<long, shared_ptr<RTSPUserInfo> >::iterator iter = clientMap.find(nTaskHandle);
		if (iter == clientMap.end())
		{
			return RTSP_ERR_INVLID_HANDLE;
		}
		info = iter->second;
	}
	info->statusFunc = pFunc;
	info->statusUserParam1 = nUserParam1;
	info->statuspUserParam2 = pUserParam2;

	return RTSP_ERR_NO_ERROR;
}

int CALLBACK XM_RTSP_StartRecv( long nTaskHandle, int nRecvType, LPMEDIA_PARAMETER lpMdeiaParam)
{
	if (lpMdeiaParam == NULL)
	{
		return RTSP_ERR_INVLID_PARAM;
	}
	shared_ptr<RTSPUserInfo>  info;
	{
		Guard locker(clientmutex);
		if (clientManager == NULL)
		{
			return RTSP_ERR_UNINIT;
		}

		std::map<long, shared_ptr<RTSPUserInfo> >::iterator iter = clientMap.find(nTaskHandle);
		if (iter == clientMap.end())
		{
			return RTSP_ERR_INVLID_HANDLE;
		}
		info = iter->second;
	}
	info->waitmedia = new WAITMEDIAINFO();

	info->client->initRecvType((RTP_RECV_Type)nRecvType);
	info->client->start();

	if (lpMdeiaParam != NULL)
	{
		info->waitmedia->mediasem.pend(STARTRTSPTIMEOUT);
		memcpy(lpMdeiaParam, &info->waitmedia->mediainfo, sizeof(MEDIA_PARAMETER));
		info->waitmedia = NULL;
	}
	

	return RTSP_ERR_NO_ERROR;
}

int CALLBACK XM_RTSP_StartRecvWithUserInfo(int nTaskHandle, char *pUserName, char* pPassword, int nRecvType, LPMEDIA_PARAMETER lpMdeiaParam)
{
	return XM_RTSP_StartRecvWithUserInfoEX(nTaskHandle, pUserName, pPassword, nRecvType, lpMdeiaParam, 3, (char*)"", (char*)"");
}

int CALLBACK XM_RTSP_StartRecvWithUserInfoEX(int nTaskHandle, char *pUserName, char* pPassword, int nRecvType, LPMEDIA_PARAMETER lpMdeiaParam, int nTimeType, char* pTartTime, char *pStopTime)
{
	if (lpMdeiaParam == NULL || pUserName == NULL || pPassword == NULL || pStopTime == NULL || pTartTime == NULL)
	{
		return RTSP_ERR_INVLID_PARAM;
	}
	shared_ptr<RTSPUserInfo>  info;
	{
		Guard locker(clientmutex);
		if (clientManager == NULL)
		{
			return RTSP_ERR_UNINIT;
		}

		std::map<long, shared_ptr<RTSPUserInfo> >::iterator iter = clientMap.find(nTaskHandle);
		if (iter == clientMap.end())
		{
			return RTSP_ERR_INVLID_HANDLE;
		}
		info = iter->second;
	}
	info->waitmedia = new WAITMEDIAINFO();

	std::string playtype;
	if (nTimeType == 0)
	{
		playtype = "ntp";
	}
	else if (nTimeType == 1)
	{
		playtype = "clock";
	}
	else if (nTimeType == 1)
	{
		playtype = "smpte";
	}
	info->client->initPlayInfo(playtype, pTartTime, pStopTime);
	info->client->initUserInfo(pUserName, pPassword);
	info->client->initRecvType((RTP_RECV_Type)nRecvType);
	info->client->start();

	if (lpMdeiaParam != NULL)
	{
		info->waitmedia->mediasem.pend(STARTRTSPTIMEOUT);
		memcpy(lpMdeiaParam, &info->waitmedia->mediainfo, sizeof(MEDIA_PARAMETER));
		info->waitmedia = NULL;
	}

	return RTSP_ERR_NO_ERROR;
}

int CALLBACK XM_RTSP_StopRecv( long nTaskHandle )
{
	shared_ptr<RTSPUserInfo>  info;
	{
		Guard locker(clientmutex);
		if (clientManager == NULL)
		{
			return RTSP_ERR_UNINIT;
		}

		std::map<long, shared_ptr<RTSPUserInfo> >::iterator iter = clientMap.find(nTaskHandle);
		if (iter == clientMap.end())
		{
			return RTSP_ERR_INVLID_HANDLE;
		}
		info = iter->second;
	}

	info->client->stop();

	return RTSP_ERR_NO_ERROR;
}

int CALLBACK XM_RTSP_GetConnectState( long nTaskHandle )
{
	return RTSP_ERR_NO_ERROR;
}
