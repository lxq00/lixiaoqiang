#include "redisasyncclient.h"

namespace redisclient {

RedisAsyncClient::RedisAsyncClient(const shared_ptr<IOWorker>& worker):RedisAsyncClientImpl(worker)
{
}

RedisAsyncClient::~RedisAsyncClient()
{
	close();
}

void RedisAsyncClient::command(const std::string &cmd, const std::deque<Value>& args, const CmdCallback& callback)
{
	std::deque<Value> argstmp = std::move(args);
	argstmp.emplace_front(cmd);

	doAsyncCommand(RedisBuilder::makeCommand(argstmp), callback);
}

MQHandle RedisAsyncClient::subscribe(const std::string &channelName, const CmdCallback& msgHandler)
{
	return RedisAsyncClientImpl::subscribe("subscribe", channelName,msgHandler,CmdCallback());
}

void RedisAsyncClient::unsubscribe(const std::string &channelName, MQHandle handle)
{
	return RedisAsyncClientImpl::unsubscribe("subscribe", channelName, handle);
}

void RedisAsyncClient::publish(const std::string &channel, const Value &msg)
{
	static const std::string publishStr = "PUBLISH";

	std::deque<Value> items(3);

	items[0] = publishStr;
	items[1] = channel;
	items[2] = msg;

	return doAsyncCommand(RedisBuilder::makeCommand(items), CmdCallback());
}

}
