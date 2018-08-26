#include "eXosip2/eX_setup.h"
#include "SIPStack/SIPTransaction.h"
//#define SIPUSETHREAD
namespace Public{
namespace SIPStack {

struct SIPTransaction::SIPTransactionInternal:public Thread
{
	struct SipMsgProcItem
	{
		SipMsgProcItem(shared_ptr<SIPSession>& sec,Function1<void,shared_ptr<SIPSession>&> _func):session(sec),func(_func){}
		shared_ptr<SIPSession>			session;
		Function1<void,shared_ptr<SIPSession>&> 	func;
	};
	void doSipMessageDiapacherProc(void* _item)
	{
		SipMsgProcItem* item = (SipMsgProcItem*)_item;
		if(item == NULL)
		{
			return;
		}
		item->func(item->session);
		SAFE_DELETE(item);
	}
	void doSipMsgProc(SipMsgProcItem* item)
	{
#ifdef SIPUSETHREAD	
		ThreadPool::ThreadPoolProc proc(&SIPTransactionInternal::doSipMessageDiapacherProc,this);
		gThreadPool.dispach(proc,item);
#else
		item->func(item->session);
		SAFE_DELETE(item);
#endif
	}
#define execute(event,exosip,transa,flag,func) do{\
	shared_ptr<SIPDispatcher> dispatcherptr = dispatcher.lock();\
	if(dispatcherptr != NULL){\
	shared_ptr<SIPSession> session(new SIPSession(event,exosip,transa,flag));\
	if(session != NULL && session->isVaild()){\
	SipMsgProcItem* item = new SipMsgProcItem(session,Function1<void,shared_ptr<SIPSession>&>(&SIPDispatcher::func, dispatcherptr.get()));\
	doSipMsgProc(item);\
	}else{}\
	};\
	}while(0);

#define executeFailue(event,exosip,transa) do{\
	if(MSG_IS_INVITE(event->request))	{execute(event,exosip,transa,SIPSession::MessageType_Responce,rcv3456xx_Invite);}\
	else if(MSG_IS_REGISTER(event->request))	{execute(event,exosip,transa,SIPSession::MessageType_Responce,rcv3456xx_Register);}\
	else if(MSG_IS_BYE(event->request))	{execute(event,exosip,transa,SIPSession::MessageType_Responce,rcv3456xx_Bye);}\
	else if(MSG_IS_CANCEL(event->request))	{execute(event,exosip,transa,SIPSession::MessageType_Responce,rcv3456xx_Cancel);}\
	else if(MSG_IS_INFO(event->request))	{execute(event,exosip,transa,SIPSession::MessageType_Responce,rcv3456xx_Info);}\
	else if(MSG_IS_OPTIONS(event->request))	{execute(event,exosip,transa,SIPSession::MessageType_Responce,rcv3456xx_Options);}\
	else if(MSG_IS_NOTIFY(event->request))	{execute(event,exosip,transa,SIPSession::MessageType_Responce,rcv3456xx_Notify);}\
	else if(MSG_IS_MESSAGE(event->request))	{execute(event,exosip,transa,SIPSession::MessageType_Responce,rcv3456xx_Message);}\
	else {execute(event,exosip,transa,SIPSession::MessageType_Responce,rcv3456xx);}\
	}while(0);

#define executeSuccess(event,exosip,transa) do{\
	if(MSG_IS_INVITE(event->request))	{execute(event,exosip,transa,SIPSession::MessageType_Responce,rcv2xx_Invite);}\
	else if(MSG_IS_REGISTER(event->request))	{execute(event,exosip,transa,SIPSession::MessageType_Responce,rcv3456xx_Register);}\
	else if(MSG_IS_BYE(event->request))	{execute(event,exosip,transa,SIPSession::MessageType_Responce,rcv2xx_Bye);}\
	else if(MSG_IS_CANCEL(event->request))	{execute(event,exosip,transa,SIPSession::MessageType_Responce,rcv2xx_Cancel);}\
	else if(MSG_IS_INFO(event->request))	{execute(event,exosip,transa,SIPSession::MessageType_Responce,rcv2xx_Info);}\
	else if(MSG_IS_OPTIONS(event->request))	{execute(event,exosip,transa,SIPSession::MessageType_Responce,rcv2xx_Options);}\
	else if(MSG_IS_NOTIFY(event->request))	{execute(event,exosip,transa,SIPSession::MessageType_Responce,rcv2xx_Notify);}\
	else if(MSG_IS_MESSAGE(event->request))	{execute(event,exosip,transa,SIPSession::MessageType_Responce,rcv2xx_Message);}\
	else {execute(event,exosip,transa,SIPSession::MessageType_Responce,rcv2xx);}\
	}while(0);
#define executeNewMessage(event,exosip,transa) do{\
	if(MSG_IS_REGISTER(event->request))	{execute(event,exosip,transa,SIPSession::MessageType_Request,rcvRegister);}\
	else if(MSG_IS_OPTIONS(event->request))	{execute(event,exosip,transa,SIPSession::MessageType_Request,rcvInfo);}\
	else if(MSG_IS_NOTIFY(event->request))	{execute(event,exosip,transa,SIPSession::MessageType_Request,rcvNotify);}\
	else if(MSG_IS_MESSAGE(event->request))	{execute(event,exosip,transa,SIPSession::MessageType_Request,rcvMessage);}\
	else {execute(event,exosip,transa,SIPSession::MessageType_Request,rcvrequest);}\
	}while(0);
public:
	NetAddr					myAddr;
	SIPTransactionType		type;

	eXosip_t*				osip;
	weak_ptr<SIPDispatcher> dispatcher;
	SIPTransaction*			transaction;
public:
	SIPTransactionInternal(SIPTransactionType _type,const NetAddr& _myaddr,const std::string& uAgent,const weak_ptr<SIPDispatcher>& _dispatcher,SIPTransaction* _trans)
		:Thread("SIPTransactionInternal"), myAddr(_myaddr), type(_type),dispatcher(_dispatcher),transaction(_trans)
	{
		osip = eXosip_malloc();
		if (!eXosip_init(osip) == OSIP_SUCCESS)
		{
			logdebug("eXosip Stack Init Faiulre|| %s %d\r\n", __FILE__, __LINE__);
		}
		if(uAgent.length() > 0)
		{
			eXosip_set_user_agent(osip,uAgent.c_str());
		}		

		eXosip_listen_addr(osip,type == SIPTransactionType_UDP ? IPPROTO_UDP : IPPROTO_TCP,myAddr.getIP().c_str(),myAddr.getPort(),AF_INET,0);
		
		createThread();
	}
	~SIPTransactionInternal()
	{
		cancelThread();
		eXosip_quit(osip);
		Thread::sleep(1000);
		osip_free(osip);
		osip = NULL;
	}
	void threadProc()
	{
		while(looping())
		{
			eXosip_event_t* pSipEvt = eXosip_event_wait(osip,0,100);
			if(pSipEvt == NULL)
			{
				continue;
			}
			doEventProc(pSipEvt);

			eXosip_lock(osip);
			eXosip_default_action(osip,pSipEvt);
			eXosip_event_free(pSipEvt);
			eXosip_unlock(osip);
		}
	}
	
	void doEventProc(eXosip_event_t* event)
	{
		switch(event->type)
		{
		case EXOSIP_REGISTRATION_SUCCESS:
			execute(event,osip,transaction,SIPSession::MessageType_Responce,rcv2xx_Register);
			break;
		case EXOSIP_REGISTRATION_FAILURE:
			execute(event,osip,transaction,SIPSession::MessageType_Responce,rcv3456xx_Register);
			break;
		case EXOSIP_CALL_INVITE:
		case EXOSIP_CALL_REINVITE:
			execute(event,osip,transaction,SIPSession::MessageType_Request,rcvInvite);
			break;
		case EXOSIP_CALL_NOANSWER:
			execute(event,osip,transaction,SIPSession::MessageType_Responce,rcv3456xx_Invite);
			break;
		case EXOSIP_CALL_ANSWERED:
			execute(event,osip,transaction,SIPSession::MessageType_Responce,rcv2xx_Invite);
			break;
		case EXOSIP_CALL_REQUESTFAILURE:
		case EXOSIP_CALL_SERVERFAILURE:
		case EXOSIP_CALL_GLOBALFAILURE:
			execute(event,osip,transaction,SIPSession::MessageType_Responce,rcv3456xx_Invite);
			break;
		case EXOSIP_CALL_ACK:
			execute(event,osip,transaction,SIPSession::MessageType_Ack,rcvAck);
			break;
		case EXOSIP_CALL_MESSAGE_NEW:
			if(strcasecmp(event->request->sip_method,"update") == 0)
			{
				execute(event,osip,transaction,SIPSession::MessageType_Request,rcvUpdate);
			}
			else
			{
				execute(event,osip,transaction,SIPSession::MessageType_Request,rcvInfo);
			}
			break;
		case EXOSIP_CALL_MESSAGE_ANSWERED:
			if(strcasecmp(event->request->sip_method,"update") == 0)
			{
				execute(event,osip,transaction,SIPSession::MessageType_Responce,rcv2xx_Update);
			}
			else
			{
				execute(event,osip,transaction,SIPSession::MessageType_Responce,rcv2xx_Info);
			}
			break;
		case EXOSIP_CALL_MESSAGE_REDIRECTED:
		case EXOSIP_CALL_MESSAGE_REQUESTFAILURE:
		case EXOSIP_CALL_MESSAGE_SERVERFAILURE:
		case EXOSIP_CALL_MESSAGE_GLOBALFAILURE:
			if(strcasecmp(event->request->sip_method,"update") == 0)
			{
				execute(event,osip,transaction,SIPSession::MessageType_Responce,rcv3456xx_Update);
			}
			else
			{
				execute(event,osip,transaction,SIPSession::MessageType_Responce,rcv3456xx_Info);
			}
			break;
		case EXOSIP_CALL_RELEASED:	
		case EXOSIP_CALL_CLOSED:
			execute(event,osip,transaction,SIPSession::MessageType_Request,rcvBye);
			break;
		case EXOSIP_CALL_PROCEEDING:
		case EXOSIP_CALL_MESSAGE_PROCEEDING:
		case EXOSIP_MESSAGE_PROCEEDING:
		case EXOSIP_SUBSCRIPTION_PROCEEDING:
		case EXOSIP_NOTIFICATION_PROCEEDING:
			execute(event,osip,transaction,SIPSession::MessageType_Responce,rcv1xx);
			break;
		case EXOSIP_MESSAGE_ANSWERED:
			executeSuccess(event,osip,transaction);
			break;
		case EXOSIP_MESSAGE_REDIRECTED:
		case EXOSIP_MESSAGE_REQUESTFAILURE:
		case EXOSIP_MESSAGE_SERVERFAILURE:
		case EXOSIP_MESSAGE_GLOBALFAILURE:
			executeFailue(event,osip,transaction);
			break;

		case EXOSIP_MESSAGE_NEW:
			if (strcasecmp(event->request->sip_method, "REGISTER") == 0)
			{
				execute(event, osip, transaction, SIPSession::MessageType_Request, rcvRegister);
			}
			else
			{
				executeNewMessage(event, osip, transaction);
			}
			break;
		case EXOSIP_IN_SUBSCRIPTION_NEW:
			execute(event,osip,transaction,SIPSession::MessageType_Request,rcvSubscribe);
			break;
		case EXOSIP_SUBSCRIPTION_ANSWERED:
			execute(event,osip,transaction,SIPSession::MessageType_Responce,rcv2xx_Subscribe);
			break;
		case EXOSIP_SUBSCRIPTION_REDIRECTED:
		case EXOSIP_SUBSCRIPTION_REQUESTFAILURE:
		case EXOSIP_SUBSCRIPTION_SERVERFAILURE:
		case EXOSIP_SUBSCRIPTION_GLOBALFAILURE:
			execute(event,osip,transaction,SIPSession::MessageType_Responce,rcv3456xx_Subscribe);
			break;
		case EXOSIP_SUBSCRIPTION_NOTIFY:
			execute(event,osip,transaction,SIPSession::MessageType_Request,rcvNotify);
			break;

		case EXOSIP_NOTIFICATION_NOANSWER:
			execute(event,osip,transaction,SIPSession::MessageType_Responce,rcv2xx_Notify);
			break;
		case EXOSIP_NOTIFICATION_REDIRECTED:
		case EXOSIP_NOTIFICATION_REQUESTFAILURE:
		case EXOSIP_NOTIFICATION_SERVERFAILURE:
		case EXOSIP_NOTIFICATION_GLOBALFAILURE:
			execute(event,osip,transaction,SIPSession::MessageType_Responce,rcv3456xx_Notify);
			break;
		default:
			loginfo("%s %d  undo sip event %d\r\n",__FUNCTION__,__LINE__,event->type);
			break;
		}
	}
};

SIPTransaction::SIPTransaction(SIPTransactionType type,uint32_t myport,const std::string& uAgent,const weak_ptr<SIPDispatcher>& dispatcher)
{
	internal = new SIPTransactionInternal(type,NetAddr(myport),uAgent,dispatcher,this);
}
SIPTransaction::~SIPTransaction()
{
	SAFE_DELETE(internal);
}
uint32_t SIPTransaction::getMyPort() const
{
	return internal->myAddr.getPort();
}
SIPTransaction::SIPTransactionType SIPTransaction::getTransType() const
{
	return internal->type;
}
void* SIPTransaction::getExosip() const
{
	return internal->osip;
}

bool SIPTransaction::checkSocketInAlive(int sock)
{
	return eXosip_Socket_IsAlive(internal->osip,sock) == OSIP_SUCCESS;
}

};
};
