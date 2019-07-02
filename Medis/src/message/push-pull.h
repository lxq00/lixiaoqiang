#pragma once
#include "pub-sub.h"

class Push_Pull
{
public:
	Push_Pull(const CmdMessageCallback& _callback) :callback(_callback), pullindex(0){}
	~Push_Pull() {}

	bool pull(void* user)
	{
		for (uint32_t i = 0; i < pulllist.size(); i++)
		{
			if (pulllist[i] == user) return true;
		}

		pulllist.push_back(user);

		return true;
	}
	bool unpull(void* user)
	{
		for (std::vector<void*>::iterator iter = pulllist.begin(); iter != pulllist.end(); iter++)
		{
			if (*iter == user)
			{
				pulllist.erase(iter);
				return true;
			}
		}

		return false;
	}
	uint32_t pullsize()
	{
		return pulllist.size();
	}
	void push(const RedisValue& val)
	{
		if (pulllist.size() == 0) return;

		uint32_t sendindex = (pullindex++) % pulllist.size();

		callback(pulllist[sendindex], val);
	}
private:
	CmdMessageCallback	callback;
	std::vector<void*>	pulllist;
	uint64_t			pullindex;
};