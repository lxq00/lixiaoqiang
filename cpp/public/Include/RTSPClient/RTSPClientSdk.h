#ifndef _RTSP_CLIENT_SDK_H_
#define _RTSP_CLIENT_SDK_H_
#include "RTSPStructs.h"
#include "RTSPClientErrorCode.h"
#include <stdint.h>
#ifdef WIN32
#include <Windows.h>
#else
#define CALLBACK
#endif

#ifdef WIN32
	#ifdef RTSPCLIENT_EXPORTS
		#define RTSPCLIENT_API __declspec(dllexport)
	#else
		#define RTSPCLIENT_API __declspec(dllimport)
	#endif
#else
	#define RTSPCLIENT_API
#endif

typedef long  (CALLBACK *RtspClientDataCallBackFun)(const unsigned char* pData, long nLength, LPFRAME_INFO sFrameInfo, long nUserParam1, void* pUserParam2);
typedef long  (CALLBACK *RTSP_CLIENT_CONNECT_CALLBACK_FUNCTION)(long nTaskHandle, long nConnectStateCode, void* pUserParam2);


#ifdef __cplusplus
	extern "C" {
#endif
		/// SDK初始化，包括Socket初始化
		/// Return 错误码
		int RTSPCLIENT_API CALLBACK XM_RTSP_Init();

		/// SDK清理工作
		/// Return 错误码
		int RTSPCLIENT_API CALLBACK XM_RTSP_UnInit();

		/// 添加一个RTSP Client任务
		/// @[in]  param pUrl 媒体RTSP url
		/// @[out] param lClientID 任务句柄，当前Client的操作凭证
		/// Return 错误码
		int RTSPCLIENT_API CALLBACK XM_RTSP_AddTask(char* pUrl, long* nTaskHandle);

		/// 删除任务
		/// @[in] param lClientID 需要销毁的任务句柄
		/// Return 错误码
		int RTSPCLIENT_API CALLBACK XM_RTSP_DelTask(long nTaskHandle);

		///设置数据回调
		///@[in] param lClientID 任务句柄
		///@[in] param pFunc 回调函数指针
		///@[in] param nUserParam1 用户数据1
		///@[in] param nUserParam2 用户数据2
		///Return 错误码
		int RTSPCLIENT_API CALLBACK XM_RTSP_SetStreamCallbackFunction(long nTaskHandle, RtspClientDataCallBackFun pFunc, long nUserParam1, void* pUserParam2);

		///连接状态回调
		///@[in] param pFunc 回调函数指针
		///@[in] param nUserParam1 用户数据1
		///@[in] param nUserParam2 用户数据2
		///Return 错误码
		int RTSPCLIENT_API CALLBACK XM_RTSP_SetConnectionCallbackFunction(long nTaskHandle, RTSP_CLIENT_CONNECT_CALLBACK_FUNCTION pFunc, long nUserParam1, void* pUserParam2);

		///开启数据接收
		///@[in] param lClientID 任务句柄
		///@[in] param nRecvType 数据接收方式 0为TCP 1为UDP
		///Return 错误码
		int RTSPCLIENT_API CALLBACK XM_RTSP_StartRecv(long nTaskHandle, int nRecvType, LPMEDIA_PARAMETER lpMdeiaParam);

		///开启数据接收扩展接口，用于需要授权验证的url
		///@[in] param lClientID 任务句柄
		///@[in] param pUserName 用户名
		///@[in] param pPassword 密码
		///@[in] param nRecvType 数据接收方式 0为TCP 1为UDP
		///Return 错误码
		int RTSPCLIENT_API CALLBACK XM_RTSP_StartRecvWithUserInfo(int nTaskHandle, char *pUserName, char* pPassword, int nRecvType, LPMEDIA_PARAMETER lpMdeiaParam);

		int RTSPCLIENT_API CALLBACK XM_RTSP_StartRecvWithUserInfoEX(int nTaskHandle, char *pUserName, char* pPassword, int nRecvType, LPMEDIA_PARAMETER lpMdeiaParam, int nTimeType, char* pTartTime, char *pStopTime);

		///停止数据接收
		///@[in] param lClientID 任务句柄
		///Return 错误码
		int RTSPCLIENT_API CALLBACK XM_RTSP_StopRecv(long nTaskHandle);

		///获取连接状态
		///@[in] param lClientID 任务句柄
		///Return 错误码
		int RTSPCLIENT_API CALLBACK XM_RTSP_GetConnectState(long nTaskHandle);

#ifdef __cplusplus
	}
#endif

#endif