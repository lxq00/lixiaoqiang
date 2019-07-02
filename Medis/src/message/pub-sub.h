#pragma once

#include "../common/redisvalue.h"

typedef Function2<void, void*, const RedisValue&> CmdMessageCallback;

class Pub_Sub
{
public:
	Pub_Sub(const CmdMessageCallback& _callback):callback(_callback) {}
	~Pub_Sub() {}

	bool subscribe(void* user)
	{
		sublist.insert(user);
		return true;
	}
	bool unsubcribe(void* user)
	{
		std::set<void*>::iterator iter = sublist.find(user);
		if (iter == sublist.end()) return false;

		sublist.erase(iter);

		return true;
	}
	uint32_t subscribesize()
	{
		return sublist.size();
	}
	void publish(const RedisValue& msg)
	{
		for (std::set<void*>::iterator iter = sublist.begin(); iter != sublist.end(); iter++)
		{
			callback(*iter, msg);
		}
	}
private:
	CmdMessageCallback	callback;
	std::set<void*>		sublist;
};