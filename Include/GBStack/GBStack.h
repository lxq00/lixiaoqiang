#ifndef __GBSTACK_H__
#define __GBSTACK_H__

#include <stdint.h>
#include <list>
#include <map>
#include <set>
using namespace std;
#include "GBStack/GBStackDefs.h"
#include "SIPStack/SIPSession.h"
#include "GBStack/GBTypes.h"
#include "GBStack/GBStructs.h"
#include "Base/Base.h"
using namespace Public::Base;
using namespace Public::SIPStack;
namespace Public{
namespace GB28181{

/// GBEventID 事件ID
typedef void* GBEventID;
/// 通讯ID
typedef void* GBCommuID;
/// 平台序号
typedef void* GBPlatformID;


enum EVENTId_Error
{
	EVENTId_Error_NoError = 0,
	EVENTId_Error_SendError = -1,
	EVENTId_Error_NotRegist = -2,
	EVENTId_Error_Invaild = -4,
	EVENTId_Error_SessionNotExist = -5,
	EVENTId_Error_ParaInvaild = -6,
	EVENTId_Error_CommuNotExist = -7,
	EVENTId_Error_PlatformNotExist = -8,
};

enum GBAlarmMethod
{
	GBAlarmMethod_All 				= 0x00,
	GBAlarmMethod_Phone			= 0x01,
	GBAlarmMethod_Device			= 0x02,
	GBAlarmMethod_SMS				= 0x03,
	GBAlarmMethod_GPS				= 0x04,
	GBAlarmMethod_Video			= 0x05,
	GBAlarmMethod_DevTrouble		= 0x06,
	GBAlarmMethod_Ohter			= 0x07,
	GBAlarmMethod_DeviceOnline	= 0x80,
	GBAlarmMethod_DeviceOffline	= 0x81,
	GBAlarmMethod_VideoLost		= 0x82,
	GBAlarmMethod_VideoOCC		= 0x83,
	GBAlarmMethod_VMD				= 0x84,
};

enum GBPtzDirection
{
	GBPtzDirectionStop = 0,		//停止

	GBPtzDirectionUp,			//向上
	GBPtzDirectionDwon,			//向下
	GBPtzDirectionLeft,			//向左
	GBPtzDirectionRight,		//向右
	GBPtzDirectionLU,			//左上
	GBPtzDirectionRU,			//右上
	GBPtzDirectionLD,			//左下
	GBPtzDirectionRD,			//右下
	GBPtzDirectionAuto,			//自动

	GBPtzDirectionZI,			//放大
	GBPtzDirectionZO,			//缩小

	GBPtzDirectionII,			//光圈放大
	GBPtzDirectionIO,			//光圈缩小
	GBPtzDirectionFI,			//聚焦远
	GBPtzDirectionFO,			//聚焦近

	GBPtzDirectionYS,			//预置位设置
	GBPtzDirectionYD,			//预置位删除
	GBPtzDirectionYC,			//预置位调用
};

struct GBDeviceStatus
{
	GBDeviceStatus(){}
	
	bool 						onLine;
	bool 						workStatus;
	bool						encodeStatus;
	bool						recordStatus;
	std::string 				deviceTime;
	std::list<TAlarmStatus>		alarmStatus;
};

struct GBDeviceInfo
{
	GBDeviceInfo(){}
	
	std::string 	manufacturer;
	std::string 	model;
	std::string		firmWare;
	int 			cameraNum;
	int 			alarmNum;
};

typedef struct{
	std::string 	decoderId;
	std::string 	playDeviceID;
}ThirdCallInfo;

struct RegisterInfo
{
	RegisterInfo(){}
	std::string			platformId;
	std::string			platformIp;
	int					platformPort;
	std::string			mySipAddr;
	int					mySipPort;
	unsigned long long	dateTime;
	SIPSocketType		socketType;

	std::string			requestIp;
	std::string			fromIp;
	std::string			toIp;
	std::string			authenIp;
};

enum CloseRegistError
{
	CloseRegistError_TimeOut,
	CloseRegistError_Closed,
	CloseRegistError_NetError,
};

struct CataLogInfo
{
	CataLogInfo():SumNum(0){}
	
	bool EndOfFile;
	int SumNum;
	std::list<TDeviceItem> resp;
};

struct RecordInfo
{
	RecordInfo():SumNum(0){}
	
	int SumNum;
	bool EndOfFile;
	std::list<TRecordItem> resp;
};

struct AlarmNotifyInfo
{
	AlarmNotifyInfo(){}
	int 			Priority;
	int 			Method;
	long long 		Time;
	std::string 	Description;
	std::map<std::string,std::string>	ExtendInfo;
};

typedef enum{
	GB28181TYPE_ERROR = 0,
	GB28181TYPE_DEVICE = 100,
	GB28181TYPE_SIPSVR = 200,
	GB28181TYPE_WEBSVR,
	GB28181TYPE_MEDIASVR,
	GB28181TYPE_PROXYSVR,
	GB28181TYPE_SAFESVR,
	GB28181TYPE_ALARMSVR,
	GB28181TYPE_DBSVR,
	GB28181TYPE_GISSVR,
	GB28181TYPE_ADMINSVR,
	GB28181TYPE_GATEWAY,
	GB28181TYPE_EXTENDPLATFORM,
	GB28181TYPE_CENTERUSER = 300,
	GB28181TYPE_CLIENT = 400,
	GB28181TYPE_EXTEND = 500,
}GB28181TYPE;

typedef enum
{
	MediaSatus_Disconnect = 120,
	MediaSatus_Eof = 121,
	MediaSatus_FileError = -1
}MediaSatus;

struct StreamSDPInfo
{
	SIPSocketType		SocketType;
	std::string			IpAddr;
	uint32_t			Port;
	std::string			Ssrc;

	std::list<SDPMediaInfo>	sdpList;

	StreamSDPInfo(){}
};

struct AlarmSubcribe
{
	int StartAlarmPriority;
	int EndAlarmPriority;
	int AlarmMethod;			//*报警方式(1-7)
	unsigned long long StartTime;
	unsigned long long EndTime;
};

class GBDispacher;
class GBStackInternal;
class GB28181_API GBStack:public enable_shared_from_this<GBStack>
{	
public:
	GBStack();
	~GBStack();
	
	bool init(const weak_ptr<GBDispacher>& dispacher, const std::string& myId, const std::string& myUser, const std::string& myPass, const std::string& uAgent);

	EVENTId_Error udpListen(GBCommuID& commuId,int myPort,const std::string& myIp = "");
	EVENTId_Error tcpListen(GBCommuID& commuId,int myPort);
	EVENTId_Error tcpConnect(GBCommuID& commuId,const std::string& destIp,int destPort);
	EVENTId_Error closeCommu(GBCommuID commuId);
	
	EVENTId_Error sendRegisteReq(GBPlatformID& platformId,GBCommuID commuId,const std::string& name,const std::string& pass,const std::string& svrId,const std::string& svrIp = "",int svrPort = 0,int expries = 30);
	EVENTId_Error sendRegisteResp(GBCommuID commuId,GBPlatformID platformId, const SIPStackError& error,unsigned long long dateTime = 0);
	EVENTId_Error closeRegiste(GBCommuID commuId,GBPlatformID platformId);

	EVENTId_Error sendGetCatalogReq(GBEventID& eventid,GBPlatformID platformId);
	EVENTId_Error sendGetCatalogResp(GBPlatformID platformId,GBEventID eventId, const SIPStackError& error,uint32_t total = 0,bool isEnd = true,const std::list<TDeviceItem>& resp = std::list<TDeviceItem>());
	
	EVENTId_Error sendDevicePtzReq(GBEventID& eventid,GBPlatformID platformId,const std::string& devid,GBPtzDirection direction,int speed);
	EVENTId_Error sendDeviceRebootReq(GBEventID& eventid,GBPlatformID platformId,const std::string& devid);
	EVENTId_Error sendDeviceRecordReq(GBEventID& eventid,GBPlatformID platformId,const std::string& devid,bool record);
	EVENTId_Error sendAlarmGuardReq(GBEventID& eventid,GBPlatformID platformId,const std::string& devid,bool guard);
	EVENTId_Error sendAlarmResetReq(GBEventID& eventid,GBPlatformID platformId,const std::string& devid);
	EVENTId_Error sendDeviceContrlResp(GBPlatformID platformId,GBEventID eventId, const SIPStackError& error);

	EVENTId_Error sendAlarmSubscribeReq(GBEventID& eventid,GBPlatformID platformId,const std::string& devid,const AlarmSubcribe& subcribe);
	EVENTId_Error sendAlarmSubscribeResp(GBPlatformID platformId,GBEventID eventId, const SIPStackError& error);

	EVENTId_Error sendCatalogSubscribeReq(GBEventID& eventid,GBPlatformID platformId,const std::string& devid,long long startTime,long long endTime);
	EVENTId_Error sendCatalogSubscribeResp(GBPlatformID platformId,GBEventID eventId, const SIPStackError& error);

	EVENTId_Error sendCatalogNotifyReq(GBPlatformID platformId,const std::string& devid,uint32_t total,bool isEnd,const std::list<TDeviceItem>& devlist);
	EVENTId_Error sendCatalogNotifyResp(GBPlatformID platformId,GBEventID eventId, const SIPStackError& error);
	
	EVENTId_Error sendAlarmNotifyReq(GBEventID& eventid,GBPlatformID platformId,const std::string& devid,const AlarmNotifyInfo& info);
	EVENTId_Error sendAlarmNotifyResp(GBPlatformID platformId,GBEventID eventId, const SIPStackError& error);

	EVENTId_Error sendDeviceStatusNotify(GBPlatformID platformId,const std::string& devid,bool online);
	
	EVENTId_Error sendQueryDeviceStatusReq(GBEventID& eventid,GBPlatformID platformId,const std::string& devid);
	EVENTId_Error sendQueryDeviceStatusResp(GBPlatformID platformId,GBEventID eventId, const SIPStackError& error,const GBDeviceStatus& status = GBDeviceStatus());
	
	EVENTId_Error sendQueryDeviceInfoReq(GBEventID& eventid,GBPlatformID platformId,const std::string& devid);
	EVENTId_Error sendQueryDeviceInfoResp(GBPlatformID platformId,GBEventID eventId, const SIPStackError& error,const GBDeviceInfo& info = GBDeviceInfo());
	
	EVENTId_Error sendQueryRecordInfoReq(GBEventID& eventid,GBPlatformID platformId,const std::string& devid,long long startTime,long long endTime,ERecordInfoType type);
	EVENTId_Error sendQueryRecordInfoResp(GBPlatformID platformId,GBEventID eventId, const SIPStackError& error,const std::string& name = "",uint32_t total = 0,bool isEnd = true,const std::list<TRecordItem>& resp= std::list<TRecordItem>());
	
	EVENTId_Error sendopenRealplayStreamReq(GBEventID& streamID,GBPlatformID platformId,const std::string& devid,int streamType,const StreamSDPInfo& sdp);
	EVENTId_Error sendopenPlaybackStreamReq(GBEventID& streamID,GBPlatformID platformId,const std::string& devid,long long startTime,long long endTime,const StreamSDPInfo& sdp);
	EVENTId_Error sendopenDownloadStreamReq(GBEventID& streamID,GBPlatformID platformId,const std::string& devid,long long startTime,long long endTime,int downSpeed,const StreamSDPInfo& sdp);
	EVENTId_Error sendOpenStreamResp(GBPlatformID platformId,GBEventID streamID, const SIPStackError& error, const StreamSDPInfo& sdp = StreamSDPInfo());
//	EVENTId_Error sendOpenStreamAck(GBPlatformID platformId,GBEventID streamID,const std::string& sendip = "",int sendport = 0);

	EVENTId_Error sendcloseStreamReq(GBPlatformID platformId,GBEventID streamID);
	EVENTId_Error sendcloseStreamResp(GBPlatformID platformId,GBEventID streamID, const SIPStackError& error);
	
	EVENTId_Error sendPlaybackContrlReq(GBEventID& eventId,GBPlatformID platformId,GBEventID streamID,float speed,long long time);
	EVENTId_Error sendPlaybackContrlResp(GBPlatformID platformId,GBEventID eventId, const SIPStackError& error,long rtpseq = 0,long rtpTime = 0);

	EVENTId_Error sendMediaStatus(GBPlatformID platformId,GBEventID streamID,MediaSatus status);

	static GB28181TYPE getGB28181Type(const std::string& id);
public:
	struct CommunicationInfo
	{
		enum _commutype
		{
			CommuType_Udp,
			CommuType_Tcp,
		}type;
		std::string			IpAddr;
		uint32_t			Port;
	};
	EVENTId_Error getCommunicationInfos(CommunicationInfo&info,GBPlatformID platformId);
private:
	shared_ptr<GBStackInternal> internal;
};

class GB28181_API GBDispacher :public enable_shared_from_this<GBDispacher>
{
public:
	GBDispacher(){}
	virtual ~GBDispacher(){}

	virtual void onRegistReq(const weak_ptr<GBStack>& gb,GBCommuID commuId,GBPlatformID platformId,const RegisterInfo& info)
	{
		shared_ptr<GBStack> gbptr = gb.lock();
		if (gbptr != NULL) 
		{
			gbptr->sendRegisteResp(commuId, platformId, SIPStackError_NoSuport);
		}
	}

	virtual void onRegistResp(const weak_ptr<GBStack>& gb,GBPlatformID platformId,const SIPStackError& error,const RegisterInfo& info = RegisterInfo()){}
	virtual void onRegistClosed(const weak_ptr<GBStack>& gb,GBCommuID commuId,GBPlatformID platformId,CloseRegistError error){}

	virtual void onGetCatalogReq(const weak_ptr<GBStack>& gb,GBPlatformID platformId,GBEventID id)
	{
		shared_ptr<GBStack> gbptr = gb.lock();
		if (gbptr != NULL)
		{
			gbptr->sendGetCatalogResp(platformId, id, SIPStackError_NoSuport);
		}
	}
	virtual void onGetCatalogResp(const weak_ptr<GBStack>& gb,GBPlatformID platformId,GBEventID id, const SIPStackError& error,const CataLogInfo& catalog = CataLogInfo()){}

	virtual void onDevicePtzReq(const weak_ptr<GBStack>& gb,GBPlatformID platformId,GBEventID id,const std::string& devid,GBPtzDirection direction,int speed)
	{
		shared_ptr<GBStack> gbptr = gb.lock();
		if (gbptr != NULL)
		{
			gbptr->sendDeviceContrlResp(platformId, id, SIPStackError_NoSuport);
		}
	}
	virtual void onDeviceRebootReq(const weak_ptr<GBStack>& gb,GBPlatformID platformId,GBEventID id,const std::string& devid)
	{
		shared_ptr<GBStack> gbptr = gb.lock();
		if (gbptr != NULL)
		{
			gbptr->sendDeviceContrlResp(platformId, id, SIPStackError_NoSuport);
		}
	}
	virtual void onDeviceRecordReq(const weak_ptr<GBStack>& gb,GBPlatformID platformId,GBEventID id,const std::string& devid,bool record)
	{
		shared_ptr<GBStack> gbptr = gb.lock();
		if (gbptr != NULL)
		{
			gbptr->sendDeviceContrlResp(platformId, id, SIPStackError_NoSuport);
		}
	}
	virtual void onAlarmGuardReq(const weak_ptr<GBStack>& gb,GBPlatformID platformId,GBEventID id,const std::string& devid,bool guard)
	{
		shared_ptr<GBStack> gbptr = gb.lock();
		if (gbptr != NULL)
		{
			gbptr->sendDeviceContrlResp(platformId, id, SIPStackError_NoSuport);
		}
	}
	virtual void onAlarmResetReq(const weak_ptr<GBStack>& gb,GBPlatformID platformId,GBEventID id,const std::string& devid)
	{
		shared_ptr<GBStack> gbptr = gb.lock();
		if (gbptr != NULL)
		{
			gbptr->sendDeviceContrlResp(platformId, id, SIPStackError_NoSuport);
		}
	}
	virtual void onDeviceContrlResp(const weak_ptr<GBStack>& gb,GBPlatformID platformId,GBEventID id, const SIPStackError& error){}

	virtual void onAlarmSubscribeReq(const weak_ptr<GBStack>& gb,GBPlatformID platformId,GBEventID id,const std::string& devid,const AlarmSubcribe& subcribe)
	{
		shared_ptr<GBStack> gbptr = gb.lock();
		if (gbptr != NULL)
		{
			gbptr->sendAlarmSubscribeResp(platformId, id, SIPStackError_NoSuport);
		}
	}
	virtual void onAlarmSubscribeResp(GBPlatformID platformId,GBEventID eventId, const SIPStackError& error){}

	virtual void onCatalogSubscribeReq(const weak_ptr<GBStack>& gb,GBPlatformID platformId,GBEventID id,const std::string& devid,unsigned long long startTime,unsigned long long endTime)
	{
		shared_ptr<GBStack> gbptr = gb.lock();
		if (gbptr != NULL)
		{
			gbptr->sendCatalogSubscribeResp(platformId, id, SIPStackError_NoSuport);
		}
	}
	virtual void onCatalogSubscribeResp(GBPlatformID platformId,GBEventID eventId, const SIPStackError& error){}

	virtual void onCatalogNotify(const weak_ptr<GBStack>& gb,GBPlatformID platformId, GBEventID id,const CataLogInfo& cata)
	{
		shared_ptr<GBStack> gbptr = gb.lock();
		if (gbptr != NULL)
		{
			gbptr->sendCatalogNotifyResp(platformId, id, SIPStackError_NoSuport);
		}
	}

	virtual void onAlarmNotifyReq(const weak_ptr<GBStack>& gb,GBPlatformID platformId,GBEventID id,const std::string& devid,const AlarmNotifyInfo& info)
	{
		shared_ptr<GBStack> gbptr = gb.lock();
		if (gbptr != NULL)
		{
			gbptr->sendAlarmNotifyResp(platformId, id, SIPStackError_NoSuport);
		}
	}
	virtual void onAlarmNotifyResp(const weak_ptr<GBStack>& gb,GBPlatformID platformId,GBEventID id, const SIPStackError& error){}

	virtual void onDeviceStatusNotify(const weak_ptr<GBStack>& gb,GBPlatformID platformId,const std::string& devid,bool online){}

	virtual void onQueryDeviceStatusReq(const weak_ptr<GBStack>& gb,GBPlatformID platformId,GBEventID id,const std::string& devid)
	{
		shared_ptr<GBStack> gbptr = gb.lock();
		if (gbptr != NULL)
		{
			gbptr->sendQueryDeviceStatusResp(platformId, id, SIPStackError_NoSuport);
		}
	}
	virtual void onQueryDeviceStatusResp(const weak_ptr<GBStack>& gb,GBPlatformID platformId,GBEventID id, const SIPStackError& error,const GBDeviceStatus& status = GBDeviceStatus()){}

	virtual void onQueryDeviceInfoReq(const weak_ptr<GBStack>& gb,GBPlatformID platformId,GBEventID id,const std::string& devid)
	{
		shared_ptr<GBStack> gbptr = gb.lock();
		if (gbptr != NULL)
		{
			gbptr->sendQueryDeviceInfoResp(platformId, id, SIPStackError_NoSuport);
		}
	}
	virtual void onQueryDeviceInfoResp(const weak_ptr<GBStack>& gb,GBPlatformID platformId,GBEventID id, const SIPStackError& error,const GBDeviceInfo& info = GBDeviceInfo()){}

	virtual void onQueryRecordInfoReq(const weak_ptr<GBStack>& gb,GBPlatformID platformId,GBEventID id,const std::string& devid,long long startTime,long long endTime,ERecordInfoType type)
	{
		shared_ptr<GBStack> gbptr = gb.lock();
		if (gbptr != NULL)
		{
			gbptr->sendQueryRecordInfoResp(platformId, id, SIPStackError_NoSuport);
		}
	}
	virtual void onQueryRecordInfoResp(const weak_ptr<GBStack>& gb,GBPlatformID platformId,GBEventID id, const SIPStackError& error,const RecordInfo& resp = RecordInfo()){}

	virtual void onOpenRealplayStreamReq(const weak_ptr<GBStack>& gb,GBPlatformID platformId,GBEventID streamId,const std::string& devid,int streamType,const StreamSDPInfo& sdp)
	{
		shared_ptr<GBStack> gbptr = gb.lock();
		if (gbptr != NULL)
		{
			gbptr->sendOpenStreamResp(platformId, streamId, SIPStackError_NoSuport);
		}
	}
	virtual void onOpenPlaybackStreamReq(const weak_ptr<GBStack>& gb,GBPlatformID platformId,GBEventID streamId,const std::string& devid,long long startTime,long long endTime,const StreamSDPInfo& sdp)
	{
		shared_ptr<GBStack> gbptr = gb.lock();
		if (gbptr != NULL)
		{
			gbptr->sendOpenStreamResp(platformId, streamId, SIPStackError_NoSuport);
		}
	}
	virtual void onOpenDownloadStreamReq(const weak_ptr<GBStack>& gb,GBPlatformID platformId,GBEventID streamId,const std::string& devid,long long startTime,long long endTime,int downSpeed,const StreamSDPInfo& sdp)
	{
		shared_ptr<GBStack> gbptr = gb.lock();
		if (gbptr != NULL)
		{
			gbptr->sendOpenStreamResp(platformId, streamId, SIPStackError_NoSuport);
		}
	}
	virtual void onOpenStreamResp(const weak_ptr<GBStack>& gb,GBPlatformID platformId,GBEventID streamId, const SIPStackError& error,const StreamSDPInfo& sdp){}
	virtual void onOpenStreamAck(const weak_ptr<GBStack>& gb,GBPlatformID platformId,GBEventID streamId,const std::string& sendip = "",int sendport = 0){}

	virtual void onCloseStreamReq(const weak_ptr<GBStack>& gb,GBPlatformID platformId,GBEventID streamId)
	{
		shared_ptr<GBStack> gbptr = gb.lock();
		if (gbptr != NULL)
		{
			gbptr->sendcloseStreamResp(platformId, streamId, SIPStackError_NoSuport);
		}
	}
	virtual void onCloseStreamResp(const weak_ptr<GBStack>& gb,GBPlatformID platformId,GBEventID streamId, const SIPStackError& error){}

	virtual void onPlaybackContrlReq(const weak_ptr<GBStack>& gb,GBPlatformID platformId,GBEventID id,GBEventID streamId,float speed,long long time)
	{
		shared_ptr<GBStack> gbptr = gb.lock();
		if (gbptr != NULL)
		{
			gbptr->sendPlaybackContrlResp(platformId, streamId, SIPStackError_NoSuport);
		}
	}
	virtual void onPlaybackContrlResp(const weak_ptr<GBStack>& gb,GBPlatformID platformId,GBEventID id, const SIPStackError& status,long rtpseq,long rtpTime){}

	virtual void onMediaStatus(const weak_ptr<GBStack>& gb,GBPlatformID platformId,GBEventID streamId,MediaSatus status){}
};

};
};

#endif //__GBSTACK_H__
