#include "redissyncclient.h"

namespace redisclient {


RedisSyncClient::RedisSyncClient(const shared_ptr<IOWorker>& _worker)
    : RedisSyncClientImpl(_worker), connectimeout(10000),cmdtimeout(10000)
{
    
}

RedisSyncClient::~RedisSyncClient()
{
	disconnect();
}

bool RedisSyncClient::connect(const NetAddr& addr)
{
	return RedisSyncClientImpl::connect(addr, connectimeout, cmdtimeout);
}

void RedisSyncClient::disconnect()
{
	RedisSyncClient::close();
}
RedisValue RedisSyncClient::command(const std::string& cmdstr, const std::deque<Value>& args, OperationResult &ec)
{
	/*{
		static Mutex mutex;
		static FILE* fd = NULL;
		{
			std::string strtmp = Value(Time::getCurrentMilliSecond()).readString() + ":";
			strtmp += cmdstr;
			if (args.size() > 0)
			{
				strtmp += " ";
				strtmp += args.front().readString();
			}
			strtmp += "\r\n";
			Guard locker(mutex);
			if (fd == NULL)
			{
				fd = fopen((File::getExcutableFileFullPath() + "/"+File::getExcutableFileName()+"---rediscmd.txt").c_str(), "wb+");
			}
			if (fd != NULL)
			{
				fwrite(strtmp.c_str(), strtmp.length(), 1, fd);
				fflush(fd);
			}
				
		}
	}*/

	std::deque<Value> tmp = std::move(args);
	tmp.emplace_front(cmdstr);

	if (!isConnected())
	{
		ec = OperationResult(Operation_Error_NetworkErr, "对象未连接成功");
		return RedisValue();
	}

	RedisValue redisval;

	if (!RedisSyncClientImpl::command(RedisBuilder::makeCommand(tmp), redisval))
	{
		ec = OperationResult(Operation_Error_NetworkTimeOut, "通讯超时");
		return RedisValue();
	}
	
	return std::move(redisval);
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
