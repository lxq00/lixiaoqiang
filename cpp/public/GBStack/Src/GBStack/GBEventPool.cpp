#include "GBEventPool.h"
#include "GBStackInternal.h"

namespace Public{
namespace GB28181{
GBEventPool::GBEventPool()
{
	workTimer = make_shared<Timer>("GBEventPool");
	workTimer->start(Timer::Proc(&GBEventPool::timerProc,this),0,1000);
}
GBEventPool::~GBEventPool()
{
	workTimer = NULL;

	Guard locker(messageMutex);
	platofrmList.clear();
}

void GBEventPool::addPlatform(const weak_ptr<GBStack>& gbt28181Stack, const weak_ptr<GBDispacher>& dispacher, const weak_ptr<GBPlatformInfo>& platfrom)
{
	Guard locker(messageMutex);

	std::map<GBPlatformID, shared_ptr<PlatformInfo> >::iterator iter = platofrmList.find((GBPlatformID)platfrom.lock().get());
	if(iter != platofrmList.end())
	{
		return;
	}
	shared_ptr<PlatformInfo> info(new PlatformInfo);
	info->stack = gbt28181Stack;
	info->platfrom = platfrom;
	info->dispacher = dispacher;

	platofrmList[(GBPlatformID)platfrom.lock().get()] = info;
}
void GBEventPool::deletePlatform(const weak_ptr<GBPlatformInfo>& platfrom)
{
	Guard locker(messageMutex);

	platofrmList.erase((GBPlatformID)platfrom.lock().get());
}

EVENTId_Error GBEventPool::buildAndSendRequest(GBEventID& eventId,GBPlatformID platformId,const std::string& method,MessageType type,int sn,const std::string& devId ,const std::string& msgXml,const std::string& contentType,bool end)
{
	bool needSend = true;
	shared_ptr<SIPSession> session;
	shared_ptr<SIPSession> fromSession;
	shared_ptr<PlatformInfo> platinfo;
	{
		Guard locker(messageMutex);

		std::map<GBPlatformID, shared_ptr<PlatformInfo> >::iterator iter = platofrmList.find(platformId);
		if (iter == platofrmList.end())
		{
			return EVENTId_Error_NotRegist;
		}
		platinfo = iter->second;		
		if (strcasecmp(method.c_str(), "notify") == 0)
		{
			std::map<MessageType, shared_ptr<MessageInfo> >::iterator siter = iter->second->subcribeList.find(type);
			if (siter != iter->second->subcribeList.end())
			{
				fromSession = siter->second->dialog.getSession();
			}
		}
	}
		
	if (fromSession == NULL)
	{
		session = make_shared<SIPSession>(method, platinfo->platfrom.lock()->registe->getMyUrl(),
			platinfo->platfrom.lock()->registe->getToUrl(devId), platinfo->platfrom.lock()->transaction.get());
	}
	else
	{
		session = make_shared<SIPSession>(method, *fromSession.get(), platinfo->platfrom.lock()->registe->getMyUrl(),
			platinfo->platfrom.lock()->registe->getToUrl(devId));
	}
	
	//目录请求且为通知 需要排队
	if (type == CATALOG_Req && strcasecmp(method.c_str(), "notify") == 0)
	{
		Guard locker(messageMutex);

		std::map<uint32_t, std::list<UndoMsgInfo> >::iterator undoiter = platinfo->undoMsgList.find(type);
		if (undoiter == platinfo->undoMsgList.end())
		{
			platinfo->undoMsgList[type] = std::list<UndoMsgInfo>();
			undoiter = platinfo->undoMsgList.find(type);
		}
		if (undoiter->second.size() > 0)
		{
			needSend = false;
		}
		UndoMsgInfo info;
		info.session = session;
		info.body = msgXml;
		info.type = contentType;
		info.isend = end;

		undoiter->second.push_back(info);
	}

	EVENTId_Error id = addNewMessage(eventId,platformId,session,type,devId,sn,false);

	if(needSend)
	{
		bool ret =  session->sendRequest(msgXml,contentType);
		if(!ret)
		{
			return EVENTId_Error_SendError;
		}
	}

	return id;
}
EVENTId_Error GBEventPool::getMessageInfo(GBPlatformID platformId,GBEventID eventId,int& messageSn,std::string& devId)
{
	Guard locker(messageMutex);

	std::map<GBPlatformID, shared_ptr<PlatformInfo> >::iterator iter = platofrmList.find(platformId);
	if(iter == platofrmList.end())
	{
		return EVENTId_Error_SessionNotExist;
	}

	std::map<GBEventID,shared_ptr<MessageInfo> >::iterator eiter = iter->second->eventMessageList.find(eventId);
	if(eiter == iter->second->eventMessageList.end())
	{
		return EVENTId_Error_SessionNotExist;
	}

	messageSn = eiter->second->sn;
	devId = eiter->second->deviceID;

	return EVENTId_Error_NoError;
}
EVENTId_Error GBEventPool::getSubcribeInfo(GBPlatformID platformId, MessageType type, int& messageSn, std::string& devId)
{
	Guard locker(messageMutex);

	std::map<GBPlatformID, shared_ptr<PlatformInfo> >::iterator iter = platofrmList.find(platformId);
	if (iter == platofrmList.end())
	{
		return EVENTId_Error_SessionNotExist;
	}

	std::map<MessageType, shared_ptr<MessageInfo> >::iterator eiter = iter->second->subcribeList.find(type);
	if (eiter == iter->second->subcribeList.end())
	{
		return EVENTId_Error_SessionNotExist;
	}

	messageSn = eiter->second->sn;
	devId = eiter->second->deviceID;

	return EVENTId_Error_NoError;
}
EVENTId_Error GBEventPool::buildAndSendResponse(GBPlatformID platformId,GBEventID eventId,const std::string& method,const std::string& msgXml,const std::string& contentType,bool end)
{
	shared_ptr<SIPSession> newsession;
	shared_ptr<SIPSession> responsesession;
	
	{
		shared_ptr<PlatformInfo> platinfo;
		shared_ptr<MessageInfo> msginfo;
		{
			Guard locker(messageMutex);

			std::map<GBPlatformID, shared_ptr<PlatformInfo> >::iterator iter = platofrmList.find(platformId);
			if (iter == platofrmList.end())
			{
				return EVENTId_Error_SessionNotExist;
			}
			platinfo = iter->second;
			std::map<GBEventID, shared_ptr<MessageInfo> >::iterator eiter = iter->second->eventMessageList.find(eventId);
			if (eiter == iter->second->eventMessageList.end())
			{
				return EVENTId_Error_SessionNotExist;
			}
			msginfo = eiter->second;
			eiter->second->startTime = Time::getCurrentMilliSecond();

			if (method == "")
			{
				responsesession = eiter->second->dialog.getSession();
			}
		}
		if (method != "")
		{
			bool needSend = true;
			bool waitpool = false;

			newsession = make_shared<SIPSession>(method, platinfo->platfrom.lock()->registe->getMyUrl(),
				platinfo->platfrom.lock()->registe->getToUrl(platinfo->platfrom.lock()->clientId),
			//	platinfo->platfrom.lock()->registe->getToUrl(msginfo->deviceID == "" ? platinfo->platfrom.lock()->registe->getMyUrl().getUserName() : msginfo->deviceID),
				platinfo->platfrom.lock()->transaction.get(), msginfo->dialog.getSession().get());

		
			//目录请求回复 需要排队
			if(msginfo->type == CATALOG_Req || msginfo->type == QUERYRECORDINFO_Req)
			{
				Guard locker(messageMutex);

				std::map<uint32_t,std::list<UndoMsgInfo> >::iterator undoiter = platinfo->undoMsgList.find(msginfo->type);
				if(undoiter == platinfo->undoMsgList.end())
				{
					platinfo->undoMsgList[msginfo->type] = std::list<UndoMsgInfo>();
					undoiter = platinfo->undoMsgList.find(msginfo->type);
				}
				if(undoiter->second.size() > 0)
				{
					needSend = false;
				}
				UndoMsgInfo info;
				info.eventid = eventId;
				info.session = newsession;
				info.body = msgXml;
				info.type = contentType;
				info.isend = end;
				waitpool =  true;
				undoiter->second.push_back(info);

				shared_ptr<MessageInfo> sessioninfo(new MessageInfo);
				sessioninfo->deviceID = msginfo->deviceID;
				sessioninfo->dialog = newsession->getDialog();
				sessioninfo->sn = 0;
				sessioninfo->type = msginfo->type;

				platinfo->dialogMessageList[newsession->getDialog()] = sessioninfo;
			}

			if(!needSend)
			{
				newsession = NULL;
			}

			//需要等待的暂时不删除，等回复删除
			if (end && !waitpool)
			{
				Guard locker(messageMutex);

				//		iter->second.dialogMessageList.erase(eiter->second->dialog);
				//		iter->second.snMessageList.erase(eiter->second->sn);

				platinfo->eventMessageList.erase(eventId);
			}
		}
	}

	if (responsesession != NULL)
	{
		responsesession->sendSuccess(msgXml, contentType);
	}
	if (newsession != NULL)
	{
		newsession->sendRequest(msgXml, contentType);
	}

	return EVENTId_Error_NoError;
}

void GBEventPool::rcvMessage(GBPlatformID platformId,shared_ptr<SIPSession>& session)
{
	std::string body =session->getMessageBody();
	EProtocolType etype;
	if (GetMsgType(body,etype) != E_EC_OK)
	{
		etype = E_PT_VOD_CTRL_REQ;
	//	session->sendFailure(GBERROR(SIPStackError_ProcolError));
		//return  ;
	}

	//目录订阅也是使用的设备目录获取的命令
	if(etype == E_PT_DEVICE_CATALOG_QUERY_REQ && strcasecmp(session->getMethod().c_str(),"SUBSCRIBE") == 0)
	{
		etype = E_PT_CATALOG_SUBSCRIBE_REQ;
	}
	
	if(etype == E_PT_KEEP_ALIVE_NOTIFY)
	{
		//新增设备状态通知
		TKeepAliveNotify cmdsmg;
		if(ParseMsg(body,etype,&cmdsmg) != E_EC_OK)
		{
			session->sendFailure(SIPStackError(SIPStackError_ProcolError));
			return;
		}

		shared_ptr<GBPlatformInfo> platform;
		{
			Guard locker(messageMutex);
			std::map<GBPlatformID, shared_ptr<PlatformInfo> >::iterator iter = platofrmList.find(platformId);
			if(iter != platofrmList.end())
			{
				platform = iter->second->platfrom.lock();
			}
		}
		if(platform == NULL || platform->registe == NULL)
		{
			session->sendFailure(SIPStackError(SIPStackError_NotRegister));
		}
		else if(platform->registe != NULL && cmdsmg.strDeviceId == platform->clientId)
		{
			platform->registe->recvkeepalive(session);
		}
		else
		{			
			procMessageProc(platformId,session,etype,&cmdsmg,cmdsmg.nSn);
		}
	}
	else if(etype == E_PT_DEVICE_CONTROL_REQ)
	{
		TDeviceControlReq cmdsmg;
		if(ParseMsg(body,etype,&cmdsmg) != E_EC_OK)
		{
			session->sendFailure(SIPStackError(SIPStackError_ProcolError));
			return;
		}
		procMessageProc(platformId,session,etype,&cmdsmg,cmdsmg.nSn);
	}
	else if(etype == E_PT_DEVICE_CONTROL_RESP)
	{
		TDeviceControlResp cmdsmg;
		if(ParseMsg(body,etype,&cmdsmg) != E_EC_OK)
		{
			session->sendFailure(SIPStackError(SIPStackError_ProcolError));
			return;
		}
		procMessageProc(platformId,session,etype,&cmdsmg,cmdsmg.nSn);
	}
	else if(etype == E_PT_DEVICE_STATUS_QUERY_REQ)
	{
		TDeviceStatusQueryReq cmdsmg;
		if(ParseMsg(body,etype,&cmdsmg) != E_EC_OK)
		{
			session->sendFailure(SIPStackError(SIPStackError_ProcolError));
			return;
		}
		procMessageProc(platformId,session,etype,&cmdsmg,cmdsmg.nSn);
	}
	else if(etype == E_PT_DEVICE_STATUS_QUERY_RESP)
	{
		TDeviceStatusQueryResp cmdsmg;
		if(ParseMsg(body,etype,&cmdsmg) != E_EC_OK)
		{
			session->sendFailure(SIPStackError(SIPStackError_ProcolError));
			return;
		}
		procMessageProc(platformId,session,etype,&cmdsmg,cmdsmg.nSn);
	}
	else if(etype == E_PT_DEVICE_CATALOG_QUERY_REQ)
	{
		TDeviceCatalogQueryReq cmdsmg;
		if(ParseMsg(body,etype,&cmdsmg) != E_EC_OK)
		{
			session->sendFailure(SIPStackError(SIPStackError_ProcolError));
			return;
		}
		procMessageProc(platformId,session,etype,&cmdsmg,cmdsmg.nSn);
	}
	else if(etype == E_PT_DEVICE_CATALOG_QUERY_RESP)
	{
		TDeviceCatalogQueryResp cmdsmg;
		if(ParseMsg(body,etype,&cmdsmg) != E_EC_OK)
		{
			session->sendFailure(SIPStackError(SIPStackError_ProcolError));
			return;
		}
		procMessageProc(platformId,session,etype,&cmdsmg,cmdsmg.nSn);
	}
	else if(etype == E_PT_DEVICE_INFO_QUERY_REQ)
	{
		TDeviceInfoQueryReq cmdsmg;
		if(ParseMsg(body,etype,&cmdsmg) != E_EC_OK)
		{
			session->sendFailure(SIPStackError(SIPStackError_ProcolError));
			return;
		}
		procMessageProc(platformId,session,etype,&cmdsmg,cmdsmg.nSn);
	}
	else if(etype == E_PT_DEVICE_INFO_QUERY_RESP)
	{
		TDeviceInfoQueryResp cmdsmg;
		if(ParseMsg(body,etype,&cmdsmg) != E_EC_OK)
		{
			session->sendFailure(SIPStackError(SIPStackError_ProcolError));
			return;
		}
		procMessageProc(platformId,session,etype,&cmdsmg,cmdsmg.nSn);
	}
	else if(etype == E_PT_RECORD_INFO_QUERY_REQ)
	{
		TRecordInfoQueryReq cmdsmg;
		if(ParseMsg(body,etype,&cmdsmg) != E_EC_OK)
		{
			session->sendFailure(SIPStackError(SIPStackError_ProcolError));
			return;
		}
		procMessageProc(platformId,session,etype,&cmdsmg,cmdsmg.nSn);
	}
	else if(etype == E_PT_RECORD_INFO_QUERY_RESP)
	{
		TRecordInfoQueryResp cmdsmg;
		if(ParseMsg(body,etype,&cmdsmg) != E_EC_OK)
		{
			session->sendFailure(SIPStackError(SIPStackError_ProcolError));
			return;
		}
		procMessageProc(platformId,session,etype,&cmdsmg,cmdsmg.nSn);
	}
	else if(etype == E_PT_ALARM_NOTIFY_REQ)
	{
		TAlarmNotifyReq cmdsmg;
		if(ParseMsg(body,etype,&cmdsmg) != E_EC_OK)
		{
			session->sendFailure(SIPStackError(SIPStackError_ProcolError));
			return;
		}
		procMessageProc(platformId,session,etype,&cmdsmg,cmdsmg.nSn);
	}
	else if(etype == E_PT_ALARM_NOTIFY_RESP)
	{
		TAlarmNotifyResp cmdsmg;
		if(ParseMsg(body,etype,&cmdsmg) != E_EC_OK)
		{
			session->sendFailure(SIPStackError(SIPStackError_ProcolError));
			return;
		}
		procMessageProc(platformId,session,etype,&cmdsmg,cmdsmg.nSn);
	}
	else if(etype == E_PT_MEDIASTATUS_NOTIFY)
	{
		MediaStatusNotify cmdsmg;
		if(ParseMsg(body,etype,&cmdsmg) != E_EC_OK)
		{
			session->sendFailure(SIPStackError(SIPStackError_ProcolError));
			return;
		}
		procMessageProc(platformId,session,etype,&cmdsmg,cmdsmg.nSn);
	}
	else if(etype == E_PT_VOD_CTRL_REQ)
	{
		TVodCtrlReq cmdsmg;
		if(ParseMsg(body,etype,&cmdsmg) != E_EC_OK)
		{
			session->sendFailure(SIPStackError(SIPStackError_ProcolError));
			return;
		}
		procMessageProc(platformId,session,etype,&cmdsmg,cmdsmg.nSn);
	}
	else if(etype == E_PT_VOD_CTRL_RESP)
	{
		TVodCtrlResp cmdsmg;
		if(ParseMsg(body,etype,&cmdsmg) != E_EC_OK)
		{
			session->sendFailure(SIPStackError(SIPStackError_ProcolError));
			return;
		}
		procMessageProc(platformId,session,etype,&cmdsmg,cmdsmg.nSn);
	}
	else if(etype == E_PT_DECODER_CONTROL_REQ)
	{
		TDecoderControlReq cmdsmg;
		if(ParseMsg(body,etype,&cmdsmg) != E_EC_OK)
		{
			session->sendFailure(SIPStackError(SIPStackError_ProcolError));
			return;
		}
		procMessageProc(platformId,session,etype,&cmdsmg,cmdsmg.nSn);
	}
	else if(etype == E_PT_DECODER_CONTROL_RESP)
	{
		TDecoderControlResp cmdsmg;
		if(ParseMsg(body,etype,&cmdsmg) != E_EC_OK)
		{
			session->sendFailure(SIPStackError(SIPStackError_ProcolError));
			return;
		}
		procMessageProc(platformId,session,etype,&cmdsmg,cmdsmg.nSn);
	}
	else if(etype == E_PT_DECODER_INFO_QUERY_REQ)
	{
		TDecoderInfoQueryReq cmdsmg;
		if(ParseMsg(body,etype,&cmdsmg) != E_EC_OK)
		{
			session->sendFailure(SIPStackError(SIPStackError_ProcolError));
			return;
		}
		procMessageProc(platformId,session,etype,&cmdsmg,cmdsmg.nSn);
	}
	else if(etype == E_PT_DECODER_INFO_QUERY_RESP)
	{
		TDecoderInfoQueryResp cmdsmg;
		if(ParseMsg(body,etype,&cmdsmg) != E_EC_OK)
		{
			session->sendFailure(SIPStackError(SIPStackError_ProcolError));
			return;
		}
		procMessageProc(platformId,session,etype,&cmdsmg,cmdsmg.nSn);
	}
	else if(etype == E_PT_MEDIASTATUS_NOTIFY)
	{
		MediaStatusNotify cmdsmg;
		if(ParseMsg(body,etype,&cmdsmg) != E_EC_OK)
		{
			session->sendFailure(SIPStackError(SIPStackError_ProcolError));
			return;
		}
		procMessageProc(platformId,session,etype,&cmdsmg,cmdsmg.nSn);
	}
	else if(etype == E_PT_CATALOG_SUBSCRIBE_REQ)
	{
		TDeviceCatalogQueryReq cmdsmg;
		if(ParseMsg(body,E_PT_DEVICE_CATALOG_QUERY_REQ,&cmdsmg) != E_EC_OK)
		{
			session->sendFailure(SIPStackError(SIPStackError_ProcolError));
			return;
		}
		procMessageProc(platformId,session,etype,&cmdsmg,cmdsmg.nSn);
	}
	else if(etype == E_PT_ALARM_SUBSCRIBE_REQ)
	{
		TAlarmQueryReq cmdsmg;
		if(ParseMsg(body,etype,&cmdsmg) != E_EC_OK)
		{
			session->sendFailure(SIPStackError(SIPStackError_ProcolError));
			return;
		}
		procMessageProc(platformId,session,etype,&cmdsmg,cmdsmg.nSn);
	}
	else
	{
		session->sendFailure(SIPStackError(SIPStackError_NoSuport,"Not Suport This Cmd"));
		return;
	}
}
void GBEventPool::procMessageProc(GBPlatformID platformId,shared_ptr<SIPSession>& session,EProtocolType type,void* param,int sn)
{
	if(param == NULL)
	{
		session->sendFailure(SIPStackError(SIPStackError_ProcolError));
		return;
	}

	//云台控制和报警订阅和目录订阅和报警通知都需要在请求包做回复，故这不做消息回复
	if(type != E_PT_VOD_CTRL_REQ && type != E_PT_ALARM_SUBSCRIBE_REQ && type != E_PT_CATALOG_SUBSCRIBE_REQ && type != E_PT_ALARM_NOTIFY_REQ && type != E_PT_MEDIASTATUS_NOTIFY)
	{
		session->sendSuccess();
	}	

	GBEventID eventid = NULL;
	shared_ptr<GBDispacher> dispacher;
	weak_ptr<GBStack> stack;
	{
		Guard locker(messageMutex);
		std::map<GBPlatformID, shared_ptr<PlatformInfo> >::iterator iter = platofrmList.find(platformId);
		if(iter != platofrmList.end())
		{
			dispacher = iter->second->dispacher.lock();
			stack = iter->second->stack;
		}
	}
	if (dispacher == NULL)
	{
		session->sendFailure(SIPStackError(SIPStackError_NoSuport,"Not Find The Registe Info"));
		return;
	}
	
	switch(type)
	{
	case E_PT_KEEP_ALIVE_NOTIFY:
		{
			TKeepAliveNotify* req = (TKeepAliveNotify*)param;
			dispacher->onDeviceStatusNotify(stack,platformId,req->strDeviceId,req->bStatus);

			break;
		}
	case E_PT_CATALOG_SUBSCRIBE_REQ:
		{
			TDeviceCatalogQueryReq* req = (TDeviceCatalogQueryReq*)param;
			Guard locker(messageMutex);
			std::map<GBPlatformID, shared_ptr<PlatformInfo> >::iterator iter = platofrmList.find(platformId);
			if(iter != platofrmList.end())
			{
				shared_ptr<MessageInfo> info(new MessageInfo());
				info->deviceID = req->strDeviceId;
				info->dialog = session->getDialog();
				info->sn = req->nSn;
				info->startTime = Time::getCurrentMilliSecond();
				info->type = CATALOG_Req;
				iter->second->subcribeList[CATALOG_Req] = info;
			}
		}
	case E_PT_DEVICE_CATALOG_QUERY_REQ:
		{
			TDeviceCatalogQueryReq* req = (TDeviceCatalogQueryReq*)param;
			addNewMessage(eventid,platformId,session,CATALOG_Req,req->strDeviceId,req->nSn,true);

			if(req->bSubcribe || type == E_PT_CATALOG_SUBSCRIBE_REQ)
			{
				dispacher->onCatalogSubscribeReq(stack,platformId,eventid,req->strDeviceId,SipTimeToSec(req->strStartTime),SipTimeToSec(req->strEndTime));
			}
			else
			{
				dispacher->onGetCatalogReq(stack,platformId,eventid);
			}
			break;
		}
	case E_PT_DEVICE_CONTROL_REQ:
		{
			TDeviceControlReq* req = (TDeviceControlReq*)param;
			if(req->bTeleBoot)
			{
				addNewMessage(eventid,platformId,session,DEVICEREBOOT_Req,req->strDeviceId,req->nSn,true);
			
				dispacher->onDeviceRebootReq(stack,platformId,eventid,req->strDeviceId);
			}
			else if(req->eRecordCmd != E_RC_NONE)
			{
				addNewMessage(eventid,platformId,session,DEVICERECORD_Req,req->strDeviceId,req->nSn,true);
				dispacher->onDeviceRecordReq(stack,platformId,eventid,req->strDeviceId,req->eRecordCmd == E_RC_RECORD ? true : false);
			}
			else if(req->eGuardCmd != E_GC_NONE)
			{
				addNewMessage(eventid,platformId,session,ALARMGUARD_Req,req->strDeviceId,req->nSn,true);
				dispacher->onAlarmGuardReq(stack,platformId,eventid,req->strDeviceId,req->eGuardCmd == E_GC_SET_GUARD ? true : false);
			}
			else if(req->bAlarmCmd)
			{
				addNewMessage(eventid,platformId,session,ALARMRESET_Req,req->strDeviceId,req->nSn,true);
				dispacher->onAlarmResetReq(stack,platformId,eventid,req->strDeviceId);
			}
			else
			{
				addNewMessage(eventid,platformId,session,DEVICEPTZ_Req,req->strDeviceId,req->nSn,true);
				dispacher->onDevicePtzReq(stack,platformId,eventid,req->strDeviceId,(GBPtzDirection)req->ptz,req->ptzParam);
			}
			
			break;
		}
	case E_PT_DEVICE_INFO_QUERY_REQ:
		{
			TDeviceInfoQueryReq* req = (TDeviceInfoQueryReq*)param;
			addNewMessage(eventid,platformId,session,QUERYDEVICEINFO_Req,req->strDeviceId,req->nSn,true);
			
			dispacher->onQueryDeviceInfoReq(stack,platformId,eventid,req->strDeviceId);
			break;
		}
	case E_PT_DEVICE_STATUS_QUERY_REQ:
		{
			TDeviceStatusQueryReq* req = (TDeviceStatusQueryReq*)param;
			addNewMessage(eventid,platformId,session,QUERYDEVICESTATUS_Req,req->strDeviceId,req->nSn,true);
			
			dispacher->onQueryDeviceStatusReq(stack,platformId,eventid,req->strDeviceId);
			break;
		}
	case E_PT_RECORD_INFO_QUERY_REQ:
		{
			TRecordInfoQueryReq* req = (TRecordInfoQueryReq*)param;
			addNewMessage(eventid,platformId,session,QUERYRECORDINFO_Req,req->strDeviceId,req->nSn,true);
			
			dispacher->onQueryRecordInfoReq(stack,platformId,eventid,req->strDeviceId,SipTimeToSec(req->strStartTime),SipTimeToSec(req->strEndTime),req->eType);
			break;
		}
	case E_PT_ALARM_SUBSCRIBE_REQ:
		{
			TAlarmQueryReq* req = (TAlarmQueryReq*)param;
			{
				Guard locker(messageMutex);
				std::map<GBPlatformID, shared_ptr<PlatformInfo> >::iterator iter = platofrmList.find(platformId);
				if(iter != platofrmList.end())
				{
					shared_ptr<MessageInfo> info(new MessageInfo());
					info->deviceID = req->strDeviceId;
					info->dialog = session->getDialog();
					info->sn = req->nSn;
					info->startTime = Time::getCurrentMilliSecond();
					info->type = ALARMSUBSCRIBE_Req;

					iter->second->subcribeList[ALARMSUBSCRIBE_Req] = info;
				}
			}
			
			addNewMessage(eventid,platformId,session,ALARMSUBSCRIBE_Req,req->strDeviceId,req->nSn,true);
			AlarmSubcribe subcribe;
			subcribe.StartAlarmPriority = req->nStartAlarmPriority;
			subcribe.EndAlarmPriority = req->nEndAlarmPriority;
			subcribe.AlarmMethod = req->nAlarmMethod;
			subcribe.StartTime = SipTimeToSec(req->strStartTime);
			subcribe.EndTime = SipTimeToSec(req->strEndTime);

			dispacher->onAlarmSubscribeReq(stack,platformId,eventid,req->strDeviceId,subcribe);
			break;
		}
	case E_PT_ALARM_NOTIFY_REQ:
		{
			TAlarmNotifyReq* req = (TAlarmNotifyReq*)param;
			addNewMessage(eventid,platformId,session,ALARMNOTIFY_Req,req->strDeviceId,req->nSn,true);
			AlarmNotifyInfo info;
			info.Priority = req->nAlarmPriority;
			info.Method = req->nAlarmMethod;
			info.Time = SipTimeToSec(req->strAlarmTime);
			info.Description = req->strAlarmDescription;
			info.ExtendInfo = req->vExtendInfo;
				
			dispacher->onAlarmNotifyReq(stack,platformId,eventid,req->strDeviceId,info);
			break;
		}
	case E_PT_VOD_CTRL_REQ:
		{
			TVodCtrlReq* req = (TVodCtrlReq*)param;
			GBEventID callEventId = NULL;
			std::string deviceID;
			{
				SipDialog dialog = session->getDialog();
				Guard locker(messageMutex);
				std::map<GBPlatformID, shared_ptr<PlatformInfo> >::iterator iter = platofrmList.find(platformId);
				if(iter != platofrmList.end())
				{
					std::map<SipDialog, shared_ptr<GBCallinfo> >::iterator citer = iter->second->dialogCallList.find(dialog);
					if (citer != iter->second->dialogCallList.end())
					{
						callEventId = citer->second.get();
						deviceID = citer->second->getDeviceUrl();
					}
				}
			}
			if (deviceID == "")
			{
				session->sendFailure(SIPStackError(SIPStackError_NoSuport,"DeviceID Error"));
				return;
			}
			
			addNewMessage(eventid,platformId,session,PLAYBACKCONTRL_Req,deviceID,req->nSn,true);
			float speed = (float)req->dSpeed;
			if(req->eType == E_RT_PAUSE || req->eType == E_RT_TEARDOWN)
			{
				speed = 0;
			}
			dispacher->onPlaybackContrlReq(stack,platformId,eventid,callEventId,speed,req->lStartTime);
			
			break;
		}		
	case E_PT_MEDIASTATUS_NOTIFY:
		{
			MediaStatusNotify* req = (MediaStatusNotify*)param;
			GBEventID callEventId = NULL;
			{
				SipDialog dialog = session->getDialog();
				Guard locker(messageMutex);
				std::map<GBPlatformID, shared_ptr<PlatformInfo> >::iterator iter = platofrmList.find(platformId);
				if(iter != platofrmList.end())
				{
					std::map<SipDialog, shared_ptr<GBCallinfo> >::iterator citer = iter->second->dialogCallList.find(dialog);
					if (citer != iter->second->dialogCallList.end())
					{
						callEventId = citer->second.get();
					}
				}
			}
			if (callEventId == NULL)
			{
				session->sendFailure(SIPStackError(SIPStackError_NoSuport,"Not Find The CallID"));
				return;
			}
			session->sendSuccess();

			dispacher->onMediaStatus(stack,platformId,callEventId,(MediaSatus)req->nType);
			break;
		}
	case E_PT_DEVICE_STATUS_QUERY_RESP:
	case E_PT_DEVICE_CONTROL_RESP:
	case E_PT_DEVICE_CATALOG_QUERY_RESP:
	case E_PT_DEVICE_INFO_QUERY_RESP:
	case E_PT_RECORD_INFO_QUERY_RESP:
	case E_PT_ALARM_NOTIFY_RESP:
	case E_PT_DECODER_INFO_QUERY_RESP:
	case E_PT_DECODER_CONTROL_RESP:
	case E_PT_VOD_CTRL_RESP:
		return procRespMessage(session,type,param,platformId,sn);
		break;
	default:
		break;
	}		
}

void GBEventPool::procRespMessage(shared_ptr<SIPSession>& session,EProtocolType type,void* param,GBPlatformID platformId,int sn)
{
	SipDialog dialog = session->getDialog();

	shared_ptr<GBDispacher> dispacher;
	weak_ptr<GBStack> stack;
	int msgtype = 0;
	GBEventID	eventid = NULL;
	{
		Guard locker(messageMutex);
		std::map<GBPlatformID, shared_ptr<PlatformInfo> >::iterator iter = platofrmList.find(platformId);
		if(iter != platofrmList.end())
		{
			std::map<int, shared_ptr<MessageInfo> >::iterator eiter = iter->second->snMessageList.find(sn);
			if (eiter != iter->second->snMessageList.end())
			{
				msgtype = eiter->second->type;
				eventid = eiter->second->id;
				eiter->second->startTime = Time::getCurrentMilliSecond();

				if (msgtype != CATALOG_Req && msgtype != QUERYRECORDINFO_Req)
				{
					iter->second->dialogMessageList.erase(eiter->second->dialog);
					iter->second->eventMessageList.erase(eiter->second->id);

					iter->second->snMessageList.erase(eiter);
				}				
			}
			dispacher = iter->second->dispacher.lock();
			stack = iter->second->stack;
		}
	}
	if (dispacher == NULL)
	{
		session->sendFailure(SIPStackError(SIPStackError_NoSuport,"Not Find The Registe Info"));
		return;
	}

	switch(msgtype)
	{
	case CATALOG_Req:
		{
			TDeviceCatalogQueryResp* resp = (TDeviceCatalogQueryResp*)param;

			CataLogInfo catalogInfo;
			bool isFind = false;
			{
				catalogInfo.SumNum = resp->nSumNum;
				catalogInfo.EndOfFile = false;
				for (unsigned int i = 0; i < resp->vDeviceItem.size(); i++)
				{
					catalogInfo.resp.push_back(resp->vDeviceItem[i]);
				}
			}

			{
				Guard locker(messageMutex);
				std::map<GBPlatformID, shared_ptr<PlatformInfo> >::iterator iter = platofrmList.find(platformId);
				if(iter != platofrmList.end())
				{
					std::map<int, shared_ptr<MessageInfo> >::iterator eiter = iter->second->snMessageList.find(sn);
					if (eiter != iter->second->snMessageList.end())
					{
						eiter->second->mresult.sum = resp->nSumNum;
						eiter->second->mresult.recvsize += catalogInfo.resp.size();

						eiter->second->startTime = Time::getCurrentMilliSecond();

						if (eiter->second->mresult.sum == eiter->second->mresult.recvsize || resp->bEndofFile)
						{
							catalogInfo.EndOfFile = true;

							iter->second->dialogMessageList.erase(eiter->second->dialog);
							iter->second->eventMessageList.erase(eiter->second->id);

							iter->second->snMessageList.erase(eiter);
						}

						isFind = true;
					}
				}
			}
			//这种情况为目录通知
			if (eventid == NULL)
			{
				dispacher->onCatalogNotify(stack, platformId, eventid, catalogInfo);

				return;
			}
			else if (isFind)
			{
				dispacher->onGetCatalogResp(stack, platformId, eventid, SIPStackError_OK, catalogInfo);
			}
			else if (!isFind)
			{
				session->sendFailure(SIPStackError(SIPStackError_NoSuport));
			}
			break;
		}
	case DEVICEPTZ_Req:
		{
			TDeviceControlResp* resp = (TDeviceControlResp*)param;

			dispacher->onDeviceContrlResp(stack,platformId,eventid,resp->bResult?SIPStackError_OK:SIPStackError_ServerError);
			break;
		}
	case DEVICEREBOOT_Req:
		{
			TDeviceControlResp* resp = (TDeviceControlResp*)param;

			dispacher->onDeviceContrlResp(stack,platformId,eventid,resp->bResult?SIPStackError_OK:SIPStackError_ServerError);
			break;
		}
	case DEVICERECORD_Req:
		{
			TDeviceControlResp* resp = (TDeviceControlResp*)param;
			
			dispacher->onDeviceContrlResp(stack,platformId,eventid,resp->bResult?SIPStackError_OK:SIPStackError_ServerError);
			break;
		}
	case ALARMSUBSCRIBE_Req:
		{
			TAlarmNotifyResp* resp = (TAlarmNotifyResp*)param;

			dispacher->onAlarmSubscribeResp(platformId,eventid,resp->bResult?SIPStackError_OK:SIPStackError_ServerError);
			break;
		}
	case ALARMGUARD_Req:
		{
			TDeviceControlResp* resp = (TDeviceControlResp*)param;

			dispacher->onDeviceContrlResp(stack,platformId,eventid,resp->bResult?SIPStackError_OK:SIPStackError_ServerError);
			break;
		}
	case ALARMRESET_Req:
		{
			TDeviceControlResp* resp = (TDeviceControlResp*)param;

			dispacher->onDeviceContrlResp(stack,platformId,eventid,resp->bResult?SIPStackError_OK:SIPStackError_ServerError);
			break;
		}
	case ALARMNOTIFY_Req:
		{
			TAlarmNotifyResp* resp = (TAlarmNotifyResp*)param;

			dispacher->onAlarmNotifyResp(stack,platformId,eventid,resp->bResult?SIPStackError_OK:SIPStackError_ServerError);
			break;
		}
	case QUERYDEVICESTATUS_Req:
		{
			TDeviceStatusQueryResp* resp = (TDeviceStatusQueryResp*)param;
			GBDeviceStatus respstatus;
			respstatus.onLine = (bool)resp->bOnline;
			respstatus.encodeStatus = (bool)resp->bEncode;
			respstatus.recordStatus = (bool)resp->bRecord;
			respstatus.workStatus = (bool)resp->bStatus;
			respstatus.deviceTime = resp->strDeviceTime;
			for(unsigned int i  = 0;i < resp->vTAlarmStatus.size();i ++)
			{
				respstatus.alarmStatus.push_back(resp->vTAlarmStatus[i]);
			}
			dispacher->onQueryDeviceStatusResp(stack,platformId,eventid,resp->bResult ? SIPStackError_OK:SIPStackError_ServerError,respstatus);
			break;
		}
	case QUERYDEVICEINFO_Req:
		{
			TDeviceInfoQueryResp* resp = (TDeviceInfoQueryResp*)param;
			GBDeviceInfo devinfo;
			devinfo.manufacturer = resp->strManufacturer;
			devinfo.model = resp->strModel;
			devinfo.firmWare = resp->strFirmware;
			devinfo.cameraNum = resp->nMaxCamera;
			devinfo.alarmNum = resp->nMaxAlarm;

			dispacher->onQueryDeviceInfoResp(stack,platformId,eventid,resp->bResult?SIPStackError_OK:SIPStackError_ServerError,devinfo);
			break;
		}
	case QUERYRECORDINFO_Req:
		{
			TRecordInfoQueryResp* resp = (TRecordInfoQueryResp*)param;

			RecordInfo recordInfo;
			bool isFind = false;
			{
				Guard locker(messageMutex);
				std::map<GBPlatformID, shared_ptr<PlatformInfo> >::iterator iter = platofrmList.find(platformId);
				if(iter != platofrmList.end())
				{
					std::map<int, shared_ptr<MessageInfo> >::iterator eiter = iter->second->snMessageList.find(sn);
					if (eiter != iter->second->snMessageList.end())
					{
						eiter->second->mresult.sum = resp->nSumNum;

						recordInfo.SumNum = resp->nSumNum;
						recordInfo.EndOfFile = resp->bEndofFile;

						for (unsigned int i = 0; i < resp->vRecordList.size(); i++)
						{
							recordInfo.resp.push_back(resp->vRecordList[i]);
						}

						eiter->second->mresult.recvsize += recordInfo.resp.size();

						eiter->second->startTime = Time::getCurrentMilliSecond();

						if (eiter->second->mresult.sum == eiter->second->mresult.recvsize || recordInfo.EndOfFile)
						{
							recordInfo.EndOfFile = true;

							iter->second->dialogMessageList.erase(eiter->second->dialog);
							iter->second->eventMessageList.erase(eiter->second->id);

							iter->second->snMessageList.erase(eiter);
						}

						isFind = true;
					}
				}
			}
			if (!isFind)
			{
				session->sendFailure(SIPStackError(SIPStackError_NoSuport));
			}
			else
			{
				dispacher->onQueryRecordInfoResp(stack, platformId, eventid, SIPStackError_OK, recordInfo);
			}			
			break;
		}
	case PLAYBACKCONTRL_Req:
		{
			TVodCtrlResp* resp = (TVodCtrlResp*)param;
			dispacher->onPlaybackContrlResp(stack,platformId,eventid,resp->nResult?SIPStackError_OK:SIPStackError_ServerError,resp->rtpinfo_seq,resp->rtpinfo_time);
			break;
		}
	default:
		break;
	}
}

void GBEventPool::procRespMessageError(shared_ptr<SIPSession>& session,GBPlatformID platformId)
{
	SipDialog dialog = session->getDialog();
	SIPStackError error = session->getError();

	int msgtype = 0;
	GBEventID	eventid = NULL;
	shared_ptr<GBRegister>	registe;
	shared_ptr<GBDispacher> dispacher;
	weak_ptr<GBStack> stack;
	bool isFind = false;
	{
		Guard locker(messageMutex);
		std::map<GBPlatformID, shared_ptr<PlatformInfo> >::iterator iter = platofrmList.find(platformId);
		if(iter == platofrmList.end())
		{
			return;
		}
		dispacher = iter->second->dispacher.lock();
		registe = iter->second->platfrom.lock()->registe;
		stack = iter->second->stack;

		std::map<SipDialog,shared_ptr<MessageInfo> >::iterator eiter = iter->second->dialogMessageList.find(dialog);
		if(eiter != iter->second->dialogMessageList.end())
		{
			msgtype = eiter->second->type;
			eventid = eiter->second->id;
			isFind = true;
		}
	}
	if (!isFind && strcasecmp(session->getMethod().c_str(), "REGISTER") == 0 && registe != NULL)
	{
		if (error.getErrorCode() == SIPStackError_OK)
		{
			registe->recv2xx(session);
		}
		else
		{
			registe->recv3456xx(session);
		}

		return;
	}
	do{		
		UndoMsgInfo session;
		{
			Guard locker(messageMutex);
			std::map<GBPlatformID, shared_ptr<PlatformInfo> >::iterator iter = platofrmList.find(platformId);
			if(iter == platofrmList.end())
			{
				break;
			}
			std::map<uint32_t,std::list<UndoMsgInfo> >::iterator undoiter = iter->second->undoMsgList.find(msgtype);
			if(undoiter != iter->second->undoMsgList.end() && undoiter->second.size() > 0)
			{
				undoiter->second.pop_front();

				if(undoiter->second.size() > 0)
				{
					session = undoiter->second.front();
				}
				else
				{
					std::map<SipDialog, shared_ptr<MessageInfo> >::iterator eiter = iter->second->dialogMessageList.find(dialog);
					if (eiter != iter->second->dialogMessageList.end())
					{
						iter->second->eventMessageList.erase(eiter->second.get());
						iter->second->snMessageList.erase(eiter->second->sn);
						iter->second->dialogMessageList.erase(eiter);
					}
				}
			}
			{
				std::map<GBEventID,shared_ptr<MessageInfo> >::iterator eiter = iter->second->eventMessageList.find(session.eventid);
				if(eiter != iter->second->eventMessageList.end())
				{
					eiter->second->startTime = Time::getCurrentMilliSecond();
				}
			}				
		}
		if(session.session != NULL)
		{
			session.session->sendRequest(session.body,session.type);
		}

	}while(0);

	if(dispacher == NULL || !isFind)
	{
		return;
	}
	if(msgtype == PLAYBACKCONTRL_Req)
	{
		int rtpseq = 0;
		long rtptime = 0;
		{
			std::string body =session->getMessageBody();
			TVodCtrlResp cmdsmg;
			if(ParseMsg(body,E_PT_VOD_CTRL_RESP,&cmdsmg) == E_EC_OK)
			{
				rtpseq = cmdsmg.rtpinfo_seq;
				rtptime = cmdsmg.rtpinfo_time;
			}
		}

		dispacher->onPlaybackContrlResp(stack,platformId,eventid,error,rtpseq,rtptime);
		{
			Guard locker(messageMutex);
			std::map<GBPlatformID, shared_ptr<PlatformInfo> >::iterator iter = platofrmList.find(platformId);
			if(iter == platofrmList.end())
			{
				return;
			}
			std::map<GBEventID,shared_ptr<MessageInfo> >::iterator eiter = iter->second->eventMessageList.find(eventid);
			if(eiter == iter->second->eventMessageList.end())
			{
				return;
			}

			iter->second->snMessageList.erase(eiter->second->sn);
			iter->second->dialogMessageList.erase(eiter->second->dialog);

			iter->second->eventMessageList.erase(eiter);
		}
		return;
	}	

	procMessage(platformId,eventid,error);
}

EVENTId_Error GBEventPool::addNewMessage(GBEventID& eventId,GBPlatformID platformId,shared_ptr<SIPSession>& session,MessageType type,const std::string& devid,int sn,bool recved)
{
	Guard locker(messageMutex);

	SipDialog dialog = session->getDialog();

	shared_ptr<MessageInfo> info(new MessageInfo);
	info->id = info.get();
	info->sn = sn;
	info->deviceID = devid;
	info->type = type;
	info->isRecved = recved;
	info->startTime = Time::getCurrentMilliSecond();
	info->dialog = dialog;

	std::map<GBPlatformID, shared_ptr<PlatformInfo> >::iterator iter = platofrmList.find(platformId);
	if(iter == platofrmList.end())
	{
		return EVENTId_Error_SessionNotExist;
	}

	iter->second->dialogMessageList.insert(pair<SipDialog,shared_ptr<MessageInfo> >(dialog,info));
	iter->second->eventMessageList.insert(pair<GBEventID,shared_ptr<MessageInfo> >(info.get(),info));
	iter->second->snMessageList.insert(pair<int,shared_ptr<MessageInfo> >(sn,info));

	eventId = info.get();

	return EVENTId_Error_NoError;
}
void GBEventPool::procMessage(GBPlatformID platformId, GBEventID eventid, const SIPStackError& error)
{
	shared_ptr<GBDispacher> dispacher;
	weak_ptr<GBStack> stack;
	MessageType msgtype;
	bool messageIsRecv = false;
	SipDialog dialog;
	{
		Guard locker(messageMutex);
		std::map<GBPlatformID, shared_ptr<PlatformInfo> >::iterator iter = platofrmList.find(platformId);
		if (iter == platofrmList.end())
		{
			return;
		}
		std::map<GBEventID, shared_ptr<MessageInfo> >::iterator eiter = iter->second->eventMessageList.find(eventid);
		if (eiter == iter->second->eventMessageList.end())
		{
			return;
		}

		dispacher = iter->second->dispacher.lock();
		stack = iter->second->stack;
		msgtype = eiter->second->type;
		messageIsRecv = eiter->second->isRecved;
		dialog = eiter->second->dialog;
	}
	if (dispacher == NULL)
	{
		return;
	}
		
	bool doMessageResp = true;
	if(error.getErrorCode() != SIPStackError_OK && messageIsRecv)
	{	
		if(dialog.getSession() != NULL)
		{
			SIPSession session(*dialog.getSession().get());
			session.sendFailure(error);
		}		
	}
	else
	{
		switch(msgtype)
		{
		case CATALOG_Req:
			{
				if(error.getErrorCode() == SIPStackError_OK)
				{
					doMessageResp = false;
					return;
				}
				dispacher->onGetCatalogResp(stack,platformId,eventid, error);
				break;
			}
		case DEVICEPTZ_Req:
		case DEVICEREBOOT_Req:
			{
				dispacher->onDeviceContrlResp(stack,platformId,eventid, error);
				break;
			}
		case DEVICERECORD_Req:
		case ALARMGUARD_Req:
		case ALARMRESET_Req:
			{
				if(error.getErrorCode() == SIPStackError_OK)
				{
					doMessageResp = false;
					return;
				}
				dispacher->onDeviceContrlResp(stack,platformId,eventid, error);
				break;
			}
		case ALARMNOTIFY_Req:
			{
				if(error.getErrorCode() == SIPStackError_OK)
				{
					doMessageResp = false;
					return;
				}
				dispacher->onAlarmNotifyResp(stack,platformId,eventid,error);
				break;
			}
		case QUERYDEVICESTATUS_Req:
			{
				if(error.getErrorCode() == SIPStackError_OK)
				{
					doMessageResp = false;
					return;
				}
				dispacher->onQueryDeviceStatusResp(stack,platformId,eventid,error);
				break;
			}
		case QUERYDEVICEINFO_Req:
			{
				if(error.getErrorCode() == SIPStackError_OK)
				{
					doMessageResp = false;
					return;
				}
				dispacher->onQueryDeviceInfoResp(stack,platformId,eventid,error);
				break;
			}
		case QUERYRECORDINFO_Req:
			{
				if(error.getErrorCode() == SIPStackError_OK)
				{
					doMessageResp = false;
					return;
				}
				dispacher->onQueryRecordInfoResp(stack,platformId,eventid,error);
				break;
			}
		case PLAYBACKCONTRL_Req:
			{
				dispacher->onPlaybackContrlResp(stack,platformId,eventid,error,0,0);
				break;
			}
		default:
			{
				doMessageResp = false;
				break;
			}
		}
	}

	if(doMessageResp)
	{
		Guard locker(messageMutex);
		std::map<GBPlatformID, shared_ptr<PlatformInfo> >::iterator iter = platofrmList.find(platformId);
		if(iter == platofrmList.end())
		{
			return;
		}
		std::map<GBEventID,shared_ptr<MessageInfo> >::iterator eiter = iter->second->eventMessageList.find(eventid);
		if(eiter == iter->second->eventMessageList.end())
		{
			return;
		}

		iter->second->snMessageList.erase(eiter->second->sn);
		iter->second->dialogMessageList.erase(eiter->second->dialog);

		iter->second->eventMessageList.erase(eiter);	
	}
}

EVENTId_Error GBEventPool::buildAndSendOpenStreamReq(GBEventID& streamID,GBPlatformID platformId,const std::string& devid,long long startTime,long long endTime,CALLType type,int downSpeed,int streamType,const StreamSDPInfo& sdp)
{
	Guard locker(messageMutex);
	std::map<GBPlatformID, shared_ptr<PlatformInfo> >::iterator iter = platofrmList.find(platformId);
	if(iter == platofrmList.end())
	{
		return EVENTId_Error_SessionNotExist;
	}

	StreamSessionSDP ssdp = GBCallinfo::buildSessionSDP(devid,startTime,endTime,type,downSpeed,streamType,sdp);
	shared_ptr<GBCallinfo> call(new GBCallinfo(iter->second->platfrom.lock()->registe,shared_ptr<SIPSession>(),ssdp));
	streamID = call.get();

	SipDialog dialog = call->getDialog();
	iter->second->dialogCallList.insert(pair<SipDialog,shared_ptr<GBCallinfo> >(dialog,call));
	iter->second->eventCallList.insert(pair<GBEventID,shared_ptr<GBCallinfo> >(streamID,call));

	return EVENTId_Error_NoError;
}
EVENTId_Error GBEventPool::buildAndSendOpenStreamResp(GBPlatformID platformId,GBEventID streamID,const SIPStackError& error,const StreamSDPInfo& sdp)
{
	shared_ptr<GBCallinfo> callinfo;

	{
		Guard locker(messageMutex);
		std::map<GBPlatformID, shared_ptr<PlatformInfo> >::iterator iter = platofrmList.find(platformId);
		if (iter == platofrmList.end())
		{
			return EVENTId_Error_SessionNotExist;
		}

		std::map<GBEventID, shared_ptr<GBCallinfo> >::iterator siter = iter->second->eventCallList.find(streamID);
		if (siter == iter->second->eventCallList.end())
		{
			return EVENTId_Error_SessionNotExist;
		}
		callinfo = siter->second;
	}
	if (callinfo != NULL)
	{
		callinfo->sendResp(streamID, error, sdp);
	}	

	return EVENTId_Error_NoError;
}
EVENTId_Error GBEventPool::buildAndSendOpenStreamAck(GBPlatformID platformId,GBEventID streamID,const std::string& sendip,int sendport)
{
	shared_ptr<GBCallinfo> callinfo;
	{
		Guard locker(messageMutex);
		std::map<GBPlatformID, shared_ptr<PlatformInfo> >::iterator iter = platofrmList.find(platformId);
		if (iter == platofrmList.end())
		{
			return EVENTId_Error_SessionNotExist;
		}

		std::map<GBEventID, shared_ptr<GBCallinfo> >::iterator siter = iter->second->eventCallList.find(streamID);
		if (siter == iter->second->eventCallList.end())
		{
			return EVENTId_Error_SessionNotExist;
		}
		callinfo = siter->second;
	}
	if (callinfo != NULL)
	{
		callinfo->sendACK(streamID, sendip, sendport);
	}	

	return EVENTId_Error_NoError;
}
EVENTId_Error GBEventPool::buildAndSendCloseStreamReq(GBPlatformID platformId,GBEventID streamID)
{
	Guard locker(messageMutex);
	std::map<GBPlatformID, shared_ptr<PlatformInfo> >::iterator iter = platofrmList.find(platformId);
	if(iter == platofrmList.end())
	{
		return EVENTId_Error_SessionNotExist;
	}

	std::map<GBEventID,shared_ptr<GBCallinfo> >::iterator siter = iter->second->eventCallList.find(streamID);
	if(siter == iter->second->eventCallList.end())
	{
		return EVENTId_Error_SessionNotExist;
	}
	
	// modify by zhongwu  PlatformInfo::dialogCallList 与PlatformInfo::eventCallList 共享同一份内存，删掉其中一个之后
	// 应该将其另一个也移除列表，以防止内存崩溃
	std::map<SipDialog, shared_ptr<GBCallinfo> >::iterator citer = iter->second->dialogCallList.find(siter->second->getDialog());
	if (citer != iter->second->dialogCallList.end())
	{
		iter->second->dialogCallList.erase(citer);
	}
	iter->second->eventCallList.erase(siter);

	return EVENTId_Error_NoError;
}
EVENTId_Error GBEventPool::buildAndSendCloseStreamResp(GBPlatformID platformId,GBEventID streamID)
{
	Guard locker(messageMutex);
	std::map<GBPlatformID, shared_ptr<PlatformInfo> >::iterator iter = platofrmList.find(platformId);
	if(iter == platofrmList.end())
	{
		return EVENTId_Error_SessionNotExist;
	}

	std::map<GBEventID,shared_ptr<GBCallinfo> >::iterator siter = iter->second->eventCallList.find(streamID);
	if(siter == iter->second->eventCallList.end())
	{
		return EVENTId_Error_SessionNotExist;
	}

	// modify by zhongwu  PlatformInfo::dialogCallList 与PlatformInfo::eventCallList 共享同一份内存，删掉其中一个之后
	// 应该将其另一个也移除列表，以防止内存崩溃
	std::map<SipDialog, shared_ptr<GBCallinfo> >::iterator citer = iter->second->dialogCallList.find(siter->second->getDialog());
	if (citer != iter->second->dialogCallList.end())
	{
		iter->second->dialogCallList.erase(citer);
	}
	iter->second->eventCallList.erase(siter);

	return EVENTId_Error_NoError;
}
EVENTId_Error GBEventPool::buildAndSendPlayCtrlReq(GBEventID& eventId,GBPlatformID platformId,GBEventID streamID,int sn,const std::string& msgXml,const std::string& contentType)
{
	shared_ptr<GBCallinfo> callinfo;
	{
		Guard locker(messageMutex);
		std::map<GBPlatformID, shared_ptr<PlatformInfo> >::iterator iter = platofrmList.find(platformId);
		if (iter == platofrmList.end())
		{
			return EVENTId_Error_SessionNotExist;
		}

		std::map<GBEventID, shared_ptr<GBCallinfo> >::iterator siter = iter->second->eventCallList.find(streamID);
		if (siter == iter->second->eventCallList.end())
		{
			return EVENTId_Error_SessionNotExist;
		}
		callinfo = siter->second;
	}
	shared_ptr<SIPSession> newsession (new SIPSession("INFO", *callinfo->getDialog().getSession().get()));
	newsession->sendRequest(msgXml,contentType);

	return addNewMessage(eventId,platformId,newsession,PLAYBACKCONTRL_Req, callinfo->getDeviceUrl(),sn,false);
}

EVENTId_Error GBEventPool::buildAndSendPlayCtrlResp(GBPlatformID platformId,GBEventID eventId,const SIPStackError& error,const std::string& msgXml,const std::string& contentType)
{
	shared_ptr<MessageInfo> info;
	{
		Guard locker(messageMutex);
		std::map<GBPlatformID, shared_ptr<PlatformInfo> >::iterator iter = platofrmList.find(platformId);
		if (iter == platofrmList.end())
		{
			return EVENTId_Error_SessionNotExist;
		}
		std::map<GBEventID, shared_ptr<MessageInfo> >::iterator eiter = iter->second->eventMessageList.find(eventId);
		if (eiter == iter->second->eventMessageList.end())
		{
			return EVENTId_Error_SessionNotExist;
		}
		info = eiter->second;
		iter->second->snMessageList.erase(eiter->second->sn);
		iter->second->dialogMessageList.erase(eiter->second->dialog);

		iter->second->eventMessageList.erase(eiter);
	}	

	info->dialog.getSession()->sendFailure(error,msgXml,contentType);

	return EVENTId_Error_NoError;
}
EVENTId_Error GBEventPool::buildAndSendMediaStatus(GBPlatformID platformId,GBEventID streamID,int sn,const std::string& msgXml,const std::string& contentType)
{
	shared_ptr<GBCallinfo> callinfo;
	{
		Guard locker(messageMutex);
		std::map<GBPlatformID, shared_ptr<PlatformInfo> >::iterator iter = platofrmList.find(platformId);
		if (iter == platofrmList.end())
		{
			return EVENTId_Error_SessionNotExist;
		}

		std::map<GBEventID, shared_ptr<GBCallinfo> >::iterator siter = iter->second->eventCallList.find(streamID);
		if (siter == iter->second->eventCallList.end())
		{
			return EVENTId_Error_SessionNotExist;
		}

		callinfo = siter->second;
	}	
	shared_ptr<SIPSession> newsession (new SIPSession("INFO", *callinfo->getDialog().getSession().get()));
	newsession->sendRequest(msgXml,contentType);

	return EVENTId_Error_NoError;
}
void GBEventPool::recvInviteReq(GBPlatformID platformId,shared_ptr<SIPSession>& session)
{
	shared_ptr<GBDispacher> dispacher;
	weak_ptr<GBStack> stack;
	GBEventID streamId;
	std::string devid;
	long long startTime;
	long long endTime;
	std::string recvip;
	int recvport;

	StreamSessionSDP sdp;
	if(!session->parseSDPByMessage(sdp))
	{
		session->sendFailure(SIPStackError(SIPStackError_ProcolError,"Parse SDP Error"));
		return;
	}

	if(sdp.ssrc == "")
	{
		std::string mediassrc;
		session->parseSDPSubject(mediassrc,sdp.ssrc);
	}

	//修正 国标流请求设备ID在TO上
	sdp.deviceId = session->getToUrl().getUserName();
	{
		Guard locker(messageMutex);
		std::map<GBPlatformID, shared_ptr<PlatformInfo> >::iterator iter = platofrmList.find(platformId);
		if(iter == platofrmList.end())
		{
			return ;
		}
		dispacher = iter->second->dispacher.lock();
		stack = iter->second->stack;

		shared_ptr<GBCallinfo> call(new GBCallinfo(iter->second->platfrom.lock()->registe,session,sdp));
		streamId = call.get();

		SipDialog dialog = call->getDialog();
		iter->second->dialogCallList.insert(pair<SipDialog,shared_ptr<GBCallinfo> >(dialog,call));
		iter->second->eventCallList.insert(pair<GBEventID,shared_ptr<GBCallinfo> >(streamId,call));
	}
	if (dispacher == NULL)
	{
		session->sendFailure(SIPStackError(SIPStackError_NoSuport));
		return;
	}
	devid = sdp.deviceId;
	startTime = sdp.vodStartTime;
	endTime = sdp.VodEndTime;
	recvip = sdp.ipAddr;
	recvport = sdp.port;

	if(sdp.type == StreamSessionType_Realplay)
	{
		dispacher->onOpenRealplayStreamReq(stack,platformId,streamId,devid,sdp.streamType,GBCallinfo::buildSDP(sdp));
	}
	else if(sdp.type == StreamSessionType_VOD)
	{
		dispacher->onOpenPlaybackStreamReq(stack,platformId,streamId,devid,startTime,endTime,GBCallinfo::buildSDP(sdp));
	}
	else
	{
		dispacher->onOpenDownloadStreamReq(stack,platformId,streamId,devid,startTime,endTime, sdp.downSpeed,GBCallinfo::buildSDP(sdp));
	}
}
void GBEventPool::recvInviteResp(GBPlatformID platformId,shared_ptr<SIPSession>& session)
{
	shared_ptr<GBDispacher> dispacher;
	weak_ptr<GBStack> stack;
	shared_ptr<GBCallinfo> callinfo;
	SipDialog dialog = session->getDialog();
	StreamSessionSDP sdp;

	GBEventID streamId;
	session->sendAck();
	session->parseSDPByMessage(sdp);

	if(sdp.ssrc == "")
	{
		std::string mediassrc;
		session->parseSDPSubject(mediassrc,sdp.ssrc);
	}

	{
		Guard locker(messageMutex);
		std::map<GBPlatformID, shared_ptr<PlatformInfo> >::iterator iter = platofrmList.find(platformId);
		if(iter == platofrmList.end())
		{
			return ;
		}
		dispacher = iter->second->dispacher.lock();
		stack = iter->second->stack;

		std::map<SipDialog,shared_ptr<GBCallinfo> >::iterator citer = iter->second->dialogCallList.find(dialog);
		if(citer == iter->second->dialogCallList.end())
		{
			return;
		}
		streamId = citer->second.get();
		callinfo = citer->second;
	}
	if (callinfo == NULL || dispacher == NULL)
	{
		return;
	}
	callinfo->recvInviteResp(session);
	dispacher->onOpenStreamResp(stack,platformId,streamId,session->getError(),GBCallinfo::buildSDP(sdp));
}
void GBEventPool::recvInviteAck(GBPlatformID platformId,shared_ptr<SIPSession>& session)
{
	shared_ptr<GBDispacher> dispacher;
	weak_ptr<GBStack> stack;
	shared_ptr<GBCallinfo> callinfo;

	SipDialog dialog = session->getDialog();
	GBEventID streamId;

	{
		Guard locker(messageMutex);
		std::map<GBPlatformID, shared_ptr<PlatformInfo> >::iterator iter = platofrmList.find(platformId);
		if(iter == platofrmList.end())
		{
			return ;
		}
		dispacher = iter->second->dispacher.lock();
		stack = iter->second->stack;

		std::map<SipDialog,shared_ptr<GBCallinfo> >::iterator citer = iter->second->dialogCallList.find(dialog);
		if(citer == iter->second->dialogCallList.end())
		{
			return;
		}
		streamId = citer->second.get();
		callinfo = citer->second;
	}

	if (callinfo == NULL || dispacher == NULL)
	{
		return;
	}
	callinfo->recvAck(session);
	dispacher->onOpenStreamAck(stack,platformId,streamId);
}

void GBEventPool::recvInviteBye(GBPlatformID platformId,shared_ptr<SIPSession>& session)
{
	session->sendSuccess();

	shared_ptr<GBDispacher> dispacher;
	weak_ptr<GBStack> stack;
	shared_ptr<GBCallinfo> callinfo;

	SipDialog dialog = session->getDialog();
	GBEventID streamId;

	{
		Guard locker(messageMutex);
		std::map<GBPlatformID, shared_ptr<PlatformInfo> >::iterator iter = platofrmList.find(platformId);
		if(iter == platofrmList.end())
		{
			return ;
		}
		dispacher = iter->second->dispacher.lock();
		stack = iter->second->stack;

		std::map<SipDialog,shared_ptr<GBCallinfo> >::iterator citer = iter->second->dialogCallList.find(dialog);
		if(citer == iter->second->dialogCallList.end())
		{
			return;
		}
		callinfo = citer->second;
		streamId = citer->second.get();
		iter->second->eventCallList.erase(citer->second.get());
		iter->second->dialogCallList.erase(citer);
	}
	if (callinfo == NULL || dispacher == NULL)
	{
		return;
	}
	callinfo->recvBye();
	dispacher->onCloseStreamReq(stack,platformId,streamId);
}
void GBEventPool::recvUpdateReq(GBPlatformID platformId,shared_ptr<SIPSession>& session)
{
	SipDialog dialog = session->getDialog();
	shared_ptr<GBCallinfo> callinfo;
	do{
		Guard locker(messageMutex);
		std::map<GBPlatformID, shared_ptr<PlatformInfo> >::iterator iter = platofrmList.find(platformId);
		if(iter == platofrmList.end())
		{
			break ;
		}
		std::map<SipDialog,shared_ptr<GBCallinfo> >::iterator citer = iter->second->dialogCallList.find(dialog);
		if(citer == iter->second->dialogCallList.end())
		{
			break;
		}
		callinfo = citer->second;
	}while(0);
	
	if(callinfo != NULL)
	{
		callinfo->recvUpdate();
		session->sendSuccess();
	}
	else
	{
		session->sendFailure(SIPStackError(SIPStackError_NoSuport,"Not Find The CallID"));
	}
}
void GBEventPool::recvUpdateResp(GBPlatformID platformId,shared_ptr<SIPSession>& session)
{
	SipDialog dialog = session->getDialog();
	shared_ptr<GBCallinfo> callinfo;
	{
		Guard locker(messageMutex);
		std::map<GBPlatformID, shared_ptr<PlatformInfo> >::iterator iter = platofrmList.find(platformId);
		if (iter == platofrmList.end())
		{
			return;
		}
		std::map<SipDialog, shared_ptr<GBCallinfo> >::iterator citer = iter->second->dialogCallList.find(dialog);
		if (citer == iter->second->dialogCallList.end())
		{
			return;
		}
		callinfo = citer->second;
	}
	if (callinfo != NULL)
	{
		callinfo->recvUpdateResp();
	}
}
struct TimeoutCallbackInfo
{
	GBPlatformID			platfromId;
	GBEventID				streamId;
	weak_ptr<GBDispacher>	dispacher;
	weak_ptr<GBStack>		stack;
};
void GBEventPool::timerProc(unsigned long param)
{	
#define SIPCOMDMAXTIMEOUT 60000

	uint64_t nowtime = Time::getCurrentMilliSecond();


	std::list<timeoutEventInfo> timeoutEventList;
	std::list<TimeoutCallbackInfo>	timeoutCallList;
	{
		Guard locker(messageMutex);
		std::map<GBPlatformID, shared_ptr<PlatformInfo> >::iterator iter;
		for(iter = platofrmList.begin();iter != platofrmList.end();iter ++)
		{
			std::map<GBEventID,shared_ptr<MessageInfo> >::iterator miter;
			std::map<GBEventID,shared_ptr<MessageInfo> >::iterator miter1;
			for(miter = iter->second->eventMessageList.begin();miter != iter->second->eventMessageList.end();miter = miter1)
			{
				miter1 = miter;
				miter1 ++;
				if(nowtime - miter->second->startTime >= SIPCOMDMAXTIMEOUT)
				{
					timeoutEventInfo info;
					info.id = iter->first;
					info.info = miter->second;
					timeoutEventList.push_back(info);

					iter->second->snMessageList.erase(miter->second->sn);
					iter->second->dialogMessageList.erase(miter->second->dialog);
					iter->second->eventMessageList.erase(miter);
				}
			}
			for(std::map<GBEventID,shared_ptr<GBCallinfo> >::iterator citer = iter->second->eventCallList.begin();citer != iter->second->eventCallList.end();)
			{
				if(citer->second->poolCallAndCheckIsTimeout())
				{
					TimeoutCallbackInfo info;
					info.platfromId = iter->first;
					info.streamId = citer->first;
					info.dispacher = iter->second->dispacher;
					info.stack = iter->second->stack;

					timeoutCallList.push_back(info);

					iter->second->dialogCallList.erase(citer->second->getDialog());
					iter->second->eventCallList.erase(citer++);
				}
				else
				{
					citer ++;
				}
			}
		}		
	}
	for(std::list<TimeoutCallbackInfo>::iterator iter = timeoutCallList.begin();iter != timeoutCallList.end();iter ++)
	{
		shared_ptr<GBDispacher> dispatcher = iter->dispacher.lock();
		if (dispatcher != NULL)
		{
			dispatcher->onCloseStreamReq(iter->stack, iter->platfromId, iter->streamId);
		}
	}

	std::list<timeoutEventInfo>::iterator iter;
	for(iter = timeoutEventList.begin();iter != timeoutEventList.end();iter ++)
	{
		if(iter->info != NULL)
		{
			procMessage(iter->id,iter->info->id,SIPStackError_Timeout);
		}
	}
}
std::string GBEventPool::getDeviceId(GBPlatformID platformId,GBEventID streamID)
{
	Guard locker(messageMutex);
	std::map<GBPlatformID, shared_ptr<PlatformInfo> >::iterator iter = platofrmList.find(platformId);
	if(iter == platofrmList.end())
	{
		return  "";
	}
	std::map<GBEventID,shared_ptr<GBCallinfo> >::iterator siter = iter->second->eventCallList.find(streamID);
	if(siter == iter->second->eventCallList.end())
	{
		return "";
	}

	return siter->second->getDeviceUrl();
}
uint32_t GBEventPool::getMessageSN(GBPlatformID platformId)
{
	Guard locker(messageMutex);

	std::map<GBPlatformID, shared_ptr<PlatformInfo> >::iterator iter = platofrmList.find(platformId);
	if(iter == platofrmList.end())
	{
		return 0;
	}
	srand((unsigned int)Time::getCurrentMilliSecond());
	while(1)
	{
		uint32_t randnum = rand();
		std::map<int,shared_ptr<MessageInfo> >::iterator piter = iter->second->snMessageList.find(randnum);
		if(piter == iter->second->snMessageList.end())
		{
			return randnum;
		}
	} 
	return 0;
}


};
};
