#ifndef REDISASYNCCLIENT_REDISCLIENT_H
#define REDISASYNCCLIENT_REDISCLIENT_H


#include "redisclientimpl.h"

namespace redisclient {

typedef void* MQHandle;

class RedisAsyncClient : boost::noncopyable {
public:
    RedisAsyncClient(const shared_ptr<IOWorker>& worker);
     ~RedisAsyncClient();

     void connect(const NetAddr& addr,const ConnectCallback& callback,const ErrorCallback& errcallback);

    // Return true if is connected to redis.
     bool isConnected() const;

    // disconnect from redis and clear command queue
     void disconnect();

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
private:
    std::shared_ptr<RedisClientImpl> pimpl;
};

}

#endif // REDISASYNCCLIENT_REDISCLIENT_H
