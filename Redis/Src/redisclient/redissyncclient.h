#ifndef REDISSYNCCLIENT_REDISCLIENT_H
#define REDISSYNCCLIENT_REDISCLIENT_H

#include "redisclientimpl.h"


namespace redisclient {
class RedisSyncClient{
public:
     RedisSyncClient(const shared_ptr<IOWorker>& worker);
     ~RedisSyncClient();

    // Connect to redis server
     bool connect(const NetAddr& addr);

    // Return true if is connected to redis.
     bool isConnected() const;

    // disconnect from redis
     void disconnect();

     // Execute command on Redis server with the list of arguments.
     RedisValue command(const std::string& cmd, const std::deque<Value>& args,OperationResult &ec);

     RedisSyncClient &setConnectTimeout(int timeout);
     RedisSyncClient &setCommandTimeout(int timeout);
protected:
	void redisErrorCallback();
private:
	Mutex				 mutex;
	shared_ptr<IOWorker> worker;
	int					 connectimeout;
	int					 cmdtimeout;

	shared_ptr<RedisClientImpl> impl;
	shared_ptr<RedisClientImpl> impltmp;
};

}


#endif // REDISSYNCCLIENT_REDISCLIENT_H
