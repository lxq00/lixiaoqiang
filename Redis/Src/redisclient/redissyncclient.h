#ifndef REDISSYNCCLIENT_REDISCLIENT_H
#define REDISSYNCCLIENT_REDISCLIENT_H

#include "redisclientimpl.h"


namespace redisclient {
class RedisSyncClient:public RedisClientImpl{
public:
     RedisSyncClient(const shared_ptr<IOWorker>& worker);
     ~RedisSyncClient();

    // Connect to redis server
     bool connect(const NetAddr& addr);

    // disconnect from redis
     void disconnect();

     // Execute command on Redis server with the list of arguments.
     RedisValue command(const std::string& cmd, const std::deque<Value>& args,OperationResult &ec);

     RedisSyncClient &setConnectTimeout(int timeout);
     RedisSyncClient &setCommandTimeout(int timeout);
private:
	int					 connectimeout;
	int					 cmdtimeout;
};

}


#endif // REDISSYNCCLIENT_REDISCLIENT_H
