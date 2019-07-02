#include "RedisDefine.h"
#include "Redis/Redis.h"
namespace Public {
namespace Redis {

struct RedisSortedSet::RedisSortedSetInternal
{
	shared_ptr<Redis_Client>	client;
	std::string				key;
	int						index;
};
RedisSortedSet::RedisSortedSet(const shared_ptr<Redis_Client>& client, const std::string& setname, int index)
{
	internal = new RedisSortedSetInternal();
	internal->client = client;
	internal->key = setname;
	internal->index = index;
}

RedisSortedSet::~RedisSortedSet()
{
	SAFE_DELETE(internal);
}
bool RedisSortedSet::push_back(const Value& key, const Value&val)
{
	shared_ptr<Redis_Client> client = internal->client;
	if (client == NULL) return false;

	if (!client->command(internal->index, "ZADD", { String::ansi2utf8(internal->key) ,String::ansi2utf8(key),String::ansi2utf8(val.readString()) }))
	{
		return false;
	}

	return true;
}
std::vector<Value> RedisSortedSet::values(int startpos, int stoppos)
{
	shared_ptr<Redis_Client> client = internal->client;
	if (client == NULL) return vector<Value>();

	RedisValue retval;
	if (!client->command(internal->index, "ZRANGE", { String::ansi2utf8(internal->key),Value(startpos).readString(),Value(stoppos).readString("%d") }, &retval))
	{
		return vector<Value>();
	}

	if(!retval.isArray()) { return std::vector<Value>(); }

	const std::vector<RedisValue> retarray = retval.getArray();

	std::vector<Value> values;
	for (uint32_t i = 0; retval.isArray() && i < retarray.size(); i++)
	{
		values.push_back(transValue(retarray[i]));
	}

	return std::move(values);
}
Value RedisSortedSet::front()
{
	shared_ptr<Redis_Client> client = internal->client;
	if (client == NULL) return Value();

	RedisValue retval;
	if (!client->command(internal->index, "ZRANGE", { String::ansi2utf8(internal->key),0,0 }, &retval))
	{
		return Value();
	}

	if(!retval.isArray()) return Value();

	std::vector<RedisValue> vals = retval.getArray();
	if (vals.size() <= 0) return Value();

	return std::move(transValue(vals[0]));
}
bool RedisSortedSet::del(const Value& val)
{
	shared_ptr<Redis_Client> client = internal->client;
	if (client == NULL) return false;

	if (!client->command(internal->index, "ZREM", { String::ansi2utf8(internal->key),String::ansi2utf8(val.readString()) }))
	{
		return false;
	}

	return true;
}
uint32_t RedisSortedSet::size()
{
	shared_ptr<Redis_Client> client = internal->client;
	if (client == NULL) return 0;

	RedisValue retval;
	if (!client->command(internal->index, "ZCARD", { String::ansi2utf8(internal->key)}, &retval))
	{
		return 0;
	}

	return (uint32_t)retval.toInt();
}

}
}