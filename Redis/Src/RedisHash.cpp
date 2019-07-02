#include "RedisDefine.h"
#include "Redis/Redis.h"

namespace Public {
namespace Redis {

struct RedisHash::RedisHashInternal
{
	shared_ptr<Redis_Client>	client;
	std::string				key;
	int						index;
};
RedisHash::RedisHash(const shared_ptr<Redis_Client>& client, const std::string& hashname, int index)
{
	internal = new RedisHashInternal();
	internal->client = client;
	internal->key = hashname;
	internal->index = index;
}
RedisHash::~RedisHash()
{
	SAFE_DELETE(internal);
}
uint32_t RedisHash::size()
{
	shared_ptr<Redis_Client> client = internal->client;
	if (client == NULL) return 0;

	RedisValue retval;
	if (!client->command(internal->index, "HLEN", { String::ansi2utf8(internal->key) },&retval))
	{
		return 0;
	}

	return (uint32_t)retval.toInt();
}
bool RedisHash::exists(const std::string& key)
{
	shared_ptr<Redis_Client> client = internal->client;
	if (client == NULL) return false;

	RedisValue retval;
	if (!client->command(internal->index, "HEXISTS", { String::ansi2utf8(internal->key),String::ansi2utf8(key) },&retval))
	{
		return false;
	}

	return retval.toInt() != 0;
}
bool RedisHash::set(const std::string& key, const Value& val)
{
	shared_ptr<Redis_Client> client = internal->client;
	if (client == NULL) return false;

	if (!client->command(internal->index, "HSET", { String::ansi2utf8(internal->key),String::ansi2utf8(key),String::ansi2utf8(val.readString()) }))
	{
		return false;
	}

	return true;
}

bool RedisHash::set(const std::map<std::string, Value>& vals)
{
	if (vals.size() == 0) return true;

	std::deque<Value> args;
	args.push_back(String::ansi2utf8(internal->key));

	for (std::map<std::string, Value>::const_iterator iter = vals.begin();iter != vals.end();iter ++)
	{
		args.push_back(String::ansi2utf8(iter->first));
		args.push_back(String::ansi2utf8(iter->second.readString()));
	}

	shared_ptr<Redis_Client> client = internal->client;
	if (client == NULL) return false;

	if (!client->command(internal->index, "HMSET", args))
	{
		return false;
	}

	return true;
}

Value RedisHash::get(const std::string& key)
{
	shared_ptr<Redis_Client> client = internal->client;
	if (client == NULL) return Value();

	RedisValue retval;
	if (!client->command(internal->index, "HGET", { String::ansi2utf8(internal->key) ,String::ansi2utf8(key) }, &retval))
	{
		return Value();
	}

	return std::move(transValue(retval));
}

std::map<std::string, Value> RedisHash::get(const std::vector<std::string>& keys)
{
	if (keys.size() == 0) return std::map<std::string, Value>();

	std::deque<Value> args;
	args.push_back(String::ansi2utf8(internal->key));

	for (uint32_t i = 0; i < keys.size(); i++)
	{
		args.push_back(String::ansi2utf8(keys[i]));
	}

	std::map<std::string, Value> values;

	shared_ptr<Redis_Client> client = internal->client;
	if (client == NULL) return values;

	RedisValue retval;
	if (!client->command(internal->index, "HMGET", args, &retval))
	{
		return std::map<std::string, Value>();
	}
	if (!retval.isArray()) { return std::map<std::string, Value>(); }

	const std::vector<RedisValue>& retvalarray = retval.getArray();

	for (uint32_t i = 0; i < keys.size(); i++)
	{
		values[keys[i]] = retvalarray.size() < i ? Value() : transValue(retvalarray[i]);
	}

	return std::move(values);
}

bool RedisHash::remove(const std::string& key)
{
	std::vector<std::string> keys;
	keys.push_back(key);

	return remove(keys);
}

bool RedisHash::remove(const std::vector<std::string>& keys)
{
	if (keys.size() == 0) return true;

	std::deque<Value> args;
	args.push_back(String::ansi2utf8(internal->key));

	for (uint32_t i = 0; i < keys.size(); i++)
	{
		args.push_back(String::ansi2utf8(keys[i]));
	}

	shared_ptr<Redis_Client> client = internal->client;
	if (client == NULL) return false;

	if (!client->command(internal->index, "HDEL", args))
	{
		return false;
	}

	return true;
}

std::set<string> RedisHash::keys()
{
	shared_ptr<Redis_Client> client = internal->client;
	if (client == NULL) return std::set<string>();

	RedisValue retval;
	if (!client->command(internal->index, "HKEYS", { String::ansi2utf8(internal->key) }, &retval))
	{
		return std::set<string>();
	}

	if (!retval.isArray()) return std::set<string>();

	std::vector<RedisValue> tmp;
	if (retval.isArray()) tmp = retval.toArray();

	std::set<string> values;
	for (uint32_t i = 0; i < tmp.size(); i++)
	{
		if(tmp[i].isString())
			values.insert(String::utf82ansi(tmp[i].toString()));
	}

	return std::move(values);
}


}
}