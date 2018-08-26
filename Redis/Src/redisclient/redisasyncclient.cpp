#include "redisasyncclient.h"

namespace redisclient {

RedisAsyncClient::RedisAsyncClient(const shared_ptr<IOWorker>& worker)
{
	pimpl = make_shared<RedisClientImpl>(worker);
}

RedisAsyncClient::~RedisAsyncClient()
{
	disconnect();
	pimpl = NULL;
}

void RedisAsyncClient::connect(const NetAddr& addr, const ConnectCallback& callback, const ErrorCallback& errcallback)
{
	shared_ptr<RedisClientImpl> tmp = pimpl;

	tmp->asyncConnect(addr, callback, errcallback);
}


bool RedisAsyncClient::isConnected() const
{
	shared_ptr<RedisClientImpl> tmp = pimpl;

	return tmp->connected();
}

void RedisAsyncClient::disconnect()
{
    pimpl->close();
}

void RedisAsyncClient::command(const std::string &cmd, const std::deque<Value>& args, const CmdCallback& callback)
{
	shared_ptr<RedisClientImpl> tmp = pimpl;

	std::deque<Value> argstmp = std::move(args);
	argstmp.emplace_front(cmd);

	tmp->doAsyncCommand(RedisBuilder::makeCommand(argstmp), callback);
}

MQHandle RedisAsyncClient::subscribe(const std::string &channelName, const CmdCallback& msgHandler)
{
	shared_ptr<RedisClientImpl> tmp = pimpl;

	return tmp->subscribe("subscribe", channelName,msgHandler,CmdCallback());
}

void RedisAsyncClient::unsubscribe(const std::string &channelName, MQHandle handle)
{
	shared_ptr<RedisClientImpl> tmp = pimpl;

	return tmp->unsubscribe("subscribe", channelName, handle);
}

void RedisAsyncClient::publish(const std::string &channel, const Value &msg)
{
	static const std::string publishStr = "PUBLISH";

	std::deque<Value> items(3);

	items[0] = publishStr;
	items[1] = channel;
	items[2] = msg;

	shared_ptr<RedisClientImpl> tmp = pimpl;

	return tmp->doAsyncCommand(RedisBuilder::makeCommand(items), CmdCallback());
}

}
