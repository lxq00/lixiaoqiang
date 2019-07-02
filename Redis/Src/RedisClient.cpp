#include "RedisDefine.h"
#include "Redis/Redis.h"
#include "Base/Base.h"
namespace Public {
namespace Redis {

struct Redis_Client::Redis_ClientInternal :public RedisSyncDbParam, public Thread
{
	struct RedisCommand
	{
		int index;
		std::string cmd;
		std::deque<Value> args;
		shared_ptr<RedisValue> resp;
		bool ret;
		Public::Base::Semaphore sem;
	};
	typedef std::list<shared_ptr<RedisCommand> > RedisList;
	RedisList queueRedis;
	Mutex mutex;
	Public::Base::Semaphore queueSem;
public:
	bool selectIndex(int _index)
	{
		if (index == _index) return true;

		index = _index;
		OperationResult ec;
		client->command("SELECT", { Value(index).readString() }, ec);
		if (!ec)
		{
			client->disconnect();
			return false;
		}
		return true;
	}
	bool initSyncClient()
	{
		if (client == NULL || !client->isConnected())
		{
			{
				client = make_shared<RedisSyncClient>(worker);
				client->setConnectTimeout(5000);
				client->setCommandTimeout(5000);
                index = 0;
			}

			if (!client->connect(redisaddr)) return false;

			if (password != "")
			{
				OperationResult ec;
				client->command("AUTH", { password }, ec);
				if (!ec)
				{
					client->disconnect();
					return false;
				}
			}
		}

		return true;
	}
private:
	void threadProc()
	{
		while (looping())
		{
			queueSem.pend(1000);
			
			initSyncClient();

			if (queueRedis.size() <= 0)
			{
				continue;
			}
			
			shared_ptr<RedisCommand> command;
			{
				Guard locker(mutex);
				command = queueRedis.front();
				queueRedis.pop_front();
			}
			command->ret = doCommand(command->index, command->cmd, command->args, command->resp.get());
			command->sem.post();
            if (!command->ret)
            {
                client = NULL;
            }
		}
	}
public:
	Redis_ClientInternal() :Thread("Redis Queue Thread")
	{}

	bool start(const shared_ptr<IOWorker>& _worker, const NetAddr& _addr, const std::string& _password)
	{
		worker = _worker;
		redisaddr = _addr;
		password = _password;
		index = 0;

		if (worker == NULL) worker = IOWorker::defaultWorker();
		if (!initSyncClient()) return false;
		
		createThread();

		return true;
	}
	void stop()
	{
		destroyThread();

		if (client != NULL)
		{
			OperationResult ec;
			client->command("QUIT", std::deque<Value>(), ec);
			client->disconnect();
		}
		client = NULL;
	}

	bool command(int index, const std::string& cmd, const std::deque<Value>& args, void* retvalptr)
	{
		shared_ptr<RedisCommand> command = make_shared<RedisCommand>();
		command->index = index;
		command->cmd = cmd;
		command->args = args;
		command->resp = make_shared<RedisValue>();
		{
			Guard locker(mutex);
			queueRedis.push_back(command);
			queueSem.post();
		}

		if (queueRedis.size() > 500)
		{
			logerror("queueRedis.size(%d) > 500 ", queueRedis.size());
		}

		if (0 > command->sem.pend(3000))
		{
			logerror("redis command timeout!");
			return false;
		}
		RedisValue tmp;
		RedisValue& retval = retvalptr == NULL ? tmp : *(RedisValue*)retvalptr;
		retval = *command->resp;
		return command->ret;
	}

	bool doCommand(int index, const std::string& cmd, const std::deque<Value>& args, void* retvalptr)
	{
		RedisValue tmp;
		RedisValue& retval = retvalptr == NULL ? tmp : *(RedisValue*)retvalptr;
		OperationResult ec;

		if (!selectIndex(index))
		{
			return false;
		}

		retval = client->command(cmd, args, ec);

		return ec && !retval.isError();
	}
	~Redis_ClientInternal()
	{
		stop();
	}
};

Redis_Client::Redis_Client()
{
	internal = new Redis_ClientInternal();
}
Redis_Client::~Redis_Client()
{
	SAFE_DELETE(internal);
}
bool Redis_Client::init(const shared_ptr<IOWorker>& worker, const NetAddr& addr, const std::string& password)
{
	return internal->start(worker, addr, password);
}

bool Redis_Client::uninit()
{
	internal->stop();

	return true;
}

bool Redis_Client::ping()
{
	RedisValue value;

	if (!command(0, "PING", std::deque<Value>(), &value))
	{
		return false;
	}

	std::string respstr = value.toString();
	if (strstr(respstr.c_str(), "PONG") == NULL)
	{
		logerror("ping() fail!\r\n");
		internal->client->disconnect();
		return false;
	}

	return true;
}
bool Redis_Client::command(int index, const std::string& cmd, const std::deque<Value>& args, void* retvalptr)
{
	return internal->command(index, cmd, args, retvalptr);
}
 
}
}

