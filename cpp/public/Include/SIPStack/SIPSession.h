#ifndef __SIPSESSION_H__
#define __SIPSESSION_H__
#include "SIPStack/SIPStackDefs.h"
#include "SIPStack/SIPUrl.h"
#include "SIPStack/SIPDialog.h"
#include "SIPStack/SIPTransport.h"
#include "SIPStack/SIPStackError.h"
namespace Public{
namespace SIPStack {
class SIPTransaction;
class SipDialog;
class eXosipDispatcher;
#define XMLCONTENTTYPE		"Application/MANSCDP+xml"
#define XMLINVITETYPE		"application/sdp"
#define XMLRTSPTYPE		"application/rtsp"

class SIP_API SIPSession
{
	friend class eXosipDispatcher;
	friend class SIPTransaction;
	friend class SipDialog;
	struct SIPSessionInternal;
private:
	enum MessageType
	{
		MessageType_Request,
		MessageType_Responce,
		MessageType_Ack,
	};
	SIPSession(void* exosipevent,void* exosip,void* transaction,MessageType type);
public:
	SIPSession(const SIPSession& session);
	SIPSession(const std::string& method,const SIPUrl& from,const SIPUrl& to,const SIPTransaction* transaction,const SIPSession* fromSession = NULL);
	SIPSession(const std::string& method,const SIPSession& session,const SIPUrl& from = SIPUrl("","",5060),const SIPUrl& to = SIPUrl("","",5060));
	~SIPSession();
	
	bool isVaild() const;
	bool isRequest() const;
	bool isAck() const;

	std::string getDateTime() const;
	void setDateTime(const std::string& date);
	
	std::string getMethod() const;
	SIPStackError getError() const;
	std::string getCallid() const;
	void cloneSession(const SIPSession& session);
		
	int getExpries(int defaultexpriex = 0) const;
	void setExpries(int expries);
	int getCseq() const;
	void setCseq(int cseq);

	std::string getProxyAuthenInfo() const;
	
	bool sendSuccess(const std::string& msgXml = "",const std::string& contentType = XMLCONTENTTYPE,const std::string& date = "");
	bool sendFailure(const SIPStackError& errorInfo,const std::string& msgXml = "",const std::string& contentType = XMLCONTENTTYPE,const std::string& date = "");
	bool send401Failure(const std::string& username);
	bool sendProceeding();
	bool sendAck();
	
	bool buildAuthenticate(const std::string& username, const std::string& passwd);
	bool sendAuthenticate(const std::string& username,const std::string& passwd,int expries);
	std::string getAuthenicateUserName() const;
	std::string getWWWAuthenicatUsername() const;
	bool Authenticate(const std::string& name,const std::string& passwd);
	SIPUrl getAuthenticateUri();

	bool sendRequest(const std::string& msgXml = "",const std::string& contentType = XMLCONTENTTYPE,const std::string& subject = std::string());
	
	SipDialog getDialog() const;
	SIPTransport getTransport() const;
	SIPTransaction* getTransaction() const;
	
	SIPUrl getRequestUrl() const;
	SIPUrl getFromUrl() const;
	SIPUrl getToUrl() const;
	SIPUrl getViaUrl() const;
	SIPUrl getMyUrl(const std::string& myid) const;
	SIPUrl getContactUrl() const;
	
	std::string getMessageBody() const;

	void* getSipMessage() const;

	bool parseSDPByMessage(StreamSessionSDP& sdpinfo);
	static std::string buildSDPXml(const std::string& myid,const StreamSessionSDP& sdpinfo);

	bool parseSDPSubject(std::string& mediassrc,std::string& recvssrc);
	static std::string buildSDPSubject(const std::string& myid,const StreamSessionSDP& sdpinfo);
private:
	SIPSessionInternal* internal;
};
};
};

#endif //__SIPSESSION_H__
