#ifndef REDISASYNCCLIENT_REDISCLIENT_H
#define REDISASYNCCLIENT_REDISCLIENT_H


#include "redisasyncclientimpl.h"

namespace redisclient {

typedef void* MQHandle;

class RedisAsyncClient : public RedisAsyncClientImpl {
public:
    RedisAsyncClient(const shared_ptr<IOWorker>& worker);
	virtual ~RedisAsyncClient();

     // Execute command on Redis server with the list of arguments.
     void command(const std::string &cmd,const std::deque<Value>& args,const CmdCallback& callback = CmdCallback());

    // Subscribe to channel. Handler msgHandler will be called
    // when someone publish message on channel. Call unsubscribe 
    // to stop the subscription.
	 MQHandle subscribe(const std::string &channelName,const CmdCallback& msgHandler);

    // Unsubscribe
     void unsubscribe(const std::string &channelName, MQHandle handle);
     
    // Publish message on channel.
     void publish(const std::string &channel, const Value &msg);
};

}

#endif // REDISASYNCCLIENT_REDISCLIENT_H
