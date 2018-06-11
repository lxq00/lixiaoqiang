#include "eXosip2/eXosip.h"
#include "SIPStack/SIPSession.h"
#include "SIPStack/SIPTransaction.h"
#include "SIPAuthenticate.h"
#include "Base/Base.h"
#include "SIPSDP.h"
#ifndef WIN32
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#endif

namespace Public{
namespace SIPStack {

class exOsipGuard
{
public:
	exOsipGuard(eXosip_t* _osip):osip(_osip)
	{
		if(osip != NULL)
		{
			eXosip_lock(osip);
		}
	}
	~exOsipGuard()
	{
		if(osip != NULL)
		{
			eXosip_unlock(osip);
		}
	}
private:
	eXosip_t*	osip;
};

struct SIPSession::SIPSessionInternal
{
	osip_message_t*		sipMsg;
	bool				ack;

	struct{
		int 			rid;
		int 			did;
		int 			tid;
		int				cid;
	}event;

	SIPTransport		transport;
	MessageType			msgtype;
	eXosip_t*			exosip;
	SIPTransaction*		transaction;
	NetAddr				myaddr;
};

SIPSession::SIPSession(void* exosipevent,void* _exosip,void* _trans,MessageType type)
{
	eXosip_event_t* sipevent = (eXosip_event_t*)exosipevent;

	internal = new SIPSessionInternal;
	internal->sipMsg = NULL;
	internal->exosip = (eXosip_t*)_exosip;
	internal->transaction = (SIPTransaction*)_trans;
	internal->msgtype = type;
	internal->event.tid = sipevent->tid;
	internal->event.did = sipevent->did;
	internal->event.rid = sipevent->rid;
	internal->event.cid = sipevent->cid;
	internal->ack = (type == MessageType_Ack);

	if(internal->exosip == NULL || internal->transaction == NULL)
	{
		return;
	}

	osip_message_t* sipmsg = NULL;

	if(type == MessageType_Request)
	{
		sipmsg = sipevent->request;			
	}
	else if (type == MessageType_Responce)
	{
		sipmsg = sipevent->response;
		if(sipmsg == NULL)
		{
			sipmsg = sipevent->request;
			sipmsg->status_code = 408;
		}
	}
	else if(sipevent->ack != NULL)
	{
		sipmsg = sipevent->ack;
	}
	
	if(sipmsg == NULL)
	{
		return;
	}

	if(sipmsg->sip_method != NULL && strcasecmp(sipmsg->sip_method,"SUBSCRIBE") == 0)
	{
		//int b = 0;
	}

	{
		in_addr fromaddr;
		fromaddr.s_addr = sipmsg->sock.addr;
		const char* fromaddrstr = inet_ntoa(fromaddr);
		internal->transport = SIPTransport(NetAddr(fromaddrstr == NULL ? "" : fromaddrstr,sipmsg->sock.port),_trans);
		internal->transport.setSock(sipmsg->sock.sock);
	}

	osip_message_t* newsipmsg = NULL;
	if(osip_message_clone(sipmsg,&newsipmsg) != OSIP_SUCCESS)
	{
		return;
	}
	if(newsipmsg->sip_method == NULL)
	{
		newsipmsg->sip_method = osip_strdup(sipevent->request->sip_method);
	}
	internal->sipMsg = newsipmsg;
	internal->myaddr = NetAddr(getRequestUrl().getHost(),internal->transaction->getMyPort());

	SIPUrl req = getRequestUrl();
	SIPUrl to = getToUrl();
	SIPUrl from = getFromUrl();
	SIPUrl my = getMyUrl("");

	//logdebug("req host %s to %s from %s my %s", req.getHost().c_str(), to.getHost().c_str(), from.getHost().c_str(), my.getHost().c_str());
}
SIPSession::SIPSession(const SIPSession& session)
{
	internal = new SIPSessionInternal;
	internal->sipMsg = NULL;
	internal->exosip = session.internal->exosip;
	internal->transaction = session.internal->transaction;
	internal->msgtype = session.internal->msgtype;
	internal->event = session.internal->event;
	internal->ack = session.internal->ack;
	internal->transport = session.internal->transport;
	internal->myaddr = session.internal->myaddr;

	osip_message_t* newsipmsg = NULL;

	if(session.internal->sipMsg != NULL && osip_message_clone(session.internal->sipMsg,&newsipmsg) != OSIP_SUCCESS)
	{
		return;
	}
	internal->sipMsg = newsipmsg;	
}
SIPSession::SIPSession(const std::string& method,const SIPUrl& _fromurl,const SIPUrl& _tourl,const SIPTransaction* transaction,const SIPSession* fromSession)
{
	SIPUrl fromurl(_fromurl);
	SIPUrl tourl(_tourl);

	internal = new SIPSessionInternal;
	internal->sipMsg = NULL;
	internal->myaddr = NetAddr(fromurl.getHost(),transaction->getMyPort());


	if(strcasecmp(method.c_str(),"info") == 0 || transaction == NULL)
	{
		return;
	}
	internal->transaction = (SIPTransaction* )transaction;
	internal->exosip = (eXosip_t*)transaction->getExosip();
	
	exOsipGuard loacker(internal->exosip);

	if(fromSession != NULL)
	{
		//如果来源消息的via个数 <=1 ,并且来源的地址和通讯地址不一致，可能是内网访问出来，修正tourl
		int setvlanum = 0;
		if(fromSession != NULL && fromSession->internal != NULL && fromSession->internal->sipMsg != NULL)
		{
			setvlanum = osip_list_size (&fromSession->internal->sipMsg->vias);
		}
		if(setvlanum <=1 && (fromSession->internal->transport.getOtherAddr().getIP() != tourl.getHost() || fromSession->internal->transport.getOtherAddr().getPort() != tourl.getPort()))
		{
			tourl = SIPUrl(tourl.getUserName(),fromSession->internal->transport.getOtherAddr());
		}
	}
	char from[128],to[128];
	
	sprintf(from,"sip:%s@%s:%d",fromurl.getUserName().c_str(),fromurl.getHost().c_str(),fromurl.getPort());
	sprintf(to,"sip:%s@%s:%d",tourl.getUserName().c_str(),tourl.getHost().c_str(),tourl.getPort());
	
	int ret = OSIP_PORT_BUSY;
	if(strcasecmp(method.c_str(),"register") == 0)
	{
	//	sprintf(from, "sip:%s@%s", fromurl.getUserName().c_str(), fromurl.getHost().c_str());
	//	sprintf(to, "sip:%s@%s", tourl.getUserName().c_str(), tourl.getHost().c_str());

	//	char proxyurl[128];
	//	sprintf(proxyurl, "sip:%s@%s", tourl.getUserName().c_str(), tourl.getHost().c_str());

		eXosip_register_remove(internal->exosip,internal->event.rid);
		ret = eXosip_register_build_initial_register(internal->exosip, to, to,from,100,&internal->sipMsg);
		internal->event.rid = ret;
	}
	else if(strcasecmp(method.c_str(),"invite") == 0)
	{
		sprintf(from, "sip:%s@%s", fromurl.getUserName().c_str(), tourl.getHost().c_str());
	//	sprintf(to, "sip:%s@%s", tourl.getUserName().c_str(), tourl.getHost().c_str());

		ret = eXosip_call_build_initial_invite(internal->exosip,&internal->sipMsg,to, from,NULL,NULL);
	//	osip_message_set_supported(internal->sipMsg, "100rel, replaces");
	}
	else
	{
		ret = eXosip_message_build_request(internal->exosip,&internal->sipMsg,method.c_str(),to,from,to);
	}
	if(ret < OSIP_SUCCESS)
	{
		internal->sipMsg = NULL;
		return;
	}

//	osip_message_set_allow(internal->sipMsg, "INVITE,ACK,OPTIONS,BYE,CANCEL,SUBSCRIBE,NOTIFY,REFER,MESSAGE,INFO,PING,PRACK");

	///从消息来源修正方向消息的via
	if(fromSession != NULL && fromSession->internal != NULL && fromSession->internal->sipMsg != NULL)
	{
		int viasize = osip_list_size (&fromSession->internal->sipMsg->vias);
		for(int i = 0;i < viasize - 1;i ++)
		{
			osip_via_t* via = NULL;
			if(osip_message_get_via(fromSession->internal->sipMsg,viasize - 1 - i,&via) >= 0 && via != NULL)
			{
				char* viastr = NULL;
				if(osip_via_to_str(via,&viastr) == OSIP_SUCCESS && viastr != NULL)
				{
					osip_message_append_via(internal->sipMsg,viastr);
					osip_free(viastr);
				}
			}
		}
	}

	osip_contact_t* contact;
	if(osip_message_get_contact(internal->sipMsg,0,&contact) != OSIP_SUCCESS || contact == NULL)
	{
		osip_message_set_contact(internal->sipMsg,to);		
	}	
	
	NetAddr toaddr(tourl.getHost(),tourl.getPort());

	internal->transport = SIPTransport(toaddr,internal->transaction);
}
SIPSession::SIPSession(const std::string& method,const SIPSession& session,const SIPUrl& _fromurl,const SIPUrl& _tourl)
{
	SIPUrl fromurl(_fromurl);
	SIPUrl tourl(_tourl);

	internal = new SIPSessionInternal;
	internal->sipMsg = NULL;
	internal->exosip = session.internal->exosip;
	internal->transaction = session.internal->transaction;
	internal->event = session.internal->event;

	if(internal->exosip == NULL || internal->transaction == NULL)
	{
		return;
	}

	exOsipGuard loacker(internal->exosip);
	
	if(fromurl.getHost() == "")
	{
		fromurl = session.getMyUrl(_fromurl.getUserName());
	}
	if(tourl.getHost() == "")
	{
		SIPUrl siptourl(session.getFromUrl().getUserName(),session.getTransport().getOtherAddr());
		tourl = siptourl;
	}
	
	char from[128],to[128];

	sprintf(from,"sip:%s@%s:%d",fromurl.getUserName().c_str(),fromurl.getHost().c_str(),fromurl.getPort());
	sprintf(to,"sip:%s@%s:%d",tourl.getUserName().c_str(),tourl.getHost().c_str(),tourl.getPort());

	int ret = OSIP_PORT_BUSY;
	if(strcasecmp(method.c_str(),"info") == 0)
	{
		ret = eXosip_call_build_info(internal->exosip,internal->event.did,&internal->sipMsg);
	}
	else if(strcasecmp(method.c_str(),"update") == 0)
	{
		ret = eXosip_call_build_update(internal->exosip,internal->event.did,&internal->sipMsg);
	}
	/*else if(strcasecmp(method.c_str(),"notify") == 0)
	{
		ret = eXosip_call_build_notify(exosip,internal->event.did,EXOSIP_SUBCRSTATE_PENDING,&internal->sipMsg);		
	}*/
	else if(strcasecmp(method.c_str(),"register") == 0)
	{
		ret = eXosip_register_build_register(internal->exosip,internal->event.rid, 100, &internal->sipMsg);
		internal->event.rid = ret;
	}
	else if(strcasecmp(method.c_str(),"invite") == 0)
	{
		ret = eXosip_call_build_initial_invite(internal->exosip,&internal->sipMsg,to,from,to,NULL);
	}
	else if(strcasecmp(method.c_str(),"ack") == 0)
	{
		ret = eXosip_call_build_ack(internal->exosip,internal->event.did,&internal->sipMsg);
	}
	else if(strcasecmp(method.c_str(),"cancel") == 0 || strcasecmp(method.c_str(),"bye") == 0)
	{
		ret = eXosip_call_terminate(internal->exosip,internal->event.cid,internal->event.did);
	}
	else 
	{
		//暂时不使用该方式
// 		if(strcasecmp(method.c_str(),"notify") == 0)
// 		{
// 			ret = eXosip_insubscription_build_notify(internal->exosip,internal->event.did,EXOSIP_SUBCRSTATE_ACTIVE,DEACTIVATED,&internal->sipMsg);
// 		}
		if(internal->sipMsg == NULL)
		{
			ret = eXosip_message_build_request(internal->exosip,&internal->sipMsg,method.c_str(),to,from,to);
			if(ret == OSIP_SUCCESS && internal->sipMsg != NULL)
			{
				if(strcasecmp(method.c_str(),"notify") == 0)
				{
					osip_free(internal->sipMsg->call_id->number);
					osip_call_id_set_number(internal->sipMsg->call_id,osip_strdup(session.internal->sipMsg->call_id->number));
				}
				if(strcmp(internal->sipMsg->req_uri->host,tourl.getHost().c_str()) != 0)
				{
					osip_free(internal->sipMsg->req_uri->host);
					osip_uri_set_host(internal->sipMsg->req_uri,osip_strdup(tourl.getHost().c_str()));
				}
				char portbufr[32];
				sprintf(portbufr,"%d",tourl.getPort());
				if(strcmp(internal->sipMsg->req_uri->port,portbufr) != 0)
				{
					osip_free(internal->sipMsg->req_uri->port);
					osip_uri_set_port(internal->sipMsg->req_uri,osip_strdup(portbufr));
				}
			}
		}
	}
	if(ret != OSIP_SUCCESS || internal->sipMsg == NULL)
	{
		internal->sipMsg = NULL;
		return;
	}
	osip_contact_t* contact;
	if(osip_message_get_contact(internal->sipMsg,0,&contact) != OSIP_SUCCESS || contact == NULL)
	{
		osip_message_set_contact(internal->sipMsg,to);		
	}
	
	NetAddr toaddr(tourl.getHost(),tourl.getPort());

	internal->transport = SIPTransport(toaddr,internal->transaction);
	internal->myaddr = NetAddr(fromurl.getHost(),internal->transaction->getMyPort());
}

SIPSession::~SIPSession()
{
	if(internal != NULL && internal->sipMsg != NULL)
	{
		osip_message_free(internal->sipMsg);
	}
	SAFE_DELETE(internal);
}
bool SIPSession::isVaild() const
{
	return internal->sipMsg != NULL;
}
bool SIPSession::isRequest() const
{
	if(!isVaild())
	{
		return false;
	}
	return MSG_IS_REQUEST(internal->sipMsg);
}
bool SIPSession::isAck() const
{
	if(!isVaild() || internal->sipMsg->sip_method == NULL)
	{
		return false;
	}
	bool ret = strcasecmp(internal->sipMsg->sip_method,"ack") == 0;

	return ret;
}
std::string SIPSession::getMethod() const
{
	if(!isVaild())
	{
		return "";
	}

	return internal->sipMsg->sip_method;
}
SIPStackError SIPSession::getError() const
{
	if (!isVaild())
	{
		return SIPStackError();
	}

	std::string errorinfo;
	{
		char * errorstrinfo = NULL;
		osip_error_info_t* einfo = NULL;
		if (osip_message_get_error_info(internal->sipMsg, 0, &einfo) == SIP_OK && einfo != NULL && osip_error_info_to_str(einfo, &errorstrinfo) == SIP_OK && errorstrinfo != NULL)
		{
			errorinfo = errorstrinfo;
			osip_free(errorstrinfo);
		}
	}

	return SIPStackError((SIPStackErrorCode)internal->sipMsg->status_code,errorinfo);
}
std::string SIPSession::getCallid() const
{
	if(!isVaild() || internal->sipMsg->call_id == NULL || internal->sipMsg->call_id->number == NULL)
	{
		return "";
	}

	return internal->sipMsg->call_id->number;
}
void SIPSession::cloneSession(const SIPSession& session)
{
	SIPSession* sess = (SIPSession*)&session;
	if(internal == NULL || internal->sipMsg == NULL || sess->internal == NULL || sess->internal->sipMsg == NULL)
	{
		return;
	}

	if(internal->sipMsg->call_id  != NULL && sess->internal->sipMsg->call_id != NULL)
	{
		osip_free(internal->sipMsg->call_id->number);
		internal->sipMsg->call_id->number = osip_strdup(session.getCallid().c_str());
	}
	if(internal->sipMsg->from != NULL && sess->internal->sipMsg->from != NULL)
	{
		osip_uri_param_t* sessFromTag;
		osip_uri_param_t* fromTag;
		osip_from_get_tag(sess->internal->sipMsg->from, &sessFromTag);
		osip_from_get_tag(internal->sipMsg->from, &fromTag);
		if (fromTag!=NULL && sessFromTag!=NULL)
		{
			osip_free(fromTag->gvalue);
			fromTag->gvalue = osip_strdup(sessFromTag->gvalue);
		}
	}
	
}

std::string SIPSession::getDateTime() const
{
	osip_header_t*        p_header = NULL;

	osip_message_get_date(internal->sipMsg, 0, &p_header);
	if (p_header != NULL)
	{
		if (p_header->hvalue != NULL)
		{
			return p_header->hvalue;
		}
	}

	return "";
}
void SIPSession::setDateTime(const std::string& date)
{
	if(!isVaild())
	{
		return;
	}

	osip_message_set_date(internal->sipMsg,date.c_str());
}

int SIPSession::getExpries(int defaultexpriex) const
{
	if(!isVaild())
	{
		return 0;
	}

	osip_header_t*        p_header = NULL;
	osip_contact_t*       pContact = NULL;
	osip_generic_param_t* pParam = NULL;
	int                expires = 0;

	osip_message_get_expires(internal->sipMsg, 0, &p_header);
	if (p_header != NULL)
	{
		if (p_header->hvalue != NULL)
		{
			expires = atoi(p_header->hvalue);
		}
	}
	else
	{
		if (0 == osip_message_get_contact(internal->sipMsg, 0, &pContact) )
		{	
			osip_contact_param_get_byname(pContact, "expires", &pParam);
			if ( (pParam != NULL) && (pParam->gvalue != NULL) )
			{
				expires = atoi(pParam->gvalue);
			}
		}
	}
	if(expires == 0 || (expires > defaultexpriex && defaultexpriex != 0))
	{
		expires = defaultexpriex;
	}

	return expires;
}
void SIPSession::setExpries(int expires)
{
	if(!isVaild())
	{
		return;
	}

	char buf[32];
	sprintf(buf,"%d",expires);

	osip_header_t*        p_header = NULL;
	osip_message_get_expires(internal->sipMsg, 0, &p_header);
	if (p_header != NULL)
	{
		if (p_header->hvalue != NULL)
		{
			osip_free(p_header->hvalue);
		}
		p_header->hvalue = osip_strdup(buf);
	}
	else 
	{
		osip_message_set_expires(internal->sipMsg,buf);
	}		
}
int SIPSession::getCseq() const
{
	if(internal == NULL || internal->sipMsg == NULL || internal->sipMsg->cseq == NULL || internal->sipMsg->cseq->number == NULL)
	{
		return 0;
	}

	return atoi(internal->sipMsg->cseq->number);
}

void SIPSession::setCseq(int cseq)
{
	char tmp[32];
	sprintf(tmp,"%d",cseq);
	if(internal == NULL || internal->sipMsg == NULL || internal->sipMsg->cseq == NULL || internal->sipMsg->cseq->number == NULL)
	{
		return;
	}
	osip_free(internal->sipMsg->cseq->number);
	internal->sipMsg->cseq->number = osip_strdup(tmp);
}
std::string SIPSession::getProxyAuthenInfo() const
{
	if (!isVaild())
	{
		return "";
	}
	{
		osip_proxy_authorization_t* authen = NULL;

		osip_message_get_proxy_authorization(internal->sipMsg, 0, &authen);
		if (authen != NULL) return authen->nonce;
	}
	{
		osip_proxy_authenticate_t* authen = NULL;

		osip_message_get_proxy_authenticate(internal->sipMsg, 0, &authen);
		if (authen != NULL) return authen->nonce;
	}

	return "";
}
bool SIPSession::sendSuccess(const std::string& msgXml,const std::string& contentType,const std::string& date)
{
	return sendFailure(SIPStackError(),msgXml,contentType,date);
}
bool SIPSession::sendFailure(const SIPStackError& errorInfo, const std::string& msgXml,const std::string& contentType,const std::string& date)
{
	if(!isVaild() || !isRequest())
	{
		return false;
	}

	if(internal->exosip == NULL || internal->transaction == NULL)
	{
		return false;
	}

	exOsipGuard loacker(internal->exosip);
	osip_message* answer = NULL;
	int messageType = 0;// 0 : call 1:subscribe 2:message


	if(MSG_IS_INVITE(internal->sipMsg) || MSG_IS_INFO(internal->sipMsg) || MSG_IS_UPDATE(internal->sipMsg))
	{
		int ret = eXosip_call_build_answer(internal->exosip, internal->event.tid, errorInfo.getErrorCode(), &answer);
		if(ret == OSIP_SUCCESS)
		{
			messageType = 0;//call
		}	
	}
	else
	{
		if(MSG_IS_SUBSCRIBE(internal->sipMsg) || MSG_IS_NOTIFY(internal->sipMsg))
		{
			if(eXosip_insubscription_build_answer(internal->exosip,internal->event.tid, errorInfo.getErrorCode(), &answer) == OSIP_SUCCESS)
			{
				messageType = 1;//subscirbe
			}			
		}
		//subscribe 和 message放在一起处理，因为有可能有些用message做subscirbe处理
		if(answer == NULL)
		{
			if(eXosip_message_build_answer(internal->exosip,internal->event.tid, errorInfo.getErrorCode(), &answer) == OSIP_SUCCESS)
			{
				messageType = 2;//message
			}			
		}
	}
	if(answer == NULL)
	{
		return false;
	}
	if(msgXml.size() > 0)
	{
		osip_message_set_body(answer, msgXml.c_str(), msgXml.size() );
		osip_message_set_content_type(answer, contentType.c_str());
		char tmpbuf[32];
		snprintf(tmpbuf, 32, "%d", (int)msgXml.size());
		osip_message_set_content_length(answer, tmpbuf);
	}

	if(errorInfo.getErrorCode() != SIPStackError_OK && errorInfo.getErrorInfo() != "" && errorInfo.getErrorInfo() != "<>")
	{
		osip_message_set_error_info(answer, errorInfo.getErrorInfo().c_str());
	}
	if(date != "")
	{
		osip_message_set_date(answer,date.c_str());
	}
	//set expries
	{
		int expries = getExpries();
		if(expries > 0)
		{
			char buf[32];
			sprintf(buf,"%d",expries);

			osip_header_t*        p_header = NULL;
			osip_message_get_expires(answer, 0, &p_header);
			if (p_header != NULL)
			{
				if (p_header->hvalue != NULL)
				{
					osip_free(p_header->hvalue);
				}
				p_header->hvalue = osip_strdup(buf);
			}
			else 
			{
				osip_message_set_expires(answer,buf);
			}		
		}
	}
	if(messageType == 0)
	{
		eXosip_call_send_answer(internal->exosip,internal->event.tid, errorInfo.getErrorCode(), answer);
	}
	else if(messageType == 1)
	{
		eXosip_insubscription_send_answer(internal->exosip,internal->event.tid, errorInfo.getErrorCode(), answer);
	}
	else
	{
		eXosip_message_send_answer(internal->exosip,internal->event.tid, errorInfo.getErrorCode(), answer);
	}

	return true;
}
bool SIPSession::send401Failure(const std::string& username)
{
	if(!isVaild() || !isRequest())
	{
		return false;
	}
	
	if(internal->exosip == NULL || internal->transaction == NULL)
	{
		return false;
	}

	exOsipGuard loacker(internal->exosip);

	std::string zip = username.substr(0, 10);
	osip_www_authenticate_t * authen = SIPAuthenticate::getWWWAuthenticate(internal->sipMsg,zip/*username*/);

	if(authen == NULL)
	{
		return false;
	}

	osip_message* answer;
	if(eXosip_message_build_answer(internal->exosip,internal->event.tid,SIP_UNAUTHORIZED, &answer) != OSIP_SUCCESS)
	{
		return false;
	}

// 	int expires = getExpries(0);
// 	if(expires != 0)
// 	{
// 		char buf[32];
// 		sprintf(buf,"%d",expires);
// 
// 		osip_header_t*        p_header = NULL;
// 		osip_message_get_expires(answer, 0, &p_header);
// 		if (p_header != NULL)
// 		{
// 			if (p_header->hvalue != NULL)
// 			{
// 				osip_free(p_header->hvalue);
// 			}
// 			p_header->hvalue = osip_strdup(buf);
// 		}
// 		else 
// 		{
// 			osip_message_set_expires(answer,buf);
// 		}		
// 	}

	osip_list_add(&answer->www_authenticates, authen, 0);

	eXosip_message_send_answer(internal->exosip,internal->event.tid, SIP_UNAUTHORIZED, answer);

	return true;
}
bool SIPSession::sendProceeding()
{
	if(!isVaild() || !isRequest())
	{
		return false;
	}

	if(internal->exosip == NULL || internal->transaction == NULL)
	{
		return false;
	}

	exOsipGuard loacker(internal->exosip);

	if(MSG_IS_INVITE(internal->sipMsg) || MSG_IS_INFO(internal->sipMsg))
	{
		eXosip_call_send_answer(internal->exosip,internal->event.tid,SIP_TRYING,NULL);
	}
	else
	{
		eXosip_message_send_answer(internal->exosip,internal->event.tid,SIP_TRYING,NULL);
	}

	return true;
}
bool SIPSession::sendAck()
{
	if(internal->exosip == NULL || internal->transaction == NULL)
	{
		return false;
	}

	exOsipGuard loacker(internal->exosip);

	eXosip_call_send_ack(internal->exosip,internal->event.did,NULL);

	return true;
}
std::string SIPSession::getWWWAuthenicatUsername()const
{
	if(!isVaild() || isRequest())
	{
		return false;
	}

	osip_www_authenticate_t *pAuthEcho = NULL;
	if (osip_message_get_www_authenticate(internal->sipMsg, 0, &pAuthEcho) == -1)
	{
		return "";
	}
	if(pAuthEcho->realm == NULL)
	{
		return "";
	}

	return SIPAuthenticate::unquote(pAuthEcho->realm);
}
std::string SIPSession::getAuthenicateUserName()const
{
	if(!isVaild())
	{
		return "";
	}

	osip_authorization_t *pAuthEcho = NULL;
	if (osip_message_get_authorization(internal->sipMsg, 0, &pAuthEcho) == -1)
	{
		return "";
	}
	if(pAuthEcho->username == NULL)
	{
		return "";
	}

	return SIPAuthenticate::unquote(pAuthEcho->username);
}
bool SIPSession::buildAuthenticate(const std::string& username, const std::string& passwd)
{
	if (!isVaild() || isRequest())
	{
		return false;
	}

	if (internal->exosip == NULL || internal->transaction == NULL)
	{
		return false;
	}

	exOsipGuard loacker(internal->exosip);

	eXosip_clear_authentication_info(internal->exosip);
	int iRet = eXosip_add_authentication_info(internal->exosip, username.c_str(), username.c_str(), passwd.c_str(), "MD5", NULL);
	if (iRet != OSIP_SUCCESS)
	{
		return false;
	}

	return true;
}
bool SIPSession::sendAuthenticate(const std::string& username,const std::string& passwd,int expries)
{
	if(!isVaild() || isRequest())
	{
		return false;
	}

	if(internal->exosip == NULL || internal->transaction == NULL)
	{
		return false;
	}
	
	exOsipGuard loacker(internal->exosip);

	eXosip_clear_authentication_info(internal->exosip);
	int iRet = eXosip_add_authentication_info(internal->exosip,username.c_str(), username.c_str(), passwd.c_str(), "MD5", NULL);
	if (iRet != OSIP_SUCCESS) 
	{
		return false;
	}

	osip_message_t *reg;
	if (eXosip_register_build_register(internal->exosip,internal->event.rid, expries, &reg)  != OSIP_SUCCESS)
	{
		return false;
	}

	if (eXosip_register_send_register(internal->exosip,internal->event.rid, reg) != OSIP_SUCCESS)
	{
		return false;
	}

	return true;
}

bool SIPSession::Authenticate(const std::string& name,const std::string& passwd)
{
	if(!isVaild() || !isRequest())
	{
		return false;
	}

	return SIPAuthenticate::Authenticate(internal->sipMsg,name,passwd);
}
SIPUrl SIPSession::getAuthenticateUri()
{
	osip_authorization_t *pAuthEcho = NULL;
	if (osip_message_get_authorization(internal->sipMsg, 0, &pAuthEcho) == -1)
	{
		return SIPUrl("","",0);
	}
	const char* pUri = (char*)SIPAuthenticate::unquote(pAuthEcho->uri == NULL ? "" : pAuthEcho->uri);

	SIPUrl defaulturi("", "", 0);

	osip_uri_t* uri = NULL;
	osip_uri_init(&uri);
	if (osip_uri_parse(uri, pUri) == OSIP_SUCCESS)
	{
		defaulturi = SIPUrl(uri->username == NULL ? "" : uri->username, uri->host == NULL ? "" : uri->host, uri->port == NULL ? "" : uri->port);
	}
	osip_uri_free(uri);

	return defaulturi;
}
bool SIPSession::sendRequest(const std::string& msgXml,const std::string& contentType,const std::string& subject)
{
	if(!isVaild() || !isRequest())
	{
		return false;
	}

	if (!subject.empty())
	{
		osip_message_set_subject(internal->sipMsg, subject.c_str());
	}

	if(msgXml.size() > 0)
	{
		osip_message_set_body(internal->sipMsg, msgXml.c_str(), msgXml.size() );
		osip_message_set_content_type(internal->sipMsg, contentType.c_str());
		char tmpbuf[32];
		snprintf(tmpbuf, 32, "%d", (int)msgXml.size());
		osip_message_set_content_length(internal->sipMsg, tmpbuf);
	}

	if(internal->exosip == NULL || internal->transaction == NULL)
	{
		return false;
	}

	exOsipGuard loacker(internal->exosip);

	osip_message_t* newmsg;
	osip_message_clone(internal->sipMsg,&newmsg);
	int ret = 0;

	if(MSG_IS_REGISTER(internal->sipMsg))
	{	
		if (!eXosip_register_send_register(internal->exosip,internal->event.rid,internal->sipMsg) == OSIP_SUCCESS)
		{
			logdebug("Send Register Sip Msg Failure %s %d\r\n", __FUNCTION__, __LINE__);
		}
	}
	else if(MSG_IS_INVITE(internal->sipMsg))
	{
		ret = eXosip_call_send_initial_invite(internal->exosip,internal->sipMsg);
	}
	else if(MSG_IS_INFO(internal->sipMsg))
	{
		ret = eXosip_call_send_request(internal->exosip,internal->event.did,internal->sipMsg);
	}
	else if(MSG_IS_ACK(internal->sipMsg))
	{
		ret = eXosip_call_send_ack(internal->exosip,internal->event.did,internal->sipMsg);
	}
	else
	{
		//暂时不使用该方式
// 		if (MSG_IS_NOTIFY(internal->sipMsg))
// 		{
// 			ret = eXosip_insubscription_send_request(internal->exosip, internal->event.did, internal->sipMsg);
// 		}
// 		if (ret < 0)
		{
			ret = eXosip_message_send_request(internal->exosip, internal->sipMsg);
		}
	}
	internal->sipMsg = newmsg;

	return true;
}

SipDialog SIPSession::getDialog() const
{
	return SipDialog(this);
}
SIPTransport SIPSession::getTransport() const
{
	return internal->transport;
}
SIPTransaction* SIPSession::getTransaction() const
{
	if(!isVaild())
	{
		return NULL;
	}

	return internal->transaction;
}
SIPUrl SIPSession::getRequestUrl() const
{
	std::string username,host,port;

	if(isVaild() && internal->sipMsg->req_uri != NULL)
	{
		if(internal->sipMsg->req_uri->username != NULL)
		{
			username = internal->sipMsg->req_uri->username;
		}
		if(internal->sipMsg->req_uri->host != NULL)
		{
			host = internal->sipMsg->req_uri->host;
		}
		if(internal->sipMsg->req_uri->port != NULL)
		{
			port = internal->sipMsg->req_uri->port;
		}
	}

	return SIPUrl(username,host,port);
}
SIPUrl SIPSession::getFromUrl() const
{
	std::string username,host,port;

	if(isVaild() && internal->sipMsg->from != NULL && internal->sipMsg->from->url != NULL)
	{
		if(internal->sipMsg->from->url->username != NULL)
		{
			username = internal->sipMsg->from->url->username;
		}
		if(internal->sipMsg->from->url->host != NULL)
		{
			host = internal->sipMsg->from->url->host;
		}
		if(internal->sipMsg->from->url->port != NULL)
		{
			port = internal->sipMsg->from->url->port;
		}
	}

	return SIPUrl(username,host,port);
}
SIPUrl SIPSession::getToUrl() const
{
	std::string username,host,port;

	if(isVaild() && internal->sipMsg->to != NULL && internal->sipMsg->to->url != NULL)
	{
		if(internal->sipMsg->to->url->username != NULL)
		{
			username = internal->sipMsg->to->url->username;
		}
		if(internal->sipMsg->to->url->host != NULL)
		{
			host = internal->sipMsg->to->url->host;
		}
		if(internal->sipMsg->to->url->port != NULL)
		{
			port = internal->sipMsg->to->url->port;
		}
	}

	return SIPUrl(username,host,port);
}
SIPUrl SIPSession::getViaUrl() const
{
	std::string username,host,port;
	
	if(isVaild())
	{
		osip_list_iterator_t inter;
		osip_contact_t* pContact = (osip_contact_t*)osip_list_get_first(&internal->sipMsg->contacts,&inter);
		if(pContact != NULL && pContact->url != NULL)
		{
			if(pContact->url->username != NULL)
			{
				username = pContact->url->username;
			}
			if(pContact->url->host != NULL)
			{
				host = pContact->url->host;
			}
			if(pContact->url->password != NULL)
			{
				port = pContact->url->password;
			}
		}
	}
	return SIPUrl(username,host,port);
}
SIPUrl SIPSession::getMyUrl(const std::string& myid) const
{
	if(!isVaild())
	{
		return SIPUrl("","",5060);
	}

	SIPTransaction* transaction = (SIPTransaction*)internal->transaction;
	if(transaction == NULL)
	{
		return SIPUrl("","",5060);
	}

	SIPUrl myurl(myid,internal->myaddr);

	return myurl;
}
SIPUrl SIPSession::getContactUrl() const
{
	if(internal == NULL || internal->sipMsg == NULL)
	{
		return SIPUrl("","",5060);
	}
	osip_contact_t* contact = NULL;
	if(osip_message_get_contact(internal->sipMsg,0,&contact) != OSIP_SUCCESS || contact == NULL || contact->url == NULL)
	{
		return SIPUrl(getFromUrl().getUserName(),internal->transport.getOtherAddr());	
	}

	return SIPUrl(contact->url->username == NULL ? "" : contact->url->username, contact->url->host == NULL ? "" : contact->url->host, contact->url->port == NULL ? "" : contact->url->port);
}
std::string SIPSession::getMessageBody() const
{
	if(!isVaild())
	{
		return "";
	}

	osip_body_t *pBody = NULL;
	int ret = osip_message_get_body(internal->sipMsg, 0, &pBody);
	if(ret != OSIP_SUCCESS || pBody == NULL || pBody->body == NULL || pBody->length <= 0)
	{
		return "";
	}

	return std::string(pBody->body,pBody->length);
}

void* SIPSession::getSipMessage() const
{
	if(!isVaild())
	{
		return NULL;
	}

	return internal->sipMsg;
}

bool SIPSession::parseSDPByMessage(StreamSessionSDP& sdpinfo)
{	
	if(!isVaild())
	{
		return false;
	}
	osip_message_t *pSipMsg = (osip_message_t *)internal->sipMsg;
	std::string deviceId = pSipMsg->to->url->username;


	osip_body_t *pBody = NULL;

	int iRet = osip_message_get_body(pSipMsg, 0, &pBody);
	if ( (-1 == iRet) || (NULL == pBody->body) )
	{
		return false;
	}
	char*pXmlStr = pBody->body;

	return s_parseSdPByMessage(sdpinfo,deviceId,pXmlStr);
}
std::string SIPSession::buildSDPXml(const std::string& myid,const StreamSessionSDP& sdpinfo)
{
	return s_buildSDPXml(myid,sdpinfo);
}

bool SIPSession::parseSDPSubject(std::string& mediassrc,std::string& recvssrc)
{
	if(!isVaild())
	{
		return false;
	}
	osip_message_t *pSipMsg = (osip_message_t *)internal->sipMsg;

	osip_header_t * sub = NULL;
	if(osip_message_get_subject(pSipMsg,0,&sub) == OSIP_UNDEFINED_ERROR || sub == NULL)
	{
		return false;
	}

	return s_ParseSubject(sub->hvalue,mediassrc,recvssrc);
}

std::string SIPSession::buildSDPSubject(const std::string& myid,const StreamSessionSDP& sdpinfo)
{
	return s_BuildSubject(myid,sdpinfo);
}

};
};
