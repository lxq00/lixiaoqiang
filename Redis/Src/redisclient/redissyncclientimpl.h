
#ifndef REDISCLIENT_REDISSYNCCLIENTIMPL_H
#define REDISCLIENT_REDISSYNCCLIENTIMPL_H


#include "Base/Base.h"
#include "Network/Network.h"
using namespace Public::Base;
using namespace Public::Network;

#include "redisvalue.h"
#include "redisparser.h"

namespace redisclient{

class RedisSyncClientImpl :public RedisParser
{
public:
	RedisSyncClientImpl(const shared_ptr<IOWorker>& worker);
	virtual ~RedisSyncClientImpl();

	bool connect(const NetAddr& addr, uint32_t connecttime,uint32_t cmdtimeout);

	void close();

	bool command(const std::string& cmdstr, RedisValue& value);

	bool isConnected() const;
private:
	shared_ptr<IOWorker>												worker;
	shared_ptr<Socket>													sock;
};
}

#endif // REDISCLIENT_REDISCLIENTIMPL_H
