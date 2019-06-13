#ifndef RABBITMQ_CLIENT_H_
#define RABBITMQ_CLIENT_H_
 
#include "Defs.h"
#include "Base/Base.h"
using namespace Public::Base;

namespace Public {
namespace Rabbitmq {
	
class  RABBITMQ_API RabbitmqClient
{
public:
	typedef Function1<void, const std::string&> SubscribeCallback;
	typedef Function1<void, bool/*offline*/> ConnectStatusCallback;
public:
	RabbitmqClient(const ConnectStatusCallback &status);
	~RabbitmqClient();
 
	bool connect(const string &strvhost, const string &strHostname, int iPort, const string &strUser, const string &strPasswd, uint32_t timeout);
	//bool connect(const string &strHostname, int iPort, const string &strUser, const string &strPasswd );
	bool disconnect();

	bool bind(const string &strQueueName, const string &strExchange, const string &strRoutekey,bool durable = false);
	
	bool publish(const string &strMessage);
	bool publish( const string &strRoutekey, const string &strMessage );
	
	bool subscribe(void* flag,const SubscribeCallback& SubscribeCall);
	bool unsubcribe(void* flag);
private:
	struct RabbitmqClientInternal;
	RabbitmqClientInternal* internal;
};
 
}
}
#endif