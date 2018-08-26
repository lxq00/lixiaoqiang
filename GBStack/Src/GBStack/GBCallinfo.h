#ifndef __GBCALLINFO_H__
#define __GBCALLINFO_H__
#include "Base/Base.h"
#include "SIPStack/SIPSession.h"
#include "GBStack/GBStack.h"
#include "SIPStack/SIPStackError.h"
#include "GBRegister.h"
using namespace Public::Base;
using namespace Public::SIPStack;

namespace Public{
namespace GB28181{

class GBCallinfo
{
	struct GBCallinfoInternal;
public:
	GBCallinfo(const weak_ptr<GBRegister>& reg,const shared_ptr<SIPSession>& session,const StreamSessionSDP& sdp);
	~GBCallinfo();
	
	void recvInviteResp(shared_ptr<SIPSession>& session);
	void recvAck(shared_ptr<SIPSession>& session);
	void recvUpdate();
	void recvUpdateResp();
	bool poolCallAndCheckIsTimeout();
	void recvBye();
	void sendResp(GBEventID id,const SIPStackError& error,const StreamSDPInfo& sdp);
	void sendACK(GBEventID id,const std::string sendip,int sendport);
	SipDialog getDialog();
	std::string getDeviceUrl();

	static StreamSessionSDP buildSessionSDP(const std::string& devid,long long startTime,long long endTime,int type,int downSpeed,int streamType,const StreamSDPInfo& sdp);
	static StreamSDPInfo buildSDP(const StreamSessionSDP& ssdp);
private:
	GBCallinfoInternal* internal;
};

};
};

#endif //__GBCALLINFO_H__
