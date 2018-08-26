#include "RedisDefine.h"
#include "Redis/Redis.h"

namespace Public {
namespace Redis {

struct RedisList::RedisListInternal 
{
	weak_ptr<Redis_Client>	client;
	std::string				key;
	int						index;
};
RedisList::RedisList(const shared_ptr<Redis_Client>& client, const std::string& listname, int index)
{
	internal = new RedisListInternal();
	internal->client = client;
	internal->key = listname;
	internal->index = index;
}
RedisList::~RedisList()
{
	SAFE_DELETE(internal);
}
uint32_t RedisList::size()
{
	shared_ptr<Redis_Client> client = internal->client.lock();
	if (client == NULL) return 0;

	RedisValue retval;
	if (!client->command(internal->index, "HLEN", { String::ansi2utf8(internal->key) }, &retval))
	{
		return 0;
	}

	return (uint32_t)retval.toInt();
}
bool RedisList::push_back(const Value& val)
{
	shared_ptr<Redis_Client> client = internal->client.lock();
	if (client == NULL) return 0;

	if (!client->command(internal->index, "LPUSH", { String::ansi2utf8(internal->key),String::ansi2utf8(val.readString()) }))
	{
		return false;
	}

	return true;
}
Value RedisList::pop()
{
	shared_ptr<Redis_Client> client = internal->client.lock();
	if (client == NULL) return Value();

	RedisValue retval;
	if (!client->command(internal->index, "LPOP", { String::ansi2utf8(internal->key) }, &retval))
	{
		return Value();
	}

	return std::move(transValue(retval));
}


}
}