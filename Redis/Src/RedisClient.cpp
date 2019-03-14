#include "RedisDefine.h"
#include "Redis/Redis.h"
namespace Public {
namespace Redis {

struct Redis_Client::Redis_ClientInternal :public RedisSyncDbParam
{
public:
	shared_ptr<Timer>			pooltimer;

	void onPoolTimerProc(unsigned long)
	{
		Guard locker(mutex);
		_initSyncClient();
	}
	bool _selectIndex(int _index)
	{
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
	bool _initSyncClient()
	{
		if (client == NULL)
		{
			client = make_shared<RedisSyncClient>(worker);
			client->setConnectTimeout(5000);
			client->setCommandTimeout(5000);
		}
		if (!client->isConnected())
		{
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
			{
				OperationResult ec;
				RedisValue ret = client->command("PING", { }, ec);
				std::string respstr = ret.toString();
				if (!ec ||strstr(respstr.c_str(),"PONG") == NULL)
				{
					client->disconnect();
					return false;
				}
			}
			_selectIndex(index);
		}

		return true;
	}

public:
	Redis_ClientInternal() { mutex = make_shared<Mutex>(); }
	bool start(const shared_ptr<IOWorker>& _worker, const NetAddr& _addr, const std::string& _password)
	{
		Guard locker(mutex);

		worker = _worker;
		redisaddr = _addr;
		password = _password;
		index = 0;

		if (worker == NULL) worker = IOWorker::defaultWorker();
		if (!_initSyncClient()) return false;

		pooltimer = make_shared<Timer>("Redis_ClientInternal");
		pooltimer->start(Timer::Proc(&Redis_ClientInternal::onPoolTimerProc, this), 0, 1000);

		return true;
	}
	void stop()
	{
		Guard locker(mutex);
		pooltimer = NULL;
		if (client != NULL)	client->disconnect();
		client = NULL;
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
	Guard locker(internal->mutex);
	return internal->client->isConnected();
}
bool Redis_Client::command(int index, const std::string& cmd, const std::deque<Value>& args, void* retvalptr)
{
	RedisValue tmp;
	RedisValue& retval = retvalptr == NULL ? tmp : *(RedisValue*)retvalptr;
	OperationResult ec;

	
	Guard locker(internal->mutex);
	if (!internal->client->isConnected()) return false;

	if (internal->index != index && !internal->_selectIndex(index))
	{
		return false;
	}


	retval = internal->client->command(cmd, args, ec);

	return ec && !retval.isError();
}
void* Redis_Client::param()
{
	return internal;
}

}
}

