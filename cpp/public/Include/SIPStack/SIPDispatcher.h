#ifndef __SIPDISPATCHER_H__
#define __SIPDISPATCHER_H__
#include "SIPStack/SIPSession.h"
namespace Public{
namespace SIPStack {

class SIP_API SIPDispatcher
{
public:
	SIPDispatcher(){}
	virtual ~SIPDispatcher(){}
	
	virtual void rcvInvite(shared_ptr<SIPSession>& session){}
	virtual void rcvAck(shared_ptr<SIPSession>& session){}
	virtual void rcvRegister(shared_ptr<SIPSession>& session){}
	virtual void rcvBye(shared_ptr<SIPSession>& session){}
	virtual void rcvCancel(shared_ptr<SIPSession>& session){}
	virtual void rcvInfo(shared_ptr<SIPSession>& session){}
	virtual void rcvUpdate(shared_ptr<SIPSession>& session){}
	virtual void rcvOptions(shared_ptr<SIPSession>& session){}
	virtual void rcvSubscribe(shared_ptr<SIPSession>& session){}
	virtual void rcvNotify(shared_ptr<SIPSession>& session){}
	virtual void rcvMessage(shared_ptr<SIPSession>& session){}
	virtual void rcvrequest(shared_ptr<SIPSession>& session){}
	virtual void rcv1xx(shared_ptr<SIPSession>& session){}
	virtual void rcv2xx_Invite(shared_ptr<SIPSession>& session){}
	virtual void rcv2xx_Register(shared_ptr<SIPSession>& session){}
	virtual void rcv2xx_Bye(shared_ptr<SIPSession>& session){}
	virtual void rcv2xx_Cancel(shared_ptr<SIPSession>& session){}
	virtual void rcv2xx_Info(shared_ptr<SIPSession>& session){}
	virtual void rcv2xx_Update(shared_ptr<SIPSession>& session){}
	virtual void rcv2xx_Options(shared_ptr<SIPSession>& session){}
	virtual void rcv2xx_Subscribe(shared_ptr<SIPSession>& session){}
	virtual void rcv2xx_Notify(shared_ptr<SIPSession>& session){}
	virtual void rcv2xx_Message(shared_ptr<SIPSession>& session){}
	virtual void rcv2xx(shared_ptr<SIPSession>& session){}
	virtual void rcv3456xx_Invite(shared_ptr<SIPSession>& session){}
	virtual void rcv3456xx_Register(shared_ptr<SIPSession>& session){}
	virtual void rcv3456xx_Bye(shared_ptr<SIPSession>& session){}
	virtual void rcv3456xx_Cancel(shared_ptr<SIPSession>& session){}
	virtual void rcv3456xx_Info(shared_ptr<SIPSession>& session){}
	virtual void rcv3456xx_Update(shared_ptr<SIPSession>& session){}
	virtual void rcv3456xx_Options(shared_ptr<SIPSession>& session){}
	virtual void rcv3456xx_Subscribe(shared_ptr<SIPSession>& session){}
	virtual void rcv3456xx_Notify(shared_ptr<SIPSession>& session){}
	virtual void rcv3456xx_Message(shared_ptr<SIPSession>& session){}
	virtual void rcv3456xx(shared_ptr<SIPSession>& session){}
};

};
};


#endif //__SIPDISPATCHER_H__
