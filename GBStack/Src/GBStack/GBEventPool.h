#ifndef __GBEVENTPOOL_H__
#define __GBEVENTPOOL_H__
#include "Base/Timer.h"
#include "GBRegister.h"
#include "GBCallinfo.h"
#include "SIPStack/SIPSession.h"
#include "GBStack/GBStack.h"
using namespace Public::SIPStack;

namespace Public{
namespace GB28181{

class GBEventPool
{
public:
	enum MessageType
	{
		CATALOG_Req,
		DEVICEPTZ_Req,
		DEVICEREBOOT_Req,
		DEVICERECORD_Req,
		ALARMGUARD_Req,
		ALARMRESET_Req,
		ALARMNOTIFY_Req,
		ALARMSUBSCRIBE_Req,
		QUERYDEVICESTATUS_Req,
		QUERYDEVICEINFO_Req,
		QUERYRECORDINFO_Req,
		QUERYTHIRDINFO_Req,
		OPENTHIRD_Req,
		PLAYBACKCONTRL_Req,
	};
	enum CALLType
	{
		CALLType_Realplay,
		CALLType_Playback,
		CALLType_Download,
	};
private:
	struct recvMuniltResult
	{
		recvMuniltResult():sum(0),recvsize(0){}
		
		uint32_t 	sum;
		uint32_t	recvsize;
	};
	struct MessageInfo
	{
		uint64_t		startTime;
		MessageType		type;
		GBEventID		id;
		int				sn;
		std::string 	deviceID;
		bool			isRecved;
		SipDialog		dialog;
		recvMuniltResult	mresult;
	};
	struct timeoutEventInfo
	{
		GBPlatformID id;
		shared_ptr<MessageInfo> info;
	};
public:
	GBEventPool();
	~GBEventPool();
	
	void addPlatform(const weak_ptr<GBStack>& gbt28181Stack, const weak_ptr<GBDispacher>& dispacher,const weak_ptr<GBPlatformInfo>& platfrom);
	void deletePlatform(const weak_ptr<GBPlatformInfo>& platfrom);
	
	EVENTId_Error buildAndSendRequest(GBEventID& eventId,GBPlatformID platformId,const std::string& method,GBEventPool::MessageType type,int sn,const std::string& devId ,const std::string& msgXml,const std::string& contentType,bool end = true);
	EVENTId_Error getMessageInfo(GBPlatformID platformId,GBEventID eventId,int& messageSn,std::string& devId);
	EVENTId_Error getSubcribeInfo(GBPlatformID platformId, MessageType type, int& messageSn, std::string& devId);
	EVENTId_Error buildAndSendResponse(GBPlatformID platformId,GBEventID eventId,const std::string& method,const std::string& msgXml,const std::string& contentType,bool end = true);
	
	void rcvMessage(GBPlatformID platformId,shared_ptr<SIPSession>& session);

	EVENTId_Error buildAndSendOpenStreamReq(GBEventID& streamID,GBPlatformID platformId,const std::string& devid,long long startTime,long long endTime,CALLType type,int downSpeed,int streamType,const StreamSDPInfo& sdp);
	EVENTId_Error buildAndSendOpenStreamResp(GBPlatformID platformId,GBEventID streamID, const SIPStackError& error,const StreamSDPInfo& sdp);
	EVENTId_Error buildAndSendOpenStreamAck(GBPlatformID platformId,GBEventID streamID,const std::string& sendip = "",int sendport = 0);
	EVENTId_Error buildAndSendCloseStreamReq(GBPlatformID platformId,GBEventID streamID);
	EVENTId_Error buildAndSendCloseStreamResp(GBPlatformID platformId,GBEventID streamID);
	EVENTId_Error buildAndSendPlayCtrlReq(GBEventID& eventId,GBPlatformID platformId,GBEventID streamID,int sn,const std::string& msgXml,const std::string& contentType);
	EVENTId_Error buildAndSendPlayCtrlResp(GBPlatformID platformId,GBEventID eventId, const SIPStackError& error,const std::string& msgXml,const std::string& contentType);
	EVENTId_Error buildAndSendMediaStatus(GBPlatformID platformId,GBEventID streamID,int sn,const std::string& msgXml,const std::string& contentType);

	void recvInviteReq(GBPlatformID platformId,shared_ptr<SIPSession>& session);
	void recvInviteResp(GBPlatformID platformId,shared_ptr<SIPSession>& session);
	void recvInviteAck(GBPlatformID platformId,shared_ptr<SIPSession>& session);
	void recvInviteBye(GBPlatformID platformId,shared_ptr<SIPSession>& session);
	void recvUpdateReq(GBPlatformID platformId,shared_ptr<SIPSession>& session);
	void recvUpdateResp(GBPlatformID platformId,shared_ptr<SIPSession>& session);

	void procRespMessageError(shared_ptr<SIPSession>& session,GBPlatformID platformId);
	
	uint32_t getMessageSN(GBPlatformID platformId);

	std::string getDeviceId(GBPlatformID platformId,GBEventID streamID);
private:
	void procMessage(GBPlatformID platformId,GBEventID eventid,const SIPStackError& error);
	void procMessageProc(GBPlatformID platformId,shared_ptr<SIPSession>& session,EProtocolType type,void* param,int sn);
	void procRespMessage(shared_ptr<SIPSession>& session,EProtocolType type,void* param,GBPlatformID platformId,int sn);
	EVENTId_Error addNewMessage(GBEventID& eventId,GBPlatformID platformId,shared_ptr<SIPSession>& session,MessageType type,const std::string& devid,int sn,bool recved);
	
	void timerProc(unsigned long param);
private:
	
	struct UndoMsgInfo
	{
		GBEventID					eventid;
		shared_ptr<SIPSession>		session;
		std::string					body;
		std::string					type;
		bool						isend;
	};

	struct PlatformInfo
	{
		weak_ptr<GBStack>						stack;
		weak_ptr<GBDispacher>					dispacher;
		weak_ptr<GBPlatformInfo>				platfrom;
		std::map<MessageType,shared_ptr<MessageInfo> >		subcribeList;
		std::map<SipDialog,shared_ptr<MessageInfo> >		dialogMessageList;
		std::map<GBEventID,shared_ptr<MessageInfo> >		eventMessageList;
		std::map<int,shared_ptr<MessageInfo> >				snMessageList;
		std::map<SipDialog,shared_ptr<GBCallinfo> >			dialogCallList;
		std::map<GBEventID,shared_ptr<GBCallinfo> >			eventCallList;
		std::map<uint32_t,std::list<UndoMsgInfo> >			undoMsgList;
	};
	Mutex 												messageMutex;
	shared_ptr<Timer>									workTimer;
	std::map<GBPlatformID,shared_ptr<PlatformInfo> > 	platofrmList;
};

};
};
#endif //__GBEVENTPOOL_H__
