#include "RedisDefine.h"
#include "Redis/Redis.h"

namespace Public {
namespace Redis {

struct RedisSet::RedisSetInternal
{
	weak_ptr<Redis_Client>	client;
	std::string				key;
	int						index;
};
RedisSet::RedisSet(const shared_ptr<Redis_Client>& client, const std::string& setname, int index)
{
	internal = new RedisSetInternal();
	internal->client = client;
	internal->key = setname;
	internal->index = index;
}
RedisSet::~RedisSet()
{
	SAFE_DELETE(internal);
}
uint32_t RedisSet::size()
{
	shared_ptr<Redis_Client> client = internal->client.lock();
	if (client == NULL) return 0;

	RedisValue retval;
	if (!client->command(internal->index, "SCARD", { String::ansi2utf8(internal->key) }, &retval))
	{
		return 0;
	}

	return (uint32_t)retval.toInt();
}
bool RedisSet::insert(const Value& val)
{
	vector<Value> vals;
	vals.push_back(val);

	return insert(vals);
}
bool RedisSet::insert(const vector<Value>& vals)
{
	if (vals.size() > 0)
	{
		std::deque<Value> args;
		args.push_back(String::ansi2utf8(internal->key));
		
		for (uint32_t i = 0; i < vals.size(); i++)
		{
			args.push_back(String::ansi2utf8(vals[i].readString()));
		}

		shared_ptr<Redis_Client> client = internal->client.lock();
		if (client == NULL) return false;

		if (!client->command(internal->index, "SADD", args))
		{
			return false;
		}
	}
	return true;
}
bool RedisSet::remove(const Value& val)
{
	vector<Value> vals;
	vals.push_back(val);

	return remove(vals);
}
bool RedisSet::remove(const vector<Value>& vals)
{
	{
		std::deque<Value> args;
		args.push_back(String::ansi2utf8(internal->key));

		for (uint32_t i = 0; i < vals.size(); i++)
		{
			args.push_back(String::ansi2utf8(vals[i].readString()));
		}

		shared_ptr<Redis_Client> client = internal->client.lock();
		if (client == NULL) return false;

		if (!client->command(internal->index, "SREM", args))
		{
			return false;
		}
	}
	return true;
}
vector<Value> RedisSet::members()
{
	shared_ptr<Redis_Client> client = internal->client.lock();
	if (client == NULL) return vector<Value>();

	RedisValue retval;
	if (!client->command(internal->index, "SMEMBERS", { String::ansi2utf8(internal->key) }, &retval))
	{
		return vector<Value>();
	}

	std::vector<RedisValue> tmp;
	if (retval.isArray()) tmp = retval.toArray();

	vector<Value> values;
	for (uint32_t i = 0; i < tmp.size(); i++)
	{
		values.push_back(transValue(tmp[i]));
	}

	return std::move(values);
}

bool RedisSet::exists(const Value& val)
{
	shared_ptr<Redis_Client> client = internal->client.lock();
	if (client == NULL) return false;

	RedisValue retval;
	if (!client->command(internal->index, "SISMEMBER", { String::ansi2utf8(internal->key),String::ansi2utf8(val.readString()) }, &retval))
	{
		return false;
	}

	return retval.toInt() != 0;
}

}
}