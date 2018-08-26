#include "GBRegister.h"
#include "Network/Network.h"
#include "Base/Base.h"
#include "GBStackInternal.h"
using namespace Public::Network;
using namespace Public::Base;
namespace Public{
namespace GB28181{

GBRegister::GBRegister(const weak_ptr<GBStack>& gb, const weak_ptr<GBDispacher>& dis,const weak_ptr<GBPlatformInfo>& platinfo,const CuttoffCallback& callback)
{
	registerSession = NULL;
	gbstack = gb;
	dispacher = dis;
	platformInfo = platinfo;
	offlineCalblack = callback;

	isStartRegist = false;
	registerStatus = SIPStackError_NotRegister;

	registerSession = NULL;

	prevRegTime = 0;
	prevKeepAliveTime = 0;
	prevRegSuccTime = 0;
	prevKeepAliveSuccTime = 0;
	
	poolTimer = make_shared<Timer>("GBRegister");
	poolTimer->start(Timer::Proc(&GBRegister::timerProc,this),0,1000);
}
GBRegister::~GBRegister()
{
	poolTimer = NULL;
	shared_ptr<GBPlatformInfo> platptr = platformInfo.lock();
	if(registerStatus == SIPStackError_OK && platptr != NULL && platptr->type == GBPlatformInfo::PlatformType_Up)
	{
		sendRegister(0);

		int times = 0;
		while(registerStatus == SIPStackError_OK && times++ <= 100)
		{
			Thread::sleep(10);
		}
		Thread::sleep(500);
	}
}
void GBRegister::regist(int _expries)
{
	initExpries = _expries;
	registerStatus = SIPStackError_NotRegister;
	isStartRegist = true;

}
void GBRegister::sendRegister(int expries)
{
	initExpries = expries;
	if(registerSession != NULL)
	{
		if(expries == 0)
		{
			registerSession = make_shared<SIPSession>("REGISTER",getMyUrl(),getToUrl(),platformInfo.lock()->transaction.get());
		}
		registerSession->setExpries(expries);
		registerSession->sendRequest();

		return;
	}

	registerSession = make_shared<SIPSession>("REGISTER",getMyUrl(),getToUrl(),platformInfo.lock()->transaction.get());
	registerSession->setExpries(expries);
	registerSession->sendRequest();
}
void GBRegister::sendKeepAlive()
{
	SIPSession session("MESSAGE",getMyUrl(),getToUrl(),platformInfo.lock()->transaction.get());

	TKeepAliveNotify Alive;
	Alive.nSn = 0;
	Alive.strDeviceId = platformInfo.lock()->myid;
	Alive.bStatus = true;

	string strXml;
	BuildMsg(E_PT_KEEP_ALIVE_NOTIFY,&Alive,strXml);

	if(registerSession != NULL)
	{
		session.cloneSession(*registerSession.get());
	}

	session.sendRequest(strXml,XMLCONTENTTYPE);
}

void GBRegister::setSession(shared_ptr<SIPSession>& session)
{
	registerSession = session;

	registerStatus = SIPStackError_OK;

	prevRegSuccTime = Time::getCurrentMilliSecond();
	prevKeepAliveSuccTime = Time::getCurrentMilliSecond();
	regExpries = session->getExpries();
}
void GBRegister::setAddResiterAck(const SIPStackError& error,unsigned long long dateTime)
{
	if(error.getErrorCode() == SIPStackError_OK && registerSession != NULL)
	{
		registerSession->sendSuccess("","",SecToSipRegTime(dateTime));
		
		isStartRegist = true;
		prevRegSuccTime = Time::getCurrentMilliSecond();
		prevKeepAliveSuccTime = Time::getCurrentMilliSecond();
		regExpries = registerSession->getExpries();
		registerStatus = SIPStackError_OK;
	}
	else if(error.getErrorCode() != SIPStackError_OK && registerSession != NULL)
	{
		registerSession->sendFailure(error);
		registerStatus = SIPStackError_NoSuport;
	}
}
void GBRegister::recv2xx(shared_ptr<SIPSession>& session)
{
	if(initExpries == 0)
	{
		return;
	}
	
	if (strcasecmp(session->getMethod().c_str(), "REGISTER") == 0)
	{
		prevRegSuccTime = Time::getCurrentMilliSecond();
		prevKeepAliveSuccTime = Time::getCurrentMilliSecond();
	}

	SIPStackErrorCode oldstatus = registerStatus;

	registerStatus = SIPStackError_OK;
	
	shared_ptr<GBDispacher> dispatcherptr = dispacher.lock();
	shared_ptr<GBPlatformInfo> platfrom = platformInfo.lock();
	if(oldstatus != SIPStackError_OK && dispatcherptr != NULL && platfrom != NULL)
	{
		RegisterInfo info;
		SIPTransport transport = session->getTransport();

		info.authenIp = platfrom->authenIp;
		info.fromIp = platfrom->fromIp;
		info.toIp = platfrom->toIp;
		info.requestIp = platfrom->requestIp;
		info.platformId = session->getFromUrl().getUserName();
		info.platformIp = transport.getOtherAddr().getIP();
		info.platformPort = transport.getOtherAddr().getPort();
		info.mySipAddr = platfrom->myIp;
		info.mySipPort = session->getTransaction()->getMyPort();
		info.dateTime = SipTimeToSec(session->getDateTime());
		info.socketType = platfrom->transaction->getTransType() == SIPTransaction::SIPTransactionType_TCP ? SocketType_TCP : SocketType_UDP;

		dispatcherptr->onRegistResp(gbstack, platfrom.get(),SIPStackError_OK,info);
	}
}
void GBRegister::recv3456xx(shared_ptr<SIPSession>& session)
{
	if(session->getError().getErrorCode() == SIPStackError_UnAuthenticate)
	{
		session->sendAuthenticate(platformInfo.lock()->usrename,platformInfo.lock()->passwd,initExpries);
		if(initExpries == 0)
		{
			registerStatus = SIPStackError_NotRegister;
		}
		return;
	}

	SIPStackErrorCode oldstatus = registerStatus;

	registerStatus = SIPStackError_NoSuport;
	
	shared_ptr<GBDispacher> dispatcherptr = dispacher.lock();
	shared_ptr<GBPlatformInfo> platfrom = platformInfo.lock();
	if(oldstatus == SIPStackError_OK && dispatcherptr != NULL && platfrom != NULL)
	{
		RegisterInfo info;
		info.authenIp = platfrom->authenIp;
		info.fromIp = platfrom->fromIp;
		info.toIp = platfrom->toIp;
		info.requestIp = platfrom->requestIp;
		info.mySipAddr = platfrom->myIp;
		info.mySipPort = session->getTransaction()->getMyPort();
		info.socketType = platfrom->transaction->getTransType() == SIPTransaction::SIPTransactionType_TCP ? SocketType_TCP : SocketType_UDP;

		dispatcherptr->onRegistResp(gbstack, platfrom.get(),session->getError(),info);
	}
}
void GBRegister::recvkeepalive(shared_ptr<SIPSession>& session)
{
	prevKeepAliveSuccTime = Time::getCurrentMilliSecond();

	session->sendSuccess();
}
void GBRegister::recvkeepalive2xx(shared_ptr<SIPSession>& session)
{
	prevKeepAliveSuccTime = Time::getCurrentMilliSecond();
}

void GBRegister::timerProc(unsigned long param)
{
	if(!isStartRegist)
	{
		return;
	}

	uint64_t nowtime = Time::getCurrentMilliSecond();
	int regterInterval = platformInfo.lock()->type == GBPlatformInfo::PlatformType_Up ? initExpries : regExpries;

	SIPStackErrorCode oldstatus = registerStatus;
	
	if(registerStatus == SIPStackError_OK && (
		((int)(nowtime - prevRegSuccTime) > regterInterval * 1000) ||
		((int)(nowtime - prevKeepAliveSuccTime) > regterInterval * 1000)))
	{
		registerStatus = SIPStackError_NotRegister;
	}
	
	if(platformInfo.lock()->type == GBPlatformInfo::PlatformType_Up)
	{
		int timeouttime = (initExpries * 1000) /(registerStatus == SIPStackError_OK ? 3 : 2);
		if(timeouttime > 30000)
		{
			timeouttime = 30000;
		}
		
		if(nowtime - prevRegTime > (uint64_t)((initExpries * 1000) / 2))
		{
			sendRegister(initExpries);
			prevRegTime = nowtime;
		}
	
		if(registerStatus == SIPStackError_OK && nowtime - prevKeepAliveTime > (uint64_t)timeouttime)
		{
			sendKeepAlive();
			prevKeepAliveTime = nowtime;			
		}
	}

	shared_ptr<GBDispacher> dispatcherptr = dispacher.lock();
	if(oldstatus !=  registerStatus && registerStatus != SIPStackError_OK && dispatcherptr != NULL)
	{
		dispatcherptr->onRegistClosed(gbstack,platformInfo.lock().get(),platformInfo.lock().get(),CloseRegistError_TimeOut);
		offlineCalblack(platformInfo.lock().get(),false);
	}
}

SIPUrl GBRegister::getMyUrl()const
{
	return SIPUrl(platformInfo.lock()->myid,NetAddr(platformInfo.lock()->myIp,platformInfo.lock()->transaction->getMyPort()));
}
SIPUrl GBRegister::getToUrl(const std::string&  id) const
{
	if(platformInfo.lock()->type == GBPlatformInfo::PlatformType_Up)
	{
		return SIPUrl(id == "" ? platformInfo.lock()->clientId : id, platformInfo.lock()->clientIp,platformInfo.lock()->clientPort);
	}
	else
	{
		SIPTransport trans = registerSession->getTransport();
		return SIPUrl(id == "" ? platformInfo.lock()->clientId : id, trans.getOtherAddr());
	}
}
shared_ptr<SIPTransaction> GBRegister::getTransaction() const
{
	return platformInfo.lock()->transaction;
}
};
};
