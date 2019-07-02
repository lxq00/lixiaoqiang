#pragma once

#include "exchange.h"

class Message
{
public:
	Message(const shared_ptr<IOWorker>& worker, const CmdMessageCallback& _callback)
	{
		for (uint32_t i = 0; i < MAXDBINDEX; i++)
		{
			exchangelist.push_back(make_shared<Exchange>(worker, _callback));
		}
	}
	~Message() {}
	bool inputCommand(const CmdResultCallback& callback, void* user, uint32_t dbindex,const shared_ptr<RedisValue>& value)
	{
		if (dbindex < 0 || dbindex >= MAXDBINDEX)
		{
			return false;
		}

		return exchangelist[dbindex]->inputCommand(callback, user, value);
	}
	void userOffline(void* user)
	{
		for (uint32_t i = 0; i < MAXDBINDEX; i++)
		{
			exchangelist[i]->userOffline(user);
		}
	}
private:
	std::vector<shared_ptr<Exchange> > exchangelist;
};