#include "GBStackInternal.h"
#include "GBEventPool.h"
#ifndef WIN32
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#endif

using namespace Public::Network;
#define SIPCOMDMAXTIMEOUT  10000
namespace Public{
namespace GB28181{

#ifdef WIN32
#include<winsock.h>
	static bool networkInitial()
	{
		WSADATA wsaData;
		WORD wVersionRequested;

		// Need Request the  Windows Sockets specification v2.2
		wVersionRequested = MAKEWORD(2, 2);
		int errorCode = WSAStartup(wVersionRequested, &wsaData);
		if (errorCode != 0)
		{
			logerror("NetFrame WSAStartup failed!\n");
			return false;
		}

		//check version...
		if (LOBYTE(wsaData.wVersion) != 2
			|| HIBYTE(wsaData.wVersion) != 2)
		{
			logerror("NetFrame Ck Windows Sockets specification failed!\n");
			WSACleanup();
			return false;
		}

		return true;
	}
#define closesock closesocket
#else
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
	static bool networkInitial()
	{
		return true;
	}
#define closesock close
#endif

GBStackInternal::GBStackInternal()
{
}

GBStackInternal::~GBStackInternal()
{
	std::map<SIPTransport, shared_ptr<GBPlatformInfo> > leaveList;
	{
		workTimer = NULL;
		
		Guard locker(gbmutex);
		leaveList = platformTransportMap;

		cutoffPlatList.clear();
		cutoffComList.clear();
		platformTransportMap.clear();
		communicationList.clear();
	}
	for(std::map<SIPTransport, shared_ptr<GBPlatformInfo> >::iterator iter = leaveList.begin();iter != leaveList.end();iter ++)
	{
		deletePlatform(iter->second);
	}
}
void GBStackInternal::init(const weak_ptr<GBStack>& stack)
{
	gbstack = stack;
	workTimer = make_shared<Timer>("GBStackInternal");
	workTimer->start(Timer::Proc(&GBStackInternal::timerProc, this), 0, 1000);
}

void GBStackInternal::rcvRegister(shared_ptr<SIPSession>&  session)
{
	if(session->getAuthenicateUserName() == "")
	{
		session->send401Failure(myUser);
		return;
	}

	if(!session->Authenticate(myUser,myPass))
	{
		session->sendFailure(SIPStackError(SIPStackError_PwdError));
		return;
	}

	bool		isHaveNewPlatfrom = false;
	shared_ptr<GBPlatformInfo> newplatform;
	shared_ptr<SIPTransaction> transaction;
	shared_ptr<GBRegister> registe;
	SIPTransport transport = session->getTransport();
	do{
		Guard locker(gbmutex);
		std::map<SIPTransport,shared_ptr<GBPlatformInfo> >::iterator iter = platformTransportMap.find(transport);
		if(iter != platformTransportMap.end() && iter->second->registe != NULL)
		{
			transaction = iter->second->transaction;
			registe = iter->second->registe;
			newplatform = iter->second;
			registe->setSession(session);
			break;
		}	
		
		std::map<GBCommuID,shared_ptr<SIPTransaction> >::iterator citer;
		for(citer = communicationList.begin();citer != communicationList.end();citer ++)
		{
			if(citer->second.get() == session->getTransaction())
			{
				transaction = citer->second;
				break;
			}
		}

		if(transaction == NULL)
		{
			break;
		}

		newplatform = make_shared<GBPlatformInfo>();
		newplatform->transport = session->getTransport();
		newplatform->myid = myId;
		newplatform->myIp = session->getMyUrl(myId).getHost();
		newplatform->myusername = myUser;
		newplatform->mypasswd = myPass;
		newplatform->clientId= session->getFromUrl().getUserName();
		newplatform->clientPort = newplatform->transport.getOtherAddr().getPort();

		newplatform->clientIp = newplatform->transport.getOtherAddr().getIP();
		newplatform->authenIp = session->getAuthenticateUri().getHost();
		newplatform->fromIp = session->getFromUrl().getHost();
		newplatform->toIp = session->getToUrl().getHost();
		newplatform->requestIp = session->getRequestUrl().getHost();

		newplatform->status = GBPlatformInfo::PlatformStatus_Authen;
		newplatform->type = GBPlatformInfo::PlatformType_Low;
		newplatform->transaction = transaction;
		
		newplatform->registe = make_shared<GBRegister>(gbstack,dispacher,newplatform,GBRegister::CuttoffCallback(&GBStackInternal::platfromCutoff,this));
		newplatform->registe->setSession(session);
		platformTransportMap[transport] = newplatform;		

		isHaveNewPlatfrom = true;
		registe = newplatform->registe;
	}while(0);

	if (isHaveNewPlatfrom)
	{
		addPlatform(gbstack, dispacher, newplatform);
	}
	
	RegisterInfo info;
	info.authenIp = newplatform->authenIp;
	info.fromIp = newplatform->fromIp;
	info.toIp = newplatform->toIp;
	info.requestIp = newplatform->requestIp;
	info.platformId = session->getFromUrl().getUserName();
	info.platformIp = transport.getOtherAddr().getIP();
	info.platformPort = transport.getOtherAddr().getPort();
	info.mySipAddr = newplatform->myIp;
	info.mySipPort = transaction->getMyPort();
	info.dateTime = SipTimeToSec(session->getDateTime());
	info.socketType = transaction->getTransType() == SIPTransaction::SIPTransactionType_TCP ? SocketType_TCP : SocketType_UDP;

	dispacher.lock()->onRegistReq(gbstack,transaction.get(),newplatform.get(),info);
}
void GBStackInternal::rcv2xx_Register(shared_ptr<SIPSession>& session)
{
	SIPTransport transport = session->getTransport();
	{
		Guard locker(gbmutex);
		std::map<SIPTransport,shared_ptr<GBPlatformInfo> >::iterator iter = platformTransportMap.find(transport);
		if(iter != platformTransportMap.end() && iter->second->registe != NULL && iter->second->type == GBPlatformInfo::PlatformType_Up)
		{
			iter->second->status = GBPlatformInfo::PlatformStatus_Success;
			iter->second->registe->recv2xx(session);
			iter->second->transport.setSock(session->getTransport().getSock());
			return;
		}		
	}
}
void GBStackInternal::rcv3456xx_Register(shared_ptr<SIPSession>& session)
{
	SIPTransport transport = session->getTransport();
	{
		Guard locker(gbmutex);
		std::map<SIPTransport,shared_ptr<GBPlatformInfo> >::iterator iter = platformTransportMap.find(transport);
		if(iter != platformTransportMap.end() && iter->second->registe != NULL && iter->second->type == GBPlatformInfo::PlatformType_Up)
		{
			iter->second->registe->recv3456xx(session);
			return;
		}
	}
}
void GBStackInternal::rcv2xx_Info(shared_ptr<SIPSession>& session)
{
	rcv3456xx_Message(session);
}
void GBStackInternal::rcv3456xx_Info(shared_ptr<SIPSession>& session)
{
	rcv3456xx_Message(session);
}
void GBStackInternal::rcv2xx_Message(shared_ptr<SIPSession>& session)
{
	rcv3456xx_Message(session);
}
void GBStackInternal::rcv3456xx_Notify(shared_ptr<SIPSession>& session)
{
	rcv3456xx_Message(session);
}
void GBStackInternal::rcv2xx_Notify(shared_ptr<SIPSession>& session)
{
	rcv3456xx_Message(session);
}
void GBStackInternal::rcv2xx(shared_ptr<SIPSession>& session)
{
	rcv3456xx_Message(session);
}
void GBStackInternal::rcv3456xx_Message(shared_ptr<SIPSession>& session)
{
	GBPlatformInfo* platform = NULL;
	SIPTransport transport = session->getTransport();
	{
		Guard locker(gbmutex);
		std::map<SIPTransport,shared_ptr<GBPlatformInfo> >::iterator iter = platformTransportMap.find(transport);
		if(iter == platformTransportMap.end())
		{
			return;
		}
		platform = iter->second.get();
		
	}
	if(platform != NULL)
	{
		procRespMessageError(session,platform);
	}
}
void GBStackInternal::rcvSubscribe(shared_ptr<SIPSession>& session)
{
	rcvMessage(session);
}

void GBStackInternal::rcvMessage(shared_ptr<SIPSession>& session)
{
	GBPlatformInfo* platform = NULL;
	SIPTransport transport = session->getTransport();
	{
		Guard locker(gbmutex);
		std::map<SIPTransport,shared_ptr<GBPlatformInfo> >::iterator iter = platformTransportMap.find(transport);
		if(iter != platformTransportMap.end())
		{
			platform = iter->second.get();
		}
	}
	if(platform == NULL)
	{
		session->sendFailure(SIPStackError(SIPStackError_NotRegister));
		return;
	}
	
	GBEventPool::rcvMessage(platform,session);
}

void GBStackInternal::rcvInvite(shared_ptr<SIPSession>& session)
{
	GBPlatformInfo* platform = NULL;
	SIPTransport transport = session->getTransport();
	{
		Guard locker(gbmutex);
		std::map<SIPTransport,shared_ptr<GBPlatformInfo> >::iterator iter = platformTransportMap.find(transport);
		if(iter != platformTransportMap.end())
		{
			platform = iter->second.get();
		}
	}
	if(platform == NULL)
	{
		session->sendFailure(SIPStackError(SIPStackError_NotRegister));
	}
	else
	{
		session->sendProceeding();

		recvInviteReq(platform,session);
	}
}
void GBStackInternal::rcv2xx_Invite(shared_ptr<SIPSession>& session)
{
	GBPlatformInfo* platform = NULL;
	SIPTransport transport = session->getTransport();
	{
		Guard locker(gbmutex);
		std::map<SIPTransport,shared_ptr<GBPlatformInfo> >::iterator iter = platformTransportMap.find(transport);
		if(iter == platformTransportMap.end())
		{
			return;
		}
		platform = iter->second.get();
	}

	recvInviteResp(platform,session);
}
void GBStackInternal::rcvAck(shared_ptr<SIPSession>& session)
{
	GBPlatformInfo* platform = NULL;
	SIPTransport transport = session->getTransport();
	{
		Guard locker(gbmutex);
		std::map<SIPTransport,shared_ptr<GBPlatformInfo> >::iterator iter = platformTransportMap.find(transport);
		if(iter == platformTransportMap.end())
		{
			return;
		}
		platform = iter->second.get();
	}

	recvInviteAck(platform,session);
}
void GBStackInternal::rcvBye(shared_ptr<SIPSession>& session)
{
	GBPlatformInfo* platform = NULL;
	SIPTransport transport = session->getTransport();
	{
		Guard locker(gbmutex);
		std::map<SIPTransport,shared_ptr<GBPlatformInfo> >::iterator iter = platformTransportMap.find(transport);
		if(iter != platformTransportMap.end())
		{
			platform = iter->second.get();
		}
	}
	if(platform == NULL)
	{
		session->sendFailure(SIPStackError(SIPStackError_NotRegister));
	}
	else
	{
		recvInviteBye(platform,session);
	}
}
void GBStackInternal::rcvCancel(shared_ptr<SIPSession>& session)
{
	rcvBye(session);
}
void GBStackInternal::rcvNotify(shared_ptr<SIPSession>& session)
{
	rcvMessage(session);
}
void GBStackInternal::rcvInfo(shared_ptr<SIPSession>& session)
{
	rcvMessage(session);
}
void GBStackInternal::recvUpdate(shared_ptr<SIPSession>& session)
{
	GBPlatformInfo* platform = NULL;
	SIPTransport transport = session->getTransport();
	{
		Guard locker(gbmutex);
		std::map<SIPTransport,shared_ptr<GBPlatformInfo> >::iterator iter = platformTransportMap.find(transport);
		if(iter == platformTransportMap.end())
		{
			return;
		}
		platform = iter->second.get();
	}

	recvUpdateReq(platform,session);
}
void GBStackInternal::rcv2xx_Update(shared_ptr<SIPSession>& session)
{
	GBPlatformInfo* platform = NULL;
	SIPTransport transport = session->getTransport();
	{
		Guard locker(gbmutex);
		std::map<SIPTransport,shared_ptr<GBPlatformInfo> >::iterator iter = platformTransportMap.find(transport);
		if(iter == platformTransportMap.end())
		{
			return;
		}
		platform = iter->second.get();
	}

	recvUpdateResp(platform,session);
}
void GBStackInternal::rcv3456xx_Invite(shared_ptr<SIPSession>& session)
{
	GBPlatformInfo* platform = NULL;
	SIPTransport transport = session->getTransport();
	{
		Guard locker(gbmutex);
		std::map<SIPTransport,shared_ptr<GBPlatformInfo> >::iterator iter = platformTransportMap.find(transport);
		if(iter == platformTransportMap.end())
		{
			return;
		}
		platform = iter->second.get();
	}

	recvInviteResp(platform,session);
}

int GBStackInternal::getRandNum(GBPlatformID platformId)
{
	return getMessageSN(platformId);
}

void GBStackInternal::platfromCutoff(GBPlatformID platform,bool force)
{
	shared_ptr<GBPlatformInfo> platfromptr;
	{
		Guard locker(gbmutex);
		shared_ptr<GBPlatformInfo> pt =  _queryPlatfromById(platform);
		if(pt == NULL)
		{
			return;
		}
		
		std::map<SIPTransport,shared_ptr<GBPlatformInfo> >::iterator iter = platformTransportMap.find(pt->transport);
		if(iter == platformTransportMap.end() || (iter->second->type == GBPlatformInfo::PlatformType_Up && !force))
		{
			return;
		}
		platfromptr = iter->second;
		cutoffPlatList.push_back(pt);
		platformTransportMap.erase(iter);
	}

	deletePlatform(platfromptr);
}
shared_ptr<GBPlatformInfo> GBStackInternal::_queryPlatfromById(GBPlatformID platid)
{
	std::map<SIPTransport,shared_ptr<GBPlatformInfo> >::iterator iter;
	for(iter = platformTransportMap.begin();iter != platformTransportMap.end();iter ++)
	{
		if(iter->second.get() == platid)
		{
			return iter->second;
		}
	}
	
	return shared_ptr<GBPlatformInfo>();
}

void GBStackInternal::timerProc(unsigned long param)
{
	std::list<shared_ptr<GBPlatformInfo> >	freeplatList;
	std::list<shared_ptr<SIPTransaction> >   freecomlist;
	{
		Guard locker(gbmutex);

		freecomlist = cutoffComList;
		cutoffComList.clear();

		for(std::list<shared_ptr<SIPTransaction> >::iterator citer = freecomlist.begin();citer != freecomlist.end();citer ++)
		{
			for(std::map<SIPTransport,shared_ptr<GBPlatformInfo> >::iterator iter = platformTransportMap.begin();iter != platformTransportMap.end();)
			{
				if(iter->second->transaction.get() == (*citer).get())
				{
					cutoffPlatList.push_back(iter->second);
					platformTransportMap.erase(iter++);
				}
				else
				{
					iter ++;
				}
			}
		}	

		for(std::map<SIPTransport,shared_ptr<GBPlatformInfo> >::iterator iter = platformTransportMap.begin();iter != platformTransportMap.end();)
		{
			if(iter->second->transport.getSock() != 0 && !iter->second->transaction->checkSocketInAlive(iter->second->transport.getSock()))
			{
				cutoffPlatList.push_back(iter->second);	
				platformTransportMap.erase(iter++);
			}
			else
			{
				iter ++;
			}
		}
		
		freeplatList = cutoffPlatList;
		cutoffPlatList.clear();
	}
	for(std::list<shared_ptr<GBPlatformInfo> >::iterator iter = freeplatList.begin();iter != freeplatList.end();iter ++)
	{
		deletePlatform(*iter);
		(*iter)->registe = NULL;
		dispacher.lock()->onRegistClosed(gbstack,(GBCommuID)(*iter)->transaction.get(),(GBPlatformID)(*iter).get(),CloseRegistError_Closed);
	}
	freeplatList.clear();
	freecomlist.clear();
}

};
};
