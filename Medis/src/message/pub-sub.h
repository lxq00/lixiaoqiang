#pragma once

#include "../common/redisvalue.h"

typedef Function2<void, void*, const RedisValue&> CmdMessageCallback;

class Pub_Sub
{
public:
	Pub_Sub(){}
	~Pub_Sub() {}

	bool subscribe(void* user, const CmdMessageCallback& callback)
	{
		sublist[user] = callback;
		return true;
	}
	bool unsubscribe(void* user)
	{
		std::map<void*, CmdMessageCallback>::iterator iter = sublist.find(user);
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
		for (std::map<void*, CmdMessageCallback>::iterator iter = sublist.begin(); iter != sublist.end(); iter++)
		{
			iter->second(iter->first, msg);
		}
	}
private:
	std::map<void*,CmdMessageCallback>		sublist;
};