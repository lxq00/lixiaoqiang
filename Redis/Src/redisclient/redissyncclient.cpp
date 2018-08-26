#include "redissyncclient.h"

namespace redisclient {

struct SyncCmdInfo
{
	Semaphore sem;
	RedisValue	retval;
	bool wait(int timeout) 
	{
		return sem.pend(timeout) >= 0;
	}

	void recvCallback(const RedisValue& val)
	{
		retval = std::move(val);
		sem.post();
	}
	void connectCallack()
	{
		sem.post();
	}
};

RedisSyncClient::RedisSyncClient(const shared_ptr<IOWorker>& _worker)
    : worker(_worker), connectimeout(10000),cmdtimeout(10000)
{
    
}


RedisSyncClient::~RedisSyncClient()
{
	disconnect();
}

bool RedisSyncClient::connect(const NetAddr& addr)
{
	impltmp = NULL;
	Guard locker(mutex);
	impl = NULL;

	shared_ptr<SyncCmdInfo> cmd = make_shared<SyncCmdInfo>();
	shared_ptr<RedisClientImpl> impltmp = make_shared<RedisClientImpl>(worker);
	
	impltmp->asyncConnect(addr, ConnectCallback(&SyncCmdInfo::connectCallack, cmd), ErrorCallback(&RedisSyncClient::redisErrorCallback, this));
	
	if (!cmd->wait(connectimeout)) return false;
	impl = impltmp;

	return true;
}
bool RedisSyncClient::isConnected() const
{
	shared_ptr<RedisClientImpl> impltmp = impl;
	if (impltmp == NULL) return false;

	return impltmp->connected();
}

void RedisSyncClient::disconnect()
{
	shared_ptr<RedisClientImpl> impltmp;
	{
		Guard locker(mutex);
		impltmp = impl;
		impl = NULL;
	}
	impltmp = NULL;
}
RedisValue RedisSyncClient::command(const std::string& cmdstr, const std::deque<Value>& args, OperationResult &ec)
{
	impltmp = NULL;

	std::deque<Value> tmp = std::move(args);
	tmp.emplace_front(cmdstr);

	Guard locker(mutex);

	if (impl == NULL)
	{
		ec = OperationResult("对象未连接成功");
		return RedisValue();
	}

	shared_ptr<SyncCmdInfo> cmd = make_shared<SyncCmdInfo>();
	
	impl->doAsyncCommand(RedisBuilder::makeCommand(tmp), CmdCallback(&SyncCmdInfo::recvCallback, cmd));

	if (!cmd->wait(cmdtimeout))
	{
		ec = OperationResult("通讯超时");
		return RedisValue();
	}
	
	return std::move(cmd->retval);
}
RedisSyncClient & RedisSyncClient::setConnectTimeout(int timeout)
{
	connectimeout = timeout;

	return *this;
}
RedisSyncClient & RedisSyncClient::setCommandTimeout(int timeout)
{
	cmdtimeout = timeout;

	return *this;
}
void RedisSyncClient::redisErrorCallback()
{
	assert(0);
	impltmp = impl;
	impl = NULL;
}
}
