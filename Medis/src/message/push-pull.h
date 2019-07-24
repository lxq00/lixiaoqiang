#pragma once
#include "pub-sub.h"

class Push_Pull
{
	struct PullInfo
	{
		void*				user;
		CmdMessageCallback	callback;
	};
public:
	Push_Pull() : pullindex(0){}
	~Push_Pull() {}

	bool pull(void* user, const CmdMessageCallback& callback)
	{
		for (uint32_t i = 0; i < pulllist.size(); i++)
		{
			if (pulllist[i].user == user) return true;
		}

		PullInfo info;
		info.user = user;
		info.callback = callback;

		pulllist.push_back(info);

		return true;
	}
	bool unpull(void* user)
	{
		for (std::vector<PullInfo>::iterator iter = pulllist.begin(); iter != pulllist.end(); iter++)
		{
			if (iter->user== user)
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

		pulllist[sendindex].callback(pulllist[sendindex].user, val);
	}
private:
	CmdMessageCallback		callback;
	std::vector<PullInfo>	pulllist;
	uint64_t				pullindex;
};