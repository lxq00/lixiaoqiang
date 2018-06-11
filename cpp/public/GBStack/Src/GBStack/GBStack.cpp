#include "Network/Network.h"
#include "GBStack/GBStack.h"
#include "GBStackInternal.h"
#include "GBEventPool.h"
#include "GBStack/publicid.h"
#include "Base/Thread.h"

#define MAXUDPNUMPERPACKET	2
#define MAXTCPNUMPERPACKET	16
using namespace Public::Network;
namespace Public{
namespace GB28181{
GBStack::GBStack()
{
	internal = make_shared<GBStackInternal>();
}
GBStack::~GBStack() 
{
	internal = NULL;
}

bool GBStack::init(const weak_ptr<GBDispacher>& dispacher, const std::string& myId, const std::string& myUser, const std::string& myPass, const std::string& uAgent)
{
	internal->init(shared_from_this());
	internal->dispacher = dispacher;
	internal->myId = myId;
	internal->myUser = myUser;
	internal->myPass = myPass;
	internal->uAgent = uAgent;

	return true;
}

EVENTId_Error GBStack::udpListen(GBCommuID& commuId,int myPort,const std::string& myIp)
{
	if(myPort <= 0)
	{
		return EVENTId_Error_ParaInvaild;
	}
	uint32_t mylistenport = myPort == 0 ? 5060 : myPort;

	Guard locker(internal->gbmutex);

	std::map<GBCommuID,shared_ptr<SIPTransaction> >::iterator iter;
	for(iter = internal->communicationList.begin();iter != internal->communicationList.end();iter ++)
	{
		if(iter->second->getTransType() == SIPTransaction::SIPTransactionType_UDP && iter->second->getMyPort() == (uint32_t)myPort)
		{
			commuId = iter->first;
			return EVENTId_Error_NoError;
		}
	}	

	shared_ptr<SIPTransaction> info;
	info = make_shared<SIPTransaction>(SIPTransaction::SIPTransactionType_UDP,mylistenport,internal->uAgent,internal);
	
	internal->communicationList[info.get()] = info;

	commuId = info.get();
	
	return EVENTId_Error_NoError;
}
EVENTId_Error GBStack::tcpListen(GBCommuID& commuId,int myPort)
{
	if(myPort <= 0)
	{
		return EVENTId_Error_ParaInvaild;
	}

	uint32_t mylistenport = myPort == 0 ? 5060 : myPort;

	Guard locker(internal->gbmutex);

	std::map<GBCommuID,shared_ptr<SIPTransaction> >::iterator iter;
	for(iter = internal->communicationList.begin();iter != internal->communicationList.end();iter ++)
	{
		if(iter->second->getTransType() == SIPTransaction::SIPTransactionType_TCP && iter->second->getMyPort() == (uint32_t)myPort)
		{
			commuId = iter->first;
			return EVENTId_Error_NoError;
		}
	}
	shared_ptr<SIPTransaction> info = make_shared<SIPTransaction>(SIPTransaction::SIPTransactionType_TCP,mylistenport,internal->uAgent,internal);

	internal->communicationList[info.get()] = info;

	commuId = info.get();

	return EVENTId_Error_NoError;
}
EVENTId_Error GBStack::tcpConnect(GBCommuID& commuId,const std::string& destIp,int destPort)
{
	if(destIp == "" || destPort <= 0)
	{
		return EVENTId_Error_ParaInvaild;
	}
	
	Guard locker(internal->gbmutex);

	std::map<GBCommuID,shared_ptr<SIPTransaction> >::iterator iter;
	for(iter = internal->communicationList.begin();iter != internal->communicationList.end();iter ++)
	{
		if(iter->second->getTransType() == SIPTransaction::SIPTransactionType_TCP)
		{
			commuId = iter->first;
			return EVENTId_Error_NoError;
		}
	}
	uint32_t mylistenport = Host::getAvailablePort(11311,Host::SocketType_TCP);

	shared_ptr<SIPTransaction> info = make_shared<SIPTransaction>(SIPTransaction::SIPTransactionType_TCP,mylistenport,internal->uAgent,internal); 

	internal->communicationList[info.get()] = info;

	commuId = info.get();
	
	return EVENTId_Error_NoError;
}
EVENTId_Error GBStack::closeCommu(GBCommuID commuId)
{
	{
		Guard locker(internal->gbmutex);

		std::map<GBCommuID,shared_ptr<SIPTransaction> >::iterator iter = internal->communicationList.find(commuId);
		if(iter == internal->communicationList.end())
		{
			return EVENTId_Error_CommuNotExist;
		}
		internal->cutoffComList.push_back(iter->second);
		internal->communicationList.erase(iter);
	}

	return EVENTId_Error_NoError;
}
EVENTId_Error GBStack::sendRegisteReq(GBPlatformID& platformId,GBCommuID commuId,const std::string& name,const std::string& pass,const std::string& svrId,const std::string& svrIp,int svrPort,int expries)
{
	shared_ptr<SIPTransaction> trackaction;
	{
		Guard locker(internal->gbmutex);

		std::map<GBCommuID, shared_ptr<SIPTransaction> >::iterator iter = internal->communicationList.find(commuId);
		if (iter == internal->communicationList.end())
		{
			return EVENTId_Error_CommuNotExist;
		}
		trackaction = iter->second;
		{
			for (std::map<SIPTransport, shared_ptr<GBPlatformInfo> >::iterator iter = internal->platformTransportMap.begin(); iter != internal->platformTransportMap.end(); iter++)
			{
				if (iter->second->clientId == svrId && iter->second->clientIp == svrIp && iter->second->clientPort == svrPort)
				{
					platformId = iter->second.get();
					return EVENTId_Error_NoError;
				}
			}
		}
	}

	shared_ptr<GBPlatformInfo> platform(new GBPlatformInfo);
	platform->type = GBPlatformInfo::PlatformType_Up;
	platform->clientId = svrId;
	platform->toIp = platform->requestIp = platform->clientIp = svrIp;
	platform->clientPort = svrPort;	
	platform->fromIp = platform->myIp = Host::guessMyIpaddr(svrIp);
	platform->usrename = name;
	platform->passwd = pass;
	platform->myid = internal->myId;
	platform->myusername = internal->myUser;
	platform->mypasswd = internal->myPass;
	platform->transaction = trackaction;
	platform->status = GBPlatformInfo::PlatformStatus_Init;

	platform->registe = make_shared<GBRegister>(shared_from_this(),internal->dispacher,platform,GBRegister::CuttoffCallback(&GBStackInternal::platfromCutoff,internal.get()));
	platform->registe->regist(expries);

	SIPTransport trans(NetAddr(svrIp,svrPort), trackaction.get());
	internal->platformTransportMap[trans] = platform;
	
	internal->addPlatform(shared_from_this(),internal->dispacher,platform);
	
	platformId = platform.get();

	return EVENTId_Error_NoError;
}

EVENTId_Error GBStack::sendRegisteResp(GBCommuID commuId,GBPlatformID platformId, const SIPStackError& error,unsigned long long dateTime)
{
	Guard locker(internal->gbmutex);

	std::map<GBCommuID,shared_ptr<SIPTransaction> >::iterator iter = internal->communicationList.find(commuId);
	if(iter == internal->communicationList.end())
	{
		return EVENTId_Error_CommuNotExist;
	}
	shared_ptr<GBPlatformInfo> platfrom = internal->_queryPlatfromById(platformId);
	if(platfrom == NULL)
	{
		return EVENTId_Error_PlatformNotExist;
	}

	if(error.getErrorCode() == SIPStackError_OK)
	{
		if(platfrom->registe != NULL)
		{
			platfrom->registe->setAddResiterAck(error,dateTime);
		}
		platfrom->status = GBPlatformInfo::PlatformStatus_Success;
	}
	else
	{
		if(platfrom->registe != NULL)
		{
			platfrom->registe->setAddResiterAck(error);
		}

		std::map<SIPTransport,shared_ptr<GBPlatformInfo> >::iterator iter = internal->platformTransportMap.find(platfrom->transport);
		if(iter != internal->platformTransportMap.end() && iter->second->type != GBPlatformInfo::PlatformType_Up)
		{
			internal->cutoffPlatList.push_back(platfrom);
			internal->platformTransportMap.erase(iter);
		}		
	}

	return EVENTId_Error_NoError;
}

EVENTId_Error GBStack::closeRegiste(GBCommuID commuId,GBPlatformID platformId)
{
	Guard locker(internal->gbmutex);

	std::map<GBCommuID,shared_ptr<SIPTransaction> >::iterator citer = internal->communicationList.find(commuId);
	if(citer == internal->communicationList.end())
	{
		return EVENTId_Error_CommuNotExist;
	}
	shared_ptr<GBPlatformInfo> platfrom = internal->_queryPlatfromById(platformId);
	if(platfrom == NULL)
	{
		return EVENTId_Error_PlatformNotExist;
	}

	std::map<SIPTransport,shared_ptr<GBPlatformInfo> >::iterator iter = internal->platformTransportMap.find(platfrom->transport);
	if(iter != internal->platformTransportMap.end())
	{
		internal->cutoffPlatList.push_back(platfrom);
		internal->platformTransportMap.erase(iter);
	}	
	
	return EVENTId_Error_NoError;
}

EVENTId_Error GBStack::sendGetCatalogReq(GBEventID& eventid,GBPlatformID platformId)
{
	shared_ptr<GBPlatformInfo> platfrom;
	{
		Guard locker(internal->gbmutex);

		platfrom = internal->_queryPlatfromById(platformId);
		if (platfrom == NULL)
		{
			return EVENTId_Error_PlatformNotExist;
		}

		if (platfrom->status != GBPlatformInfo::PlatformStatus_Success)
		{
			return EVENTId_Error_NotRegist;
		}
	}

	TDeviceCatalogQueryReq catalogquery;

	catalogquery.nSn = internal->getRandNum(platformId);
	catalogquery.strDeviceId = platfrom->registe->getToUrl().getUserName();
	
	string strXml;
	BuildMsg(E_PT_DEVICE_CATALOG_QUERY_REQ,&catalogquery,strXml);

	return internal->buildAndSendRequest(eventid,platformId,"MESSAGE",GBEventPool::CATALOG_Req,catalogquery.nSn,catalogquery.strDeviceId,strXml,XMLCONTENTTYPE);
}
EVENTId_Error GBStack::sendGetCatalogResp(GBPlatformID platformId,GBEventID id, const SIPStackError& error,uint32_t total,bool isEnd,const std::list<TDeviceItem>& resp)
{
	int maxsendSize = MAXUDPNUMPERPACKET;

	{
		Guard locker(internal->gbmutex);

		shared_ptr<GBPlatformInfo> platfrom = internal->_queryPlatfromById(platformId);
		if (platfrom == NULL)
		{
			return EVENTId_Error_PlatformNotExist;
		}

		if (platfrom->status != GBPlatformInfo::PlatformStatus_Success)
		{
			return EVENTId_Error_NotRegist;
		}

		maxsendSize = platfrom->transaction->getTransType() == SIPTransaction::SIPTransactionType_TCP ? MAXTCPNUMPERPACKET : MAXUDPNUMPERPACKET;
	}

	int messageSn;
	std::string devId;

	if(internal->getMessageInfo(platformId,id,messageSn,devId) != EVENTId_Error_NoError)
	{
		return EVENTId_Error_SessionNotExist;
	}	

	std::list<TDeviceItem>::const_iterator respiter = resp.begin();
	do
	{
		TDeviceCatalogQueryResp cataresp;
		cataresp.strDeviceId = devId;
		cataresp.nSn = messageSn;
		cataresp.nSumNum = total;

		for(int i = 0;i < maxsendSize && respiter != resp.end();i ++,respiter++)
		{
			cataresp.vDeviceItem.push_back(*respiter);
		}
		cataresp.bEndofFile = respiter == resp.end()?isEnd:false;
		std::string strXml;
		BuildMsg(E_PT_DEVICE_CATALOG_QUERY_RESP,&cataresp,strXml);

		internal->buildAndSendResponse(platformId,id,"MESSAGE",strXml,XMLCONTENTTYPE,respiter == resp.end()?isEnd:false);
	}while(respiter != resp.end());

	return EVENTId_Error_NoError;
}
EVENTId_Error GBStack::sendDevicePtzReq(GBEventID& eventid,GBPlatformID platformId,const std::string& devid,GBPtzDirection direction,int speed)
{
	{
		Guard locker(internal->gbmutex);

		shared_ptr<GBPlatformInfo> platfrom = internal->_queryPlatfromById(platformId);
		if (platfrom == NULL)
		{
			return EVENTId_Error_PlatformNotExist;
		}

		if (platfrom->status != GBPlatformInfo::PlatformStatus_Success)
		{
			return EVENTId_Error_NotRegist;
		}
	}	

	TDeviceControlReq req;
	req.ptz = direction;
	req.ptzParam = speed;
	req.nSn = internal->getRandNum(platformId);
	req.strDeviceId = devid;

	string strXml;
	BuildMsg(E_PT_DEVICE_CONTROL_REQ,&req,strXml);

	return internal->buildAndSendRequest(eventid,platformId,"MESSAGE",GBEventPool::DEVICEPTZ_Req,req.nSn,devid,strXml,XMLCONTENTTYPE);
}

EVENTId_Error GBStack::sendDeviceRebootReq(GBEventID& eventid,GBPlatformID platformId,const std::string& devid)
{
	{
		Guard locker(internal->gbmutex);

		shared_ptr<GBPlatformInfo> platfrom = internal->_queryPlatfromById(platformId);
		if (platfrom == NULL)
		{
			return EVENTId_Error_PlatformNotExist;
		}

		if (platfrom->status != GBPlatformInfo::PlatformStatus_Success)
		{
			return EVENTId_Error_NotRegist;
		}
	}	

	TDeviceControlReq req;

	req.nSn = internal->getRandNum(platformId);
	req.strDeviceId = devid;
	req.bTeleBoot = true;

	string strXml;
	BuildMsg(E_PT_DEVICE_CONTROL_REQ,&req,strXml);

	return internal->buildAndSendRequest(eventid,platformId,"MESSAGE",GBEventPool::DEVICEREBOOT_Req,req.nSn,devid,strXml,XMLCONTENTTYPE);
}
EVENTId_Error GBStack::sendDeviceRecordReq(GBEventID& eventid,GBPlatformID platformId,const std::string& devid,bool record)
{
	{
		Guard locker(internal->gbmutex);

		shared_ptr<GBPlatformInfo> platfrom = internal->_queryPlatfromById(platformId);
		if (platfrom == NULL)
		{
			return EVENTId_Error_PlatformNotExist;
		}

		if (platfrom->status != GBPlatformInfo::PlatformStatus_Success)
		{
			return EVENTId_Error_NotRegist;
		}
	}

	TDeviceControlReq req;

	req.nSn = internal->getRandNum(platformId);
	req.strDeviceId = devid;
	req.eRecordCmd = record?E_RC_RECORD:E_RC_STOP_RECORD;

	string strXml;
	BuildMsg(E_PT_DEVICE_CONTROL_REQ,&req,strXml);

	return internal->buildAndSendRequest(eventid,platformId,"MESSAGE",GBEventPool::DEVICERECORD_Req,req.nSn,devid,strXml,XMLCONTENTTYPE);
}
EVENTId_Error GBStack::sendAlarmGuardReq(GBEventID& eventid, GBPlatformID platformId, const std::string& devid, bool guard)
{
	{
		Guard locker(internal->gbmutex);

		shared_ptr<GBPlatformInfo> platfrom = internal->_queryPlatfromById(platformId);
		if (platfrom == NULL)
		{
			return EVENTId_Error_PlatformNotExist;
		}

		if (platfrom->status != GBPlatformInfo::PlatformStatus_Success)
		{
			return EVENTId_Error_NotRegist;
		}
	}

	TDeviceControlReq req;

	req.nSn = internal->getRandNum(platformId);
	req.strDeviceId = devid;
	req.eGuardCmd = guard?E_GC_SET_GUARD:E_GC_REST_GUARD;

	string strXml;
	BuildMsg(E_PT_DEVICE_CONTROL_REQ,&req,strXml);

	return internal->buildAndSendRequest(eventid,platformId,"MESSAGE",GBEventPool::ALARMGUARD_Req,req.nSn,devid,strXml,XMLCONTENTTYPE);
}
EVENTId_Error GBStack::sendAlarmResetReq(GBEventID& eventid,GBPlatformID platformId,const std::string& devid)
{
	{
		Guard locker(internal->gbmutex);

		shared_ptr<GBPlatformInfo> platfrom = internal->_queryPlatfromById(platformId);
		if (platfrom == NULL)
		{
			return EVENTId_Error_PlatformNotExist;
		}

		if (platfrom->status != GBPlatformInfo::PlatformStatus_Success)
		{
			return EVENTId_Error_NotRegist;
		}
	}
	TDeviceControlReq req;

	req.nSn = internal->getRandNum(platformId);
	req.strDeviceId = devid;
	req.bAlarmCmd = true;

	string strXml;
	BuildMsg(E_PT_DEVICE_CONTROL_REQ,&req,strXml);
	
	return internal->buildAndSendRequest(eventid,platformId,"MESSAGE",GBEventPool::ALARMRESET_Req,req.nSn,devid,strXml,XMLCONTENTTYPE);
}
EVENTId_Error GBStack::sendDeviceContrlResp(GBPlatformID platformId,GBEventID id, const SIPStackError& error)
{
	{
		Guard locker(internal->gbmutex);

		shared_ptr<GBPlatformInfo> platfrom = internal->_queryPlatfromById(platformId);
		if (platfrom == NULL)
		{
			return EVENTId_Error_PlatformNotExist;
		}

		if (platfrom->status != GBPlatformInfo::PlatformStatus_Success)
		{
			return EVENTId_Error_NotRegist;
		}
	}

	int messageSn;
	std::string devId;

	if(internal->getMessageInfo(platformId,id,messageSn,devId) != EVENTId_Error_NoError)
	{
		return EVENTId_Error_SessionNotExist;
	}

	TDeviceControlResp resp;
	resp.strDeviceId = devId;
	resp.nSn = messageSn;
	resp.bResult = (error.getErrorCode() == SIPStackError_OK) ? true: false;

	std::string strXml;
	BuildMsg(E_PT_DEVICE_CONTROL_RESP,&resp,strXml);


	return internal->buildAndSendResponse(platformId,id,"MESSAGE",strXml,XMLCONTENTTYPE);
}
EVENTId_Error GBStack::sendAlarmSubscribeReq(GBEventID& eventid,GBPlatformID platformId,const std::string& devid,const AlarmSubcribe& subcribe)
{
	shared_ptr<GBPlatformInfo> platfrom;

	{
		Guard locker(internal->gbmutex);

		platfrom = internal->_queryPlatfromById(platformId);
		if (platfrom == NULL)
		{
			return EVENTId_Error_PlatformNotExist;
		}

		if (platfrom->status != GBPlatformInfo::PlatformStatus_Success)
		{
			return EVENTId_Error_NotRegist;
		}

		if (platfrom->status != GBPlatformInfo::PlatformStatus_Success)
		{
			return EVENTId_Error_NotRegist;
		}
	}	

	TAlarmQueryReq alarmquery;

	alarmquery.nSn = internal->getRandNum(platformId);
	alarmquery.strDeviceId = platfrom->registe->getToUrl().getUserName();
	alarmquery.nStartAlarmPriority = subcribe.StartAlarmPriority;
	alarmquery.nEndAlarmPriority = subcribe.EndAlarmPriority;
	alarmquery.nAlarmMethod = subcribe.AlarmMethod;
	alarmquery.strStartTime = SecToSipTime(subcribe.StartTime);
	alarmquery.strStartTime = SecToSipTime(subcribe.EndTime);

	string strXml;
	BuildMsg(E_PT_ALARM_SUBSCRIBE_REQ,&alarmquery,strXml);

	return internal->buildAndSendRequest(eventid,platformId,"SUBSCRIBE",GBEventPool::ALARMSUBSCRIBE_Req,alarmquery.nSn,alarmquery.strDeviceId,strXml,XMLCONTENTTYPE);
}
EVENTId_Error GBStack::sendAlarmSubscribeResp(GBPlatformID platformId,GBEventID id, const SIPStackError& error)
{
	{
		Guard locker(internal->gbmutex);

		shared_ptr<GBPlatformInfo> platfrom = internal->_queryPlatfromById(platformId);
		if (platfrom == NULL)
		{
			return EVENTId_Error_PlatformNotExist;
		}

		if (platfrom->status != GBPlatformInfo::PlatformStatus_Success)
		{
			return EVENTId_Error_NotRegist;
		}
	}

	int messageSn;
	std::string devId;

	if(internal->getMessageInfo(platformId,id,messageSn,devId) != EVENTId_Error_NoError)
	{
		return EVENTId_Error_SessionNotExist;
	}

	TAlarmNotifyResp resp;

	resp.strDeviceId = devId;
	resp.nSn = messageSn;
	resp.bResult = (error.getErrorCode() == SIPStackError_OK) ? true: false;

	string strXml;
	BuildMsg(E_PT_ALARM_NOTIFY_RESP,&resp,strXml);


	return internal->buildAndSendResponse(platformId,id,"",strXml,XMLCONTENTTYPE);
}

EVENTId_Error GBStack::sendCatalogSubscribeReq(GBEventID& eventid,GBPlatformID platformId,const std::string& devid,long long startTime,long long endTime)
{
	shared_ptr<GBPlatformInfo> platfrom;

	{
		Guard locker(internal->gbmutex);

		platfrom = internal->_queryPlatfromById(platformId);
		if (platfrom == NULL)
		{
			return EVENTId_Error_PlatformNotExist;
		}

		if (platfrom->status != GBPlatformInfo::PlatformStatus_Success)
		{
			return EVENTId_Error_NotRegist;
		}
	}

	TDeviceCatalogQueryReq catalogquery;

	catalogquery.nSn = internal->getRandNum(platformId);
	catalogquery.strDeviceId = platfrom->registe->getToUrl().getUserName();
	catalogquery.bSubcribe = true;
	catalogquery.strStartTime = SecToSipTime(startTime);
	catalogquery.strStartTime = SecToSipTime(endTime);

	string strXml;
	BuildMsg(E_PT_DEVICE_CATALOG_QUERY_REQ,&catalogquery,strXml);

	return internal->buildAndSendRequest(eventid,platformId,"SUBSCRIBE",GBEventPool::CATALOG_Req,catalogquery.nSn,catalogquery.strDeviceId,strXml,XMLCONTENTTYPE);
}

EVENTId_Error GBStack::sendCatalogSubscribeResp(GBPlatformID platformId,GBEventID id, const SIPStackError& error)
{
	{
		Guard locker(internal->gbmutex);

		shared_ptr<GBPlatformInfo> platfrom = internal->_queryPlatfromById(platformId);
		if (platfrom == NULL)
		{
			return EVENTId_Error_PlatformNotExist;
		}

		if (platfrom->status != GBPlatformInfo::PlatformStatus_Success)
		{
			return EVENTId_Error_NotRegist;
		}
	}	

	int messageSn;
	std::string devId;

	if(internal->getMessageInfo(platformId,id,messageSn,devId) != EVENTId_Error_NoError)
	{
		return EVENTId_Error_SessionNotExist;
	}

	TDeviceCatalogQueryResp resp;

	resp.strDeviceId = devId;
	resp.nSn = messageSn;
	resp.bSubcribe = true;
	resp.bResult = (error.getErrorCode() == SIPStackError_OK) ? true: false;

	string strXml;
	BuildMsg(E_PT_DEVICE_CATALOG_QUERY_RESP,&resp,strXml);


	return internal->buildAndSendResponse(platformId,id,"",strXml,XMLCONTENTTYPE);
}
EVENTId_Error GBStack::sendCatalogNotifyReq(GBPlatformID platformId,const std::string& devid,uint32_t total,bool isEnd,const std::list<TDeviceItem>& resp)
{
	shared_ptr<GBPlatformInfo> platfrom;
	int maxsendSize = MAXUDPNUMPERPACKET;
	{
		Guard locker(internal->gbmutex);

		platfrom = internal->_queryPlatfromById(platformId);
		if (platfrom == NULL)
		{
			return EVENTId_Error_PlatformNotExist;
		}

		if (platfrom->status != GBPlatformInfo::PlatformStatus_Success)
		{
			return EVENTId_Error_NotRegist;
		}

		maxsendSize = platfrom->transaction->getTransType() == SIPTransaction::SIPTransactionType_TCP ? MAXTCPNUMPERPACKET : MAXUDPNUMPERPACKET;
	}

	int messageSn = 0;
	std::string sdevId;

	if (internal->getSubcribeInfo(platformId, GBEventPool::CATALOG_Req, messageSn, sdevId) != EVENTId_Error_NoError)
	{
		return EVENTId_Error_SessionNotExist;
	}

	std::list<TDeviceItem>::const_iterator respiter = resp.begin();
	do
	{
		TDeviceCatalogQueryResp cataresp;
		cataresp.strDeviceId = devid;
		cataresp.nSn = messageSn;
		cataresp.nSumNum = total;

		for(int i = 0;i < maxsendSize && respiter != resp.end();i ++,respiter++)
		{
			cataresp.vDeviceItem.push_back(*respiter);
		}
		cataresp.bEndofFile = respiter == resp.end()?isEnd:false;
		std::string strXml;
		BuildMsg(E_PT_DEVICE_CATALOG_QUERY_RESP,&cataresp,strXml);

		GBEventID eventid;
		internal->buildAndSendRequest(eventid,platformId,"NOTIFY",GBEventPool::CATALOG_Req,cataresp.nSn,platfrom->clientId,strXml,XMLCONTENTTYPE);
	}while(respiter != resp.end());

	return EVENTId_Error_NoError;
}

EVENTId_Error GBStack::sendCatalogNotifyResp(GBPlatformID platformId,GBEventID id, const SIPStackError& error)
{
	{
		Guard locker(internal->gbmutex);

		shared_ptr<GBPlatformInfo> platfrom = internal->_queryPlatfromById(platformId);
		if (platfrom == NULL)
		{
			return EVENTId_Error_PlatformNotExist;
		}

		if (platfrom->status != GBPlatformInfo::PlatformStatus_Success)
		{
			return EVENTId_Error_NotRegist;
		}
	}	

	int messageSn;
	std::string devId;

	if(internal->getMessageInfo(platformId,id,messageSn,devId) != EVENTId_Error_NoError)
	{
		return EVENTId_Error_SessionNotExist;
	}

	TDeviceCatalogQueryResp resp;

	resp.strDeviceId = devId;
	resp.nSn = messageSn;
	resp.bSubcribe = true;
	resp.bResult = (error.getErrorCode() == SIPStackError_OK) ? true: false;

	string strXml;
	BuildMsg(E_PT_DEVICE_CATALOG_QUERY_RESP,&resp,strXml);


	return internal->buildAndSendResponse(platformId,id,"",strXml,XMLCONTENTTYPE);
}
EVENTId_Error GBStack::sendAlarmNotifyReq(GBEventID& eventid,GBPlatformID platformId,const std::string& devid,const AlarmNotifyInfo& info)
{
	shared_ptr<GBPlatformInfo> platfrom;
	{
		Guard locker(internal->gbmutex);

		platfrom = internal->_queryPlatfromById(platformId);
		if (platfrom == NULL)
		{
			return EVENTId_Error_PlatformNotExist;
		}

		if (platfrom->status != GBPlatformInfo::PlatformStatus_Success)
		{
			return EVENTId_Error_NotRegist;
		}
	}	

	int messageSn = 0;
	std::string sdevId;

	if (internal->getSubcribeInfo(platformId, GBEventPool::ALARMRESET_Req, messageSn, sdevId) != EVENTId_Error_NoError)
	{
		return EVENTId_Error_SessionNotExist;
	}

	TAlarmNotifyReq req;

	req.nSn = messageSn;
	req.strDeviceId = devid;
	req.nAlarmPriority = info.Priority;
	req.nAlarmMethod = info.Method;
	req.strAlarmTime = SecToSipTime(info.Time);
	req.strAlarmDescription = info.Description;
	req.vExtendInfo = info.ExtendInfo;

	string strXml;
	BuildMsg(E_PT_ALARM_NOTIFY_REQ,&req,strXml);
	
	return internal->buildAndSendRequest(eventid,platformId,"NOTIFY",GBEventPool::ALARMNOTIFY_Req,req.nSn,platfrom->clientId,strXml,XMLCONTENTTYPE);
}
EVENTId_Error GBStack::sendAlarmNotifyResp(GBPlatformID platformId,GBEventID id, const SIPStackError& error)
{
	{
		Guard locker(internal->gbmutex);

		shared_ptr<GBPlatformInfo> platfrom = internal->_queryPlatfromById(platformId);
		if (platfrom == NULL)
		{
			return EVENTId_Error_PlatformNotExist;
		}

		if (platfrom->status != GBPlatformInfo::PlatformStatus_Success)
		{
			return EVENTId_Error_NotRegist;
		}
	}	

	int messageSn;
	std::string devId;

	if(internal->getMessageInfo(platformId,id,messageSn,devId) != EVENTId_Error_NoError)
	{
		return EVENTId_Error_SessionNotExist;
	}

	TAlarmNotifyResp resp;
	resp.strDeviceId = devId;
	resp.nSn = messageSn;
	resp.bResult = (error.getErrorCode() == SIPStackError_OK) ? true: false;

	string strXml;
	BuildMsg(E_PT_ALARM_NOTIFY_RESP,&resp,strXml);


	return internal->buildAndSendResponse(platformId,id,"MESSAGE",strXml,XMLCONTENTTYPE);
}
EVENTId_Error GBStack::sendQueryDeviceStatusReq(GBEventID& eventid,GBPlatformID platformId,const std::string& devid)
{
	{
		Guard locker(internal->gbmutex);

		shared_ptr<GBPlatformInfo> platfrom = internal->_queryPlatfromById(platformId);
		if (platfrom == NULL)
		{
			return EVENTId_Error_PlatformNotExist;
		}

		if (platfrom->status != GBPlatformInfo::PlatformStatus_Success)
		{
			return EVENTId_Error_NotRegist;
		}
	}	

	TDeviceStatusQueryReq req;

	req.nSn = internal->getRandNum(platformId);
	req.strDeviceId = devid;

	string strXml;
	BuildMsg(E_PT_DEVICE_STATUS_QUERY_REQ,&req,strXml);
	
	return internal->buildAndSendRequest(eventid,platformId,"MESSAGE",GBEventPool::QUERYDEVICESTATUS_Req,req.nSn,devid,strXml,XMLCONTENTTYPE);
}
EVENTId_Error GBStack::sendQueryDeviceStatusResp(GBPlatformID platformId,GBEventID id, const SIPStackError& error,const GBDeviceStatus& status)
{
	{
		Guard locker(internal->gbmutex);

		shared_ptr<GBPlatformInfo> platfrom = internal->_queryPlatfromById(platformId);
		if (platfrom == NULL)
		{
			return EVENTId_Error_PlatformNotExist;
		}

		if (platfrom->status != GBPlatformInfo::PlatformStatus_Success)
		{
			return EVENTId_Error_NotRegist;
		}
	}	

	int messageSn;
	std::string devId;

	if(internal->getMessageInfo(platformId,id,messageSn,devId) != EVENTId_Error_NoError)
	{
		return EVENTId_Error_SessionNotExist;
	}

	TDeviceStatusQueryResp resp;
	resp.strDeviceId = devId;
	resp.nSn = messageSn;
	resp.bResult = error.getErrorCode() == SIPStackError_OK ? true : false;
	resp.bRecord = status.recordStatus;
	resp.bOnline = status.onLine;
	resp.bEncode = status.encodeStatus;
	resp.bStatus = status.workStatus;
	resp.strDeviceTime = status.deviceTime;
	resp.vTAlarmStatus.resize(status.alarmStatus.size());

	int index = 0;
		
	std::list<TAlarmStatus>::const_iterator iter;
	for(iter = status.alarmStatus.begin();iter != status.alarmStatus.end();iter ++)
	{
		resp.vTAlarmStatus[index] = *iter;
		index ++;
	}

	string strXml;
	BuildMsg(E_PT_DEVICE_STATUS_QUERY_RESP,&resp,strXml);

	Thread::sleep(15);

	return internal->buildAndSendResponse(platformId,id,"MESSAGE",strXml,XMLCONTENTTYPE);
}

EVENTId_Error GBStack::sendDeviceStatusNotify(GBPlatformID platformId,const std::string& devid,bool online)
{
	shared_ptr<GBPlatformInfo> platfrom;
	{
		Guard locker(internal->gbmutex);

		platfrom = internal->_queryPlatfromById(platformId);
		if (platfrom == NULL)
		{
			return EVENTId_Error_PlatformNotExist;
		}

		if (platfrom->status != GBPlatformInfo::PlatformStatus_Success)
		{
			return EVENTId_Error_NotRegist;
		}
	}
	TKeepAliveNotify req;

	req.nSn = internal->getRandNum(platformId);
	req.strDeviceId = devid;
	req.bStatus = online;

	string strXml;
	BuildMsg(E_PT_KEEP_ALIVE_NOTIFY,&req,strXml);

	GBEventID eventid;

	return internal->buildAndSendRequest(eventid,platformId,"NOTIFY",GBEventPool::QUERYDEVICESTATUS_Req,req.nSn,platfrom->clientId,strXml,XMLCONTENTTYPE);
}

EVENTId_Error GBStack::sendQueryDeviceInfoReq(GBEventID& eventid,GBPlatformID platformId,const std::string& devid)
{
	{
		Guard locker(internal->gbmutex);

		shared_ptr<GBPlatformInfo> platfrom = internal->_queryPlatfromById(platformId);
		if (platfrom == NULL)
		{
			return EVENTId_Error_PlatformNotExist;
		}

		if (platfrom->status != GBPlatformInfo::PlatformStatus_Success)
		{
			return EVENTId_Error_NotRegist;
		}
	}
	
	
	TDeviceInfoQueryReq req;
	
	req.nSn = internal->getRandNum(platformId);
	req.strDeviceId = devid;

	string strXml;
	BuildMsg(E_PT_DEVICE_INFO_QUERY_REQ,&req,strXml);
	
	
	return internal->buildAndSendRequest(eventid,platformId,"MESSAGE",GBEventPool::QUERYDEVICEINFO_Req,req.nSn,devid,strXml,XMLCONTENTTYPE);
}
EVENTId_Error GBStack::sendQueryDeviceInfoResp(GBPlatformID platformId,GBEventID id, const SIPStackError& error,const GBDeviceInfo& info)
{
	{
		Guard locker(internal->gbmutex);

		shared_ptr<GBPlatformInfo> platfrom = internal->_queryPlatfromById(platformId);
		if (platfrom == NULL)
		{
			return EVENTId_Error_PlatformNotExist;
		}

		if (platfrom->status != GBPlatformInfo::PlatformStatus_Success)
		{
			return EVENTId_Error_NotRegist;
		}
	}	

	int messageSn;
	std::string devId;

	if(internal->getMessageInfo(platformId,id,messageSn,devId) != EVENTId_Error_NoError)
	{
		return EVENTId_Error_SessionNotExist;
	}

	TDeviceInfoQueryResp resp;
	resp.strDeviceId = devId;
	resp.nSn = messageSn;
	resp.bResult = error.getErrorCode() == SIPStackError_OK ? true : false;
	resp.strManufacturer = info.manufacturer;
	resp.strModel = info.model;
	resp.strFirmware = info.firmWare;
	resp.nMaxAlarm = info.alarmNum;
	resp.nMaxCamera = info.cameraNum;

	string strXml;
	BuildMsg(E_PT_DEVICE_INFO_QUERY_RESP,&resp,strXml);


	return internal->buildAndSendResponse(platformId,id,"MESSAGE",strXml,XMLCONTENTTYPE);
}
EVENTId_Error GBStack::sendQueryRecordInfoReq(GBEventID& eventid,GBPlatformID platformId,const std::string& devid,long long startTime,long long endTime,ERecordInfoType type)
{
	{
		Guard locker(internal->gbmutex);

		shared_ptr<GBPlatformInfo> platfrom = internal->_queryPlatfromById(platformId);
		if (platfrom == NULL)
		{
			return EVENTId_Error_PlatformNotExist;
		}

		if (platfrom->status != GBPlatformInfo::PlatformStatus_Success)
		{
			return EVENTId_Error_NotRegist;
		}
	}
	
	TRecordInfoQueryReq req;

	req.nSn = internal->getRandNum(platformId);
	req.strDeviceId = devid;
	req.eType = type;
	req.strStartTime = SecToSipTime(startTime);
	req.strEndTime = SecToSipTime(endTime);
	req.strRecorderID = devid;
	req.strFilePath = devid;
	req.strAddress = "Address1";
	req.nSecrecy = 0;

	string strXml;
	BuildMsg(E_PT_RECORD_INFO_QUERY_REQ,&req,strXml);
	
	
	return internal->buildAndSendRequest(eventid,platformId,"MESSAGE",GBEventPool::QUERYRECORDINFO_Req,req.nSn,devid,strXml,XMLCONTENTTYPE);
}
EVENTId_Error GBStack::sendQueryRecordInfoResp(GBPlatformID platformId,GBEventID id, const SIPStackError& error,const std::string& name,uint32_t total,bool isEnd,const std::list<TRecordItem>& resp)
{
	uint32_t maxsendSize = MAXUDPNUMPERPACKET;
	{
		Guard locker(internal->gbmutex);

		shared_ptr<GBPlatformInfo> platfrom = internal->_queryPlatfromById(platformId);
		if (platfrom == NULL)
		{
			return EVENTId_Error_PlatformNotExist;
		}

		if (platfrom->status != GBPlatformInfo::PlatformStatus_Success)
		{
			return EVENTId_Error_NotRegist;
		}

		maxsendSize = platfrom->transaction->getTransType() == SIPTransaction::SIPTransactionType_TCP ? MAXTCPNUMPERPACKET : MAXUDPNUMPERPACKET;
	}	

	int messageSn;
	std::string devId;

	if(internal->getMessageInfo(platformId,id,messageSn,devId) != EVENTId_Error_NoError)
	{
		return EVENTId_Error_SessionNotExist;
	}

	std::list<TRecordItem>::const_iterator respiter = resp.begin();
	do
	{	
		TRecordInfoQueryResp iresp;
		iresp.strDeviceId = devId;
		iresp.nSn = messageSn;
		iresp.strName = name;
		iresp.nSumNum = total;

		for(uint32_t i = 0;i < maxsendSize && respiter != resp.end();i ++,respiter++)
		{
			iresp.vRecordList.push_back(*respiter);
		}

		std::string strXml;
		BuildMsg(E_PT_RECORD_INFO_QUERY_RESP,&iresp,strXml);

		internal->buildAndSendResponse(platformId,id,"MESSAGE",strXml,XMLCONTENTTYPE,respiter != resp.end()?false:isEnd);
	}while(respiter != resp.end());

	
	return EVENTId_Error_NoError;
}
EVENTId_Error GBStack::sendopenRealplayStreamReq(GBEventID& streamID,GBPlatformID platformId,const std::string& devid,int streamType,const StreamSDPInfo& sdp)
{
	{
		Guard locker(internal->gbmutex);

		shared_ptr<GBPlatformInfo> platfrom = internal->_queryPlatfromById(platformId);
		if (platfrom == NULL)
		{
			return EVENTId_Error_PlatformNotExist;
		}

		if (platfrom->status != GBPlatformInfo::PlatformStatus_Success)
		{
			return EVENTId_Error_NotRegist;
		}
	}

	return internal->buildAndSendOpenStreamReq(streamID,platformId,devid,0,0,GBEventPool::CALLType_Realplay,1,streamType,sdp);
}
EVENTId_Error GBStack::sendopenPlaybackStreamReq(GBEventID& streamID,GBPlatformID platformId,const std::string& devid,long long startTime,long long endTime,const StreamSDPInfo& sdp)
{
	{
		Guard locker(internal->gbmutex);

		shared_ptr<GBPlatformInfo> platfrom = internal->_queryPlatfromById(platformId);
		if (platfrom == NULL)
		{
			return EVENTId_Error_PlatformNotExist;
		}

		if (platfrom->status != GBPlatformInfo::PlatformStatus_Success)
		{
			return EVENTId_Error_NotRegist;
		}
	}	

	return internal->buildAndSendOpenStreamReq(streamID,platformId,devid,startTime,endTime,GBEventPool::CALLType_Playback,1,0,sdp);
}
EVENTId_Error GBStack::sendopenDownloadStreamReq(GBEventID& streamID,GBPlatformID platformId,const std::string& devid,long long startTime,long long endTime,int downSpeed,const StreamSDPInfo& sdp)
{
	{
		Guard locker(internal->gbmutex);

		shared_ptr<GBPlatformInfo> platfrom = internal->_queryPlatfromById(platformId);
		if (platfrom == NULL)
		{
			return EVENTId_Error_PlatformNotExist;
		}

		if (platfrom->status != GBPlatformInfo::PlatformStatus_Success)
		{
			return EVENTId_Error_NotRegist;
		}
	}	

	return internal->buildAndSendOpenStreamReq(streamID,platformId,devid,startTime,endTime,GBEventPool::CALLType_Download,downSpeed,0,sdp);
}
EVENTId_Error GBStack::sendOpenStreamResp(GBPlatformID platformId,GBEventID streamID, const SIPStackError& error,const StreamSDPInfo& sdp)
{
	{
		Guard locker(internal->gbmutex);

		shared_ptr<GBPlatformInfo> platfrom = internal->_queryPlatfromById(platformId);
		if (platfrom == NULL)
		{
			return EVENTId_Error_PlatformNotExist;
		}

		if (platfrom->status != GBPlatformInfo::PlatformStatus_Success)
		{
			return EVENTId_Error_NotRegist;
		}
	}	

	return internal->buildAndSendOpenStreamResp(platformId,streamID,error, sdp);
}
// EVENTId_Error GBStack::sendOpenStreamAck(GBPlatformID platformId,GBEventID streamID,const std::string& sendip,int sendport)
// {
// 	Guard locker(internal->gbmutex);
// 								
// 	std::set<GBPlatformInfo*>::iterator piter = internal->platformList.find((GBPlatformInfo*)platformId);
// 	if(piter == internal->platformList.end())
// 	{
// 		return EVENTId_Error_PlatformNotExist;
// 	}
// 	if((*piter)->status != GBPlatformInfo::PlatformStatus_Success)
// 	{
// 		return EVENTId_Error_NotRegist;
// 	}
// 
// 	return internal->buildAndSendOpenStreamAck(platformId,streamID,sendip,sendport);
// }
EVENTId_Error GBStack::sendcloseStreamReq(GBPlatformID platformId,GBEventID streamID)
{
	{
		Guard locker(internal->gbmutex);

		shared_ptr<GBPlatformInfo> platfrom = internal->_queryPlatfromById(platformId);
		if (platfrom == NULL)
		{
			return EVENTId_Error_PlatformNotExist;
		}

		if (platfrom->status != GBPlatformInfo::PlatformStatus_Success)
		{
			return EVENTId_Error_NotRegist;
		}
	}	

	return internal->buildAndSendCloseStreamReq(platformId,streamID);
}
EVENTId_Error GBStack::sendcloseStreamResp(GBPlatformID platformId,GBEventID streamID, const SIPStackError& error)
{
	{
		Guard locker(internal->gbmutex);

		shared_ptr<GBPlatformInfo> platfrom = internal->_queryPlatfromById(platformId);
		if (platfrom == NULL)
		{
			return EVENTId_Error_PlatformNotExist;
		}

		if (platfrom->status != GBPlatformInfo::PlatformStatus_Success)
		{
			return EVENTId_Error_NotRegist;
		}
	}	

	return internal->buildAndSendCloseStreamResp(platformId,streamID);
}
EVENTId_Error GBStack::sendPlaybackContrlReq(GBEventID& eventId,GBPlatformID platformId,GBEventID streamID,float speed,long long time)
{
	{
		Guard locker(internal->gbmutex);

		shared_ptr<GBPlatformInfo> platfrom = internal->_queryPlatfromById(platformId);
		if (platfrom == NULL)
		{
			return EVENTId_Error_PlatformNotExist;
		}

		if (platfrom->status != GBPlatformInfo::PlatformStatus_Success)
		{
			return EVENTId_Error_NotRegist;
		}
	}	
	
	TVodCtrlReq req;

	req.nSn = internal->getRandNum(platformId);
	req.eType = (speed == 0) ? E_RT_PAUSE : E_RT_PLAY;
	req.dSpeed = speed;
	req.lStartTime = (long)time;

	string strXml;
	BuildMsg(E_PT_VOD_CTRL_REQ,&req,strXml);

	return internal->buildAndSendPlayCtrlReq(eventId,platformId,streamID,req.nSn,strXml,XMLRTSPTYPE);
}

EVENTId_Error GBStack::sendPlaybackContrlResp(GBPlatformID platformId,GBEventID eventId, const SIPStackError& error,long rtpseq,long rtpTime)
{
	{
		Guard locker(internal->gbmutex);

		shared_ptr<GBPlatformInfo> platfrom = internal->_queryPlatfromById(platformId);
		if (platfrom == NULL)
		{
			return EVENTId_Error_PlatformNotExist;
		}

		if (platfrom->status != GBPlatformInfo::PlatformStatus_Success)
		{
			return EVENTId_Error_NotRegist;
		}
	}	

	int messageSn;
	std::string devId;

	if(internal->getMessageInfo(platformId,eventId,messageSn,devId) != EVENTId_Error_NoError)
	{
		return EVENTId_Error_SessionNotExist;
	}

	TVodCtrlResp iresp;
	iresp.nSn = messageSn;
	iresp.nResult = (error.getErrorCode() == SIPStackError_OK)?true:false;
	iresp.rtpinfo_seq = rtpseq;
	iresp.rtpinfo_time = rtpTime;

	string strXml;
	BuildMsg(E_PT_VOD_CTRL_RESP,&iresp,strXml);

	return internal->buildAndSendPlayCtrlResp(platformId,eventId,error,strXml,XMLRTSPTYPE);
}
EVENTId_Error GBStack::sendMediaStatus(GBPlatformID platformId,GBEventID streamID,MediaSatus status)
{
	{
		Guard locker(internal->gbmutex);

		shared_ptr<GBPlatformInfo> platfrom = internal->_queryPlatfromById(platformId);
		if (platfrom == NULL)
		{
			return EVENTId_Error_PlatformNotExist;
		}

		if (platfrom->status != GBPlatformInfo::PlatformStatus_Success)
		{
			return EVENTId_Error_NotRegist;
		}
	}	

	MediaStatusNotify req;

	req.nSn = internal->getRandNum(platformId);
	req.nType = status;
	req.strDeviceId = internal->getDeviceId(platformId,streamID);

	string strXml;
	BuildMsg(E_PT_MEDIASTATUS_NOTIFY,&req,strXml);

	return internal->buildAndSendMediaStatus(platformId,streamID,req.nSn,strXml,XMLCONTENTTYPE);
}
EVENTId_Error GBStack::getCommunicationInfos(CommunicationInfo&info,GBPlatformID platformId)
{
	Guard locker(internal->gbmutex);
	
	shared_ptr<GBPlatformInfo> platfrom = internal->_queryPlatfromById(platformId);
	if(platfrom == NULL)
	{
		return EVENTId_Error_PlatformNotExist;
	}

	info.type = (CommunicationInfo::_commutype)platfrom->transaction->getTransType();
	info.Port = platfrom->transaction->getMyPort();
	info.IpAddr = platfrom->myIp;

	return EVENTId_Error_NoError;
}

GB28181TYPE GBStack::getGB28181Type(const std::string& id)
{
	CPublicID pid(id);

	return (GB28181TYPE)pid.GetSvrType();
}
};
};

