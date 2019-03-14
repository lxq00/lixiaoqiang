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
    : RedisClientImpl(_worker), connectimeout(10000),cmdtimeout(10000)
{
    
}

RedisSyncClient::~RedisSyncClient()
{
	disconnect();
}

bool RedisSyncClient::connect(const NetAddr& addr)
{
	shared_ptr<SyncCmdInfo> cmd = make_shared<SyncCmdInfo>();
	
	asyncConnect(addr, ConnectCallback(&SyncCmdInfo::connectCallack, cmd),DisconnectCallback());
	
	if (!cmd->wait(connectimeout)) return false;

	return true;
}

void RedisSyncClient::disconnect()
{
	close();
}
RedisValue RedisSyncClient::command(const std::string& cmdstr, const std::deque<Value>& args, OperationResult &ec)
{
	std::deque<Value> tmp = std::move(args);
	tmp.emplace_front(cmdstr);

	if (!isConnected())
	{
		ec = OperationResult(Operation_Error_NetworkErr, "对象未连接成功");
		return RedisValue();
	}

	shared_ptr<SyncCmdInfo> cmd = make_shared<SyncCmdInfo>();
	
	doAsyncCommand(RedisBuilder::makeCommand(tmp), CmdCallback(&SyncCmdInfo::recvCallback, cmd));

	if (!cmd->wait(cmdtimeout))
	{
		ec = OperationResult(Operation_Error_NetworkTimeOut, "通讯超时");
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

}
