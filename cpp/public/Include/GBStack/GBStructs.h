/**
 * 结构体定义头文件
 *
 */

#pragma once
#include <string>
#include <cstring>
#include <vector>
#include <map>
#include <list>
#include "SIPStack/SIPSession.h"
using namespace std;
using namespace Public::SIPStack;

/**
 *E_PT_DEVICE_CONTROL
 */
//云台控制命令
struct TPtzCmd
{
	bool bControl;		//是否控制云台
	int nAddress;			//地址，暂时不知道有何用途(0-4095)
	EPtzZoomOpt eZoomOpt;	//变倍操作
	EPtzTiltOpt eTiltOpt;	//垂直方向操作
	EPtzPanOpt ePanOpt;		//水平方向操作
	int nTiltSpeed;			//垂直方向速度(0-255)
	int nPanSpeed;			//水平方向速度(0-255)
	int nZoomSpeed;			//变倍速度(0-15)
};
/*
//设备控制命令请求
struct TDeviceControlReq
{
	int nSn;					// *序列号
	string strDeviceId;			// *设备编码
	TPtzCmd oPtzCmd;			//云台控制命令
	BOOL bTeleBoot;				//是否远程启动
	ERecordCmd eRecordCmd;		//录像控制操作	
	EGuardCmd eGuardCmd;		//报警撤防/布防命令
	BOOL bAlarmCmd;				//报警复位操作
	vector<string> vExtendInfo;	//扩展信息

	TDeviceControlReq():bTeleBoot(0),eRecordCmd(E_RC_NONE),eGuardCmd(E_GC_NONE),
		bAlarmCmd(0){memset(&oPtzCmd,0,sizeof(TPtzCmd));}
};
*/

enum Ptz
{
	PtzStop = 0,

	PtzUp,
	PtzDwon,
	PtzLeft,
	PtzRight,
	PtzLU,
	PtzRU,
	PtzLD,
	PtzRD,
	PtzAuto,

	PtzZI,
	PtzZO,

	PtzII,
	PtzIO,
	PtzFI,
	PtzFO,

	PtzYS,
	PtzYD,
	PtzYC,
};
//设备控制命令请求
struct TDeviceControlReq
{
	int nSn;					//*序列号
	string strDeviceId;			//*设备编码
	int ptz;
	int ptzParam;
	bool bTeleBoot;				//是否远程启动
	ERecordCmd eRecordCmd;		//录像控制操作	
	EGuardCmd eGuardCmd;		//报警撤防/布防命令
	bool bAlarmCmd;				//报警复位操作
	vector<string> vExtendInfo;	//扩展信息

	TDeviceControlReq():ptz(-1), bTeleBoot(0),eRecordCmd(E_RC_NONE),eGuardCmd(E_GC_NONE),
		bAlarmCmd(0){}
};

//设备控制命令回应
struct TDeviceControlResp
{
	int nSn;					//*序列号
	string strDeviceId;			//*设备编码
	bool bResult;				//*结果
};



/**
 *E_PT_DEVICE_STATUS_QUERY
 */
//设备状态信息请求
struct TDeviceStatusQueryReq
{
	TDeviceStatusQueryReq():bSubcribe(false){}
	int nSn;					//*序列号
	string strDeviceId;			//*设备编码

	bool   bSubcribe;
	string strStartTime;
	string strEndTime;
};
//报警状态信息
struct TAlarmStatus
{
	string strDeviceId;			//*设备编码
	EAlarmStatus eAlarmStatus;	//*报警状态
};
//设备状态信息回应
struct TDeviceStatusQueryResp
{
	int nSn;					//*序列号
	string strDeviceId;			//*设备编码
	bool bResult;				//*结果(成功、失败)
	bool bOnline;				//*是否在线
	bool bStatus;				//*是否工作
	string strReason;			//不工作的原因(可以为空)
	bool bEncode;				//编码状态
	bool bRecord;				//录像状态
	string strDeviceTime;		//设备时间
	vector<TAlarmStatus> vTAlarmStatus;//报警状态列表
	vector<string> vExtendInfo;	//扩展信息

	TDeviceStatusQueryResp():bEncode(false),bRecord(false){}
};


/**
 *E_PT_DEVICE_STATUS_QUERY
 */
//设备目录查询请求
typedef TDeviceStatusQueryReq TDeviceCatalogQueryReq;
//设备条目信息
struct TDeviceItem
{
	string strDeviceId;										//设备/区域/系统编码(20个字符标识)
	string strName;												//设备/区域/系统名称
	string strManufacturer;								//设备厂商
	string strFirameware;								//固件版本 add by zhongwu
	string strModel;											//设备型号
	string strOwner;											//设备归属
	string strCivilCode;										//行政区域
	string strBlock;												//警区
	string strAddress;											//安装地址
	int nParental;												//是否有子设备
	string strParentId;										//父设备/区域/系统ID(20个字符标识)
	int safetyMode;											//信令安全模式
	int nRegisterWay;										//注册方法
	string certificateNo;										//证书序列号
	string certificateIndentify;							//证书有效识别
	int certificateError;										//无效原因码
	string certificateEnding;								//证书终止有效期
	int nSecrecy;													//	保密属性
	string strIPAddress;										//设备/区域/系统IP地址
	int nPort;														//设备/区域/系统端口号
	string strPasswd;											//设备口令
	int bStatus;												//设备状态  -1 unused 1:online 0 :offline
	float fLongitude;											//经度
	float fLatitude;												//纬度
	int	  nCode;

	TDeviceItem():nParental(-1),safetyMode(-1),nRegisterWay(-1),nSecrecy(-1),nPort(-1),bStatus(0),fLongitude(-1),fLatitude(-1),nCode(-1){}
};
//设备目录查询回应
struct TDeviceCatalogQueryResp
{
	TDeviceCatalogQueryResp():bSubcribe(false){}
	int nSn;					//*序列号
	int nSumNum;				//总数 -1 表示无效
	bool bEndofFile;			//是否结束
	string strDeviceId;			//*设备编码
	vector<TDeviceItem> vDeviceItem;//设备列表
	vector<string> vExtendInfo;	//扩展信息

	bool			bSubcribe;
	bool			bResult;
};



/**
 *E_PT_DEVICE_INFO_QUERY
 */
//设备信息查询请求
typedef TDeviceStatusQueryReq TDeviceInfoQueryReq;
//设备信息查询回应
struct TDeviceInfoQueryResp
{
	int nSn;					//*序列号
	string strDeviceId;			//*设备编码
	bool bResult;				//*结果
	string strManufacturer;		//设备生产商
	string strModel;			//设备型号
	string strFirmware;			//设备固件版本
	int nMaxCamera;				//通道个数
	int nMaxAlarm;
	vector<string> vExtendInfo;	//扩展信息
	TDeviceInfoQueryResp():nMaxCamera(-1), nMaxAlarm(-1){}
};


/**
 *E_PT_DEVICE_CONTROL
 */
//设备状态通知请求
struct TKeepAliveNotify
{
	int nSn;					//*序列号
	string strDeviceId;			//*设备编码
	bool bStatus;				//*状态
};

struct TAlarmQueryReq
{
	int nSn;					//*序列号
	string strDeviceId;			//*设备编码
	int nStartAlarmPriority;
	int nEndAlarmPriority;
	int nAlarmMethod;			//*报警方式(1-7)
	string strStartTime;
	string strEndTime;
};


/**
 *E_PT_DEVICE_CONTROL
 */
//报警通知请求
struct TAlarmNotifyReq
{
	int nSn;					//*序列号
	string strDeviceId;			//*设备编码
	int nAlarmPriority;			//*报警级别(1-4)
	int nAlarmMethod;			//*报警方式(1-7)
	string strAlarmTime;		//*报警时间
	string strAlarmDescription;	//报警描述
	double dLongitude;			//经度
	double dLatitude;			//维度
	std::map<string,string> vExtendInfo;	//扩展信息

	TAlarmNotifyReq():dLongitude(-1),dLatitude(-1){}
};
//报警通知应答
typedef TDeviceControlResp TAlarmNotifyResp;

struct TAlarmSubcribeReq
{
	int nSn;
	string strDeviceId;
	
};

//历史文件信息查询请求
struct TRecordInfoQueryReq
{
	int nSn;					//*序列号
	string strDeviceId;			//*设备编码
	string strStartTime;		//开始时间
	string strEndTime;			//结束时间
	string strFilePath;			//文件名
	string strAddress;			//地址
	int nSecrecy;				//保密
	ERecordInfoType eType;		//类型
	string strRecorderID;		//触发id
};


//历史文件信息查询回应
struct TRecordItem
{
	string strDeviceId;			//*设备编码
	string strName;				//*名称
	string strStartTime;		//开始时间
	string strEndTime;			//结束时间
	string strFilePath;			//文件名
	string strAddress;			//地址
	unsigned long long	nFileSize;				//文件大小
	int nSecrecy;				//保密
	ERecordInfoType eType;		//类型
	string strRecorderID;		//触发id
};
struct TRecordInfoQueryResp
{
	int nSn;					//*序列号
	string strDeviceId;			//*设备编码
	string strName;				//*开始时间
	int nSumNum;				//*结束时间
	bool bEndofFile;			//是否结束
	vector<TRecordItem> vRecordList;
};

//E_PT_DECODER_CONTROL_REQ
struct TDecoderControlReq
{
	int nSn;
	string strDecoderId;
	string strDeviceId;
	bool bStart;
};
typedef TDeviceControlResp TDecoderControlResp;

//E_PT_DECODER_INFO_QUERY_REQ
struct TDecoderInfoQueryReq
{
	int nSn;
	string strDecoderId;
};

//E_PT_DECODER_INFO_QUERY_RESP
struct TDecoderInfo
{
	string strDecoderId;
	string strDeviceId;
};
struct TDecoderInfoQueryResp
{
	int nSn;
	string strDeviceId;
	vector<TDecoderInfo> vDecoderInfo;
};

typedef struct{
	string		 DeviceAor;			//设备地址编码
	string		 DestIpAddr;			//请求者IP地址
	int			 DestPort;			//请求者接受端口
	INVITETYPE_E PlayType;
	long		 StartTime;
	long		 EndTime;
	bool   		 IsOutPlatForm;
	int			 cseq;
}TInviteIMaxReq;


typedef struct{
	string		DeviceAor;			//设备地址编码
	int 		Result;				//执行结果
}TIviteIMaxResp;

struct TVodCtrlReq
{
	TVodCtrlReq():lStartTime(0),dSpeed(1){}
	ERtspType eType;//类型
	int nSn;
	long lStartTime;
	double dSpeed;
};

struct TVodCtrlResp
{
	TVodCtrlResp():nSn(0),nResult(0),rtpinfo_seq(0),rtpinfo_time(0){}
	int nSn;
	int nResult;
	long rtpinfo_seq;
	long rtpinfo_time;
};

typedef struct{
	int			nSn;
	string		strDeviceId;
	int			nType;
}MediaStatusNotify;


