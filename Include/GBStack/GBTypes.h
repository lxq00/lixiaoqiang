/**
 * 类型定义文件(包括错误码的定义)
 */

#include "GBStack/GBStackDefs.h"
#include <string>

using namespace std;

#pragma once


/**
 * 类型定义
 */
enum EProtocolType
{
	/*控制*/
	//设备控制请求TDeviceControlReq
	E_PT_DEVICE_CONTROL_REQ				= 1,
	//设备控制回应TDeviceControlResp
	E_PT_DEVICE_CONTROL_RESP			= 2,
	
	/*查询*/
	//设备状态查询()
	E_PT_DEVICE_STATUS_QUERY_REQ		= 11,
	E_PT_DEVICE_STATUS_QUERY_RESP		= 12,
	//设备目录信息查询
	E_PT_DEVICE_CATALOG_QUERY_REQ		= 13,
	E_PT_DEVICE_CATALOG_QUERY_RESP		= 14,
	//设备信息查询 
	E_PT_DEVICE_INFO_QUERY_REQ			= 15,
	E_PT_DEVICE_INFO_QUERY_RESP			= 16,
	//录像信息查询
	E_PT_RECORD_INFO_QUERY_REQ			= 17,
	E_PT_RECORD_INFO_QUERY_RESP			= 18,


	/*通知*/
	//设备状态通知(无回应)
	E_PT_KEEP_ALIVE_NOTIFY				= 20,
	//报警通知
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

//PTZ镜头变倍操作
enum EPtzZoomOpt
{
	E_ZOOM_NONE,		//无操作
	E_ZOOM_OUT,			//缩小
	E_ZOOM_IN,			//放大
};

//PTZ垂直方向操作
enum EPtzTiltOpt
{
	E_TILT_NONE,		//无操作
	E_TILT_DOWN,		//下
	E_TILT_UP,			//上
};

//PTZ水平方向操作
enum EPtzPanOpt
{
	E_PAN_NONE,			//无操作
	E_PAN_RIGHT,		//右
	E_PAN_LEFT,			//左
};

//录像控制操作
enum ERecordCmd
{
	E_RC_NONE,			//无操作
	E_RC_RECORD,		//录像
	E_RC_STOP_RECORD,	//停止录像
};

//报警撤防布防命令
enum EGuardCmd
{
	E_GC_NONE,			//无操作
	E_GC_SET_GUARD,		//布防
	E_GC_REST_GUARD,	//撤防
};

//设备报警状态
enum EAlarmStatus
{
	E_AS_ONDUTY,
	E_AS_OFFDUTY,
	E_AS_ALARM,
};

//文件查询类型
enum ERecordInfoType
{
	E_FQT_NONE,
	E_FQT_ALL,
	E_FQT_TIME,
	E_FQT_ALARM,
	E_FQT_MANUAL,
};


//rtsp类型
enum ERtspType
{
	//E_RT_RESP,	//回应
	E_RT_PLAY,	//播放
	E_RT_PAUSE,	//暂停
	E_RT_TEARDOWN,	//终止
};

enum EErrorCode
{
	E_EC_OK,				//成功
	E_EC_INVALID_DOC,	//无效的文档
	E_EC_PARSE_XML_DOC,		//解析xml文档出错
	E_EC_INVALID_CMD_TYPE,	//无效的命令类型
	E_EC_INVALID_PARAM,		//无效的参数
};

GB28181_API unsigned long long SipTimeToSec(const string& timefromt);

GB28181_API string SecToSipTime(unsigned long long sectime);

GB28181_API string SecToSipRegTime(unsigned long long sectime);
