/**
 * ���Ͷ����ļ�(����������Ķ���)
 */

#include "GBStack/GBStackDefs.h"
#include <string>

using namespace std;

#pragma once


/**
 * ���Ͷ���
 */
enum EProtocolType
{
	/*����*/
	//�豸��������TDeviceControlReq
	E_PT_DEVICE_CONTROL_REQ				= 1,
	//�豸���ƻ�ӦTDeviceControlResp
	E_PT_DEVICE_CONTROL_RESP			= 2,
	
	/*��ѯ*/
	//�豸״̬��ѯ()
	E_PT_DEVICE_STATUS_QUERY_REQ		= 11,
	E_PT_DEVICE_STATUS_QUERY_RESP		= 12,
	//�豸Ŀ¼��Ϣ��ѯ
	E_PT_DEVICE_CATALOG_QUERY_REQ		= 13,
	E_PT_DEVICE_CATALOG_QUERY_RESP		= 14,
	//�豸��Ϣ��ѯ 
	E_PT_DEVICE_INFO_QUERY_REQ			= 15,
	E_PT_DEVICE_INFO_QUERY_RESP			= 16,
	//¼����Ϣ��ѯ
	E_PT_RECORD_INFO_QUERY_REQ			= 17,
	E_PT_RECORD_INFO_QUERY_RESP			= 18,


	/*֪ͨ*/
	//�豸״̬֪ͨ(�޻�Ӧ)
	E_PT_KEEP_ALIVE_NOTIFY				= 20,
	//����֪ͨ
	E_PT_ALARM_NOTIFY_REQ				= 22,
	E_PT_ALARM_NOTIFY_RESP				= 23,

	//FILE TO END
	E_PT_MEDIASTATUS_NOTIFY				= 30,

	E_PT_CATALOG_SUBSCRIBE_REQ			= 31,
	E_PT_ALARM_SUBSCRIBE_REQ			= 32,

	/**/
	E_PT_INVITE_REQ						= 40,
	E_PT_INVITE_RESP					= 41,
	E_PT_IVITE_BYE						= 42,
	E_PT_VOD_CTRL_REQ					= 43,
	E_PT_VOD_CTRL_RESP					= 44,


	E_PT_DECODER_CONTROL_REQ			= 50,
	E_PT_DECODER_CONTROL_RESP			= 49,
	E_PT_DECODER_INFO_QUERY_REQ			= 51,
	E_PT_DECODER_INFO_QUERY_RESP		= 52,

	
	E_PT_CHECK_TIME  					= 53,//struct tm

	E_PT_MAX,
};

//PTZ��ͷ�䱶����
enum EPtzZoomOpt
{
	E_ZOOM_NONE,		//�޲���
	E_ZOOM_OUT,			//��С
	E_ZOOM_IN,			//�Ŵ�
};

//PTZ��ֱ�������
enum EPtzTiltOpt
{
	E_TILT_NONE,		//�޲���
	E_TILT_DOWN,		//��
	E_TILT_UP,			//��
};

//PTZˮƽ�������
enum EPtzPanOpt
{
	E_PAN_NONE,			//�޲���
	E_PAN_RIGHT,		//��
	E_PAN_LEFT,			//��
};

//¼����Ʋ���
enum ERecordCmd
{
	E_RC_NONE,			//�޲���
	E_RC_RECORD,		//¼��
	E_RC_STOP_RECORD,	//ֹͣ¼��
};

//����������������
enum EGuardCmd
{
	E_GC_NONE,			//�޲���
	E_GC_SET_GUARD,		//����
	E_GC_REST_GUARD,	//����
};

//�豸����״̬
enum EAlarmStatus
{
	E_AS_ONDUTY,
	E_AS_OFFDUTY,
	E_AS_ALARM,
};

//�ļ���ѯ����
enum ERecordInfoType
{
	E_FQT_NONE,
	E_FQT_ALL,
	E_FQT_TIME,
	E_FQT_ALARM,
	E_FQT_MANUAL,
};


//rtsp����
enum ERtspType
{
	//E_RT_RESP,	//��Ӧ
	E_RT_PLAY,	//����
	E_RT_PAUSE,	//��ͣ
	E_RT_TEARDOWN,	//��ֹ
};

enum EErrorCode
{
	E_EC_OK,				//�ɹ�
	E_EC_INVALID_DOC,	//��Ч���ĵ�
	E_EC_PARSE_XML_DOC,		//����xml�ĵ�����
	E_EC_INVALID_CMD_TYPE,	//��Ч����������
	E_EC_INVALID_PARAM,		//��Ч�Ĳ���
};

GB28181_API unsigned long long SipTimeToSec(const string& timefromt);

GB28181_API string SecToSipTime(unsigned long long sectime);

GB28181_API string SecToSipRegTime(unsigned long long sectime);
