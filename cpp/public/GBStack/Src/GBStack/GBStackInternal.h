#ifndef __GBSTACKINTERNAL_H__
#define __GBSTACKINTERNAL_H__
#include "Network/Network.h"
#include "GBStack/GBStack.h"
#include "GBRegister.h"
#include "GBCallinfo.h"
#include "GBEventPool.h"
#ifdef WIN32
typedef int socklen_t;
#endif
namespace Public{
namespace GB28181{
	struct GBPlatformInfo
	{
		enum{
			PlatformStatus_Init,
			PlatformStatus_Authen,
			PlatformStatus_Success,
		}						status;
		enum{
			PlatformType_Up,
			PlatformType_Low,
		}								type;

		std::string 					clientId;
		std::string 					clientIp;
		int 							clientPort;
		std::string						fromIp;
		std::string						toIp;
		std::string						requestIp;
		std::string						authenIp;

		std::string						myIp;

		std::string						usrename;
		std::string						passwd;

		std::string						myid;
		std::string						myusername;
		std::string						mypasswd;
		
		shared_ptr<SIPTransaction>		transaction;
		shared_ptr<GBRegister>			registe;
		SIPTransport					transport;
	};

class GBStackInternal:public SIPDispatcher ,public GBEventPool
{
public:
	Mutex 												gbmutex;
	std::map<GBCommuID, shared_ptr<SIPTransaction> >		communicationList;
	std::map<SIPTransport, shared_ptr<GBPlatformInfo> >	platformTransportMap;

	std::list<shared_ptr<GBPlatformInfo> >				cutoffPlatList;
	std::list<shared_ptr<SIPTransaction> >			cutoffComList;

	weak_ptr<GBDispacher>								dispacher;
	std::string 										myId;
	std::string											myUser;
	std::string					 						myPass;
	std::string											uAgent;
	weak_ptr<GBStack>									gbstack;
	shared_ptr<Timer>									workTimer;
public:
	void platfromCutoff(GBPlatformID platform,bool force);
public:
	GBStackInternal();
	~GBStackInternal();

	void init(const weak_ptr<GBStack>& stack);
	void rcvInvite(shared_ptr<SIPSession>& session);
	void rcvAck(shared_ptr<SIPSession>& session);
	void rcvRegister(shared_ptr<SIPSession>& session);
	void rcvBye(shared_ptr<SIPSession>& session);
	void rcvCancel(shared_ptr<SIPSession>& session);
	void rcvInfo(shared_ptr<SIPSession>& session);
	void recvUpdate(shared_ptr<SIPSession>& session);
	void rcvNotify(shared_ptr<SIPSession>& session);
	void rcvSubscribe(shared_ptr<SIPSession>& session);
	void rcvMessage(shared_ptr<SIPSession>& session);
	void rcv2xx_Invite(shared_ptr<SIPSession>& session);
	void rcv2xx_Update(shared_ptr<SIPSession>& session);
	void rcv2xx_Register(shared_ptr<SIPSession>& session);
	void rcv2xx_Message(shared_ptr<SIPSession>& session);
	void rcv3456xx_Message(shared_ptr<SIPSession>& session);
	void rcv2xx_Notify(shared_ptr<SIPSession>& session);
	void rcv2xx(shared_ptr<SIPSession>& session);
	void rcv3456xx_Notify(shared_ptr<SIPSession>& session);
	void rcv3456xx_Invite(shared_ptr<SIPSession>& session);
	void rcv3456xx_Register(shared_ptr<SIPSession>& session);
	void rcv2xx_Info(shared_ptr<SIPSession>& session);
	void rcv3456xx_Info(shared_ptr<SIPSession>& session);

	int getRandNum(GBPlatformID platformId);
private:
	void timerProc(unsigned long param);
public:
	shared_ptr<GBPlatformInfo> _queryPlatfromById(GBPlatformID platid);
};


};
};

#endif //__GBSTACKINTERNAL_H__
