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
		/// SDK��ʼ��������Socket��ʼ��
		/// Return ������
		int RTSPCLIENT_API CALLBACK XM_RTSP_Init();

		/// SDK������
		/// Return ������
		int RTSPCLIENT_API CALLBACK XM_RTSP_UnInit();

		/// ���һ��RTSP Client����
		/// @[in]  param pUrl ý��RTSP url
		/// @[out] param lClientID ����������ǰClient�Ĳ���ƾ֤
		/// Return ������
		int RTSPCLIENT_API CALLBACK XM_RTSP_AddTask(char* pUrl, long* nTaskHandle);

		/// ɾ������
		/// @[in] param lClientID ��Ҫ���ٵ�������
		/// Return ������
		int RTSPCLIENT_API CALLBACK XM_RTSP_DelTask(long nTaskHandle);

		///�������ݻص�
		///@[in] param lClientID ������
		///@[in] param pFunc �ص�����ָ��
		///@[in] param nUserParam1 �û�����1
		///@[in] param nUserParam2 �û�����2
		///Return ������
		int RTSPCLIENT_API CALLBACK XM_RTSP_SetStreamCallbackFunction(long nTaskHandle, RtspClientDataCallBackFun pFunc, long nUserParam1, void* pUserParam2);

		///����״̬�ص�
		///@[in] param pFunc �ص�����ָ��
		///@[in] param nUserParam1 �û�����1
		///@[in] param nUserParam2 �û�����2
		///Return ������
		int RTSPCLIENT_API CALLBACK XM_RTSP_SetConnectionCallbackFunction(long nTaskHandle, RTSP_CLIENT_CONNECT_CALLBACK_FUNCTION pFunc, long nUserParam1, void* pUserParam2);

		///�������ݽ���
		///@[in] param lClientID ������
		///@[in] param nRecvType ���ݽ��շ�ʽ 0ΪTCP 1ΪUDP
		///Return ������
		int RTSPCLIENT_API CALLBACK XM_RTSP_StartRecv(long nTaskHandle, int nRecvType, LPMEDIA_PARAMETER lpMdeiaParam);

		///�������ݽ�����չ�ӿڣ�������Ҫ��Ȩ��֤��url
		///@[in] param lClientID ������
		///@[in] param pUserName �û���
		///@[in] param pPassword ����
		///@[in] param nRecvType ���ݽ��շ�ʽ 0ΪTCP 1ΪUDP
		///Return ������
		int RTSPCLIENT_API CALLBACK XM_RTSP_StartRecvWithUserInfo(int nTaskHandle, char *pUserName, char* pPassword, int nRecvType, LPMEDIA_PARAMETER lpMdeiaParam);

		int RTSPCLIENT_API CALLBACK XM_RTSP_StartRecvWithUserInfoEX(int nTaskHandle, char *pUserName, char* pPassword, int nRecvType, LPMEDIA_PARAMETER lpMdeiaParam, int nTimeType, char* pTartTime, char *pStopTime);

		///ֹͣ���ݽ���
		///@[in] param lClientID ������
		///Return ������
		int RTSPCLIENT_API CALLBACK XM_RTSP_StopRecv(long nTaskHandle);

		///��ȡ����״̬
		///@[in] param lClientID ������
		///Return ������
		int RTSPCLIENT_API CALLBACK XM_RTSP_GetConnectState(long nTaskHandle);

#ifdef __cplusplus
	}
#endif

#endif