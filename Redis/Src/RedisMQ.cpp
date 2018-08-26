#include "RedisDefine.h"
#include "Redis/Redis.h"
namespace Public {
namespace Redis {

struct RedisMQ::RedisMQInternal
{
	struct SubHandlerInfo
	{
		MQHandle		subhandle;
		std::string     channel;
		MessageHandler	handler;
		void onMessage(const RedisValue &buf)
		{
			handler(channel, buf.toString());
		}
	};
	struct AsyncSocketInfo
	{
		typedef enum {
			AsyncEvent_Init,
			AsyncEvent_Connecting,
			AsyncEvent_ConnectEnd,
			AsyncEvent_Authening,
			AsyncEvent_AuthenEnd,
			AsyncEvent_Selecting,
			AsyncEvent_Success,
		}AsyncEvent;

		std::string						password;
		AsyncEvent						event;
		int								index;
		NetAddr							redisaddr;
		shared_ptr<IOWorker>			worker;
	public:
		AsyncSocketInfo(const shared_ptr<IOWorker>& _worker, const NetAddr& addr, const std::string& passwd,int id)
		{
			worker = _worker;
			event = AsyncEvent_Init;
			password = passwd;
			index = id;
			redisaddr = addr;
		}
		~AsyncSocketInfo() {}
		void initSocket()
		{
			Guard locker(mutex);
			if (event == AsyncEvent_Init)
			{
				event = AsyncEvent_Connecting;
				client = make_shared<RedisAsyncClient>(worker);
				client->connect(redisaddr, ConnectCallback(&AsyncSocketInfo::connectCallback, this), ErrorCallback(&AsyncSocketInfo::disconnectCallback, this));
			}
			if (event == AsyncEvent_ConnectEnd)
			{
				event = AsyncEvent_Authening;

				client->command("AUTH", { password }, CmdCallback(&AsyncSocketInfo::authenCallback, this));
			}
			if (event == AsyncEvent_AuthenEnd)
			{
				event = AsyncEvent_Selecting;

				client->command("SELECT", { Value(index).readString() }, CmdCallback(&AsyncSocketInfo::selectCallback, this));
			}
		}
		bool success()
		{
			return event == AsyncEvent_Success;
		}
	private:
		void connectCallback()
		{
			Guard locker(mutex);
			event = password == "" ? AsyncEvent_AuthenEnd : AsyncEvent_ConnectEnd;
		}
		void disconnectCallback()
		{
			Guard locker(mutex);
			event = AsyncEvent_Init;
		}
		void authenCallback(const redisclient::RedisValue &v)
		{
			Guard locker(mutex);
			if (v.toString() == "OK") event = AsyncEvent_AuthenEnd;
			else event = AsyncEvent_Init;
		}
		void selectCallback(const redisclient::RedisValue &v) 
		{
			Guard locker(mutex);
			if (v.toString() == "OK")
			{
				for (std::map<std::string, shared_ptr<SubHandlerInfo> >::iterator iter = sublist.begin(); iter != sublist.end(); iter++)
				{
					iter->second->subhandle = client->subscribe(iter->first, SubcribeHandler(&SubHandlerInfo::onMessage, iter->second));
				}

				event = AsyncEvent_Success;
			}
			else event = AsyncEvent_Init;
		}
	public:
		Mutex							mutex;
		std::map<std::string, shared_ptr<SubHandlerInfo> > sublist;
		shared_ptr<RedisAsyncClient>	client;
	};
public:
	shared_ptr<AsyncSocketInfo>		publisher;
	shared_ptr<AsyncSocketInfo>		subscriber;

	shared_ptr<Timer>			pooltimer;
	

	
	void onPoolTimerProc(unsigned long)
	{
		publisher->initSocket();
		subscriber->initSocket();
	}
public:
	RedisMQInternal(RedisSyncDbParam* param, int index)
	{
		publisher = make_shared<AsyncSocketInfo>(param->worker, param->redisaddr, param->password,index);
		subscriber = make_shared<AsyncSocketInfo>(param->worker, param->redisaddr, param->password, index);

		pooltimer = make_shared<Timer>("Redis_ClientInternal");
		pooltimer->start(Timer::Proc(&RedisMQInternal::onPoolTimerProc, this), 0, 1000);
	}
	~RedisMQInternal()
	{
		publisher = NULL;
		subscriber = NULL;
		pooltimer = NULL;
		publisher = NULL;
		subscriber = NULL;
	}
};
RedisMQ::RedisMQ(const shared_ptr<Redis_Client>& client, int index)
{
	internal = new RedisMQInternal((RedisSyncDbParam*)client->param(),index);
}
RedisMQ::~RedisMQ()
{
	SAFE_DELETE(internal);
}

void RedisMQ::publish(const std::string& channelName, const std::string& value)
{
	std::string flag = String::tolower(channelName);

	shared_ptr<RedisAsyncClient> tmp = internal->publisher->client;
	if(tmp != NULL)
		tmp->publish(flag, value);
}
void RedisMQ::subscrib(const std::string& channelName, const MessageHandler& handler)
{
	std::string flag = String::tolower(channelName);

	Guard locker(internal->subscriber->mutex);
	std::map<std::string, shared_ptr<RedisMQInternal::SubHandlerInfo> >::iterator iter = internal->subscriber->sublist.find(flag);
	if (iter == internal->subscriber->sublist.end())
	{
		shared_ptr<RedisMQInternal::SubHandlerInfo> sub = make_shared<RedisMQInternal::SubHandlerInfo>();
		sub->channel = channelName;
		if (internal->subscriber->success())
		{
			sub->subhandle =  internal->subscriber->client->subscribe(flag, SubcribeHandler(&RedisMQInternal::SubHandlerInfo::onMessage, sub));
		}
		internal->subscriber->sublist[flag] = sub;
		iter = internal->subscriber->sublist.find(flag);
	}
	iter->second->handler = handler;
}
void RedisMQ::unsubscrib(const std::string& channelName)
{
	std::string flag = String::tolower(channelName);

	Guard locker(internal->subscriber->mutex);
	std::map<std::string, shared_ptr<RedisMQInternal::SubHandlerInfo> >::iterator iter = internal->subscriber->sublist.find(flag);
	if (iter != internal->subscriber->sublist.end())
	{
		if (internal->subscriber->success())
		{
			internal->subscriber->client->unsubscribe(flag, iter->second->subhandle);
		}
		internal->subscriber->sublist.erase(iter);
	}
}

}
}

