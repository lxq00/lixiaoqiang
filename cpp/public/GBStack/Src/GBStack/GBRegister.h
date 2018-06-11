#ifndef __GBREGISTER_H__
#define __GBREGISTER_H__
#include "SIPStack/SIPTransaction.h"
#include "SIPStack/SIPStackError.h"
#include "GBStack/GBTypes.h"
#include "GBStack/GBStructs.h"
#include "InterProtocol/InterProtocol.h"
#include "Base/Base.h"
#include "GBStack/GBStack.h"
using namespace Public::SIPStack;
using namespace Public::Base;
namespace Public{
namespace GB28181{

struct GBPlatformInfo;

class GBRegister
{
public:
	typedef Function2<void,void*,bool>		CuttoffCallback;
public:
	GBRegister(const weak_ptr<GBStack>& gb, const weak_ptr<GBDispacher>& dispacher,const weak_ptr<GBPlatformInfo>& platinfo,const CuttoffCallback& callback);
	~GBRegister();
	
	void regist(int expries = 30);

	void setSession(shared_ptr<SIPSession>& session);
	void setAddResiterAck(const SIPStackError& error,unsigned long long dateTime = 0);
	void recv2xx(shared_ptr<SIPSession>& session);
	void recv3456xx(shared_ptr<SIPSession>& session);
	void recvkeepalive(shared_ptr<SIPSession>& session);
	void recvkeepalive2xx(shared_ptr<SIPSession>& session);

	shared_ptr<SIPTransaction> getTransaction() const;
	SIPUrl getMyUrl() const;
	SIPUrl getToUrl(const std::string& id = "") const;
private:
	void sendRegister(int expries);
	void sendKeepAlive();
	void timerProc(unsigned long param);
private:
	weak_ptr<GBStack> 		gbstack;
	weak_ptr<GBDispacher>	dispacher;
	weak_ptr<GBPlatformInfo>platformInfo;
	CuttoffCallback			offlineCalblack;

	int 					initExpries;
	int						regExpries;
	
	SIPStackErrorCode		registerStatus;
	uint64_t				prevRegTime;
	uint64_t				prevKeepAliveTime;
	uint64_t				prevRegSuccTime;
	uint64_t				prevKeepAliveSuccTime;
	shared_ptr<SIPSession>	registerSession;

	bool 					isStartRegist;
	shared_ptr<Timer>		poolTimer;
};


};
};


#endif //__GBREGISTER_H__
