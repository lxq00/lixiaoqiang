#include "RedisDefine.h"
#include "Redis/Redis.h"


namespace Public {
namespace Redis {

struct RedisKey::RedisKeyInternal
{
	shared_ptr<Redis_Client>	client;
};
RedisKey::RedisKey(const shared_ptr<Redis_Client>& client)
{
	internal = new RedisKeyInternal();
	internal->client = client;
}

RedisKey::~RedisKey()
{
	SAFE_DELETE(internal);
}
bool RedisKey::setnx(int index,const std::string& key, const Value& val)
{
	shared_ptr<Redis_Client> client = internal->client;
	if (client == NULL) return false;

	RedisValue retval;
	if (!client->command(index, "SETNX", { String::ansi2utf8(key),String::ansi2utf8(val.readString()) }, &retval))
	{
		return false;
	}

	return retval.toInt() != 0;
}
bool RedisKey::expire(int index, const std::string& key, int ttl_ms)
{
	shared_ptr<Redis_Client> client = internal->client;
	if (client == NULL) return false;

	RedisValue retval;
	if (!client->command(index, "PEXPIRE", { String::ansi2utf8(key),Value(ttl_ms).readString()}, &retval))
	{
		return false;
	}

	return retval.toInt() != 0;
}
bool RedisKey::set(int index, const std::string& key, const Value& val)
{
	shared_ptr<Redis_Client> client = internal->client;
	if (client == NULL) return false;

	if (!client->command(index, "SET", { String::ansi2utf8(key),String::ansi2utf8(val.readString()) }))
	{
		return false;
	}

	return true;
}
Value RedisKey::get(int index, const std::string& key)
{
	shared_ptr<Redis_Client> client = internal->client;
	if (client == NULL) return Value();

	RedisValue retval;
	if (!client->command(index, "GET", { String::ansi2utf8(key) }, &retval))
	{
		return Value();
	}

	return std::move(transValue(retval));
}
bool RedisKey::exists(int index,const std::string& key)
{
	shared_ptr<Redis_Client> client = internal->client;
	if (client == NULL) return false;

	RedisValue retval;
	if (!client->command(index, "EXISTS", { String::ansi2utf8(key) }, &retval))
	{
		return false;
	}

	return retval.toInt() != 0;
}
bool RedisKey::del(int index,const std::string& key)
{
	std::vector<std::string> keys;
	keys.push_back(key);

	return del(index,keys);
}
bool RedisKey::del(int index,const std::vector<std::string>& keys)
{
	if (keys.size() > 0)
	{
		std::deque<Value> args;
		for (uint32_t i = 0; i < keys.size(); i++)
		{
			args.push_back(String::ansi2utf8(keys[i]));
		}

		shared_ptr<Redis_Client> client = internal->client;
		if (client == NULL) return false;

		if (!client->command(index, "DEL", args))
		{
			return false;
		}
	}

	return true;
}
std::vector<std::string> RedisKey::keys(int index,const std::string& pattern)
{
	shared_ptr<Redis_Client> client = internal->client;
	if (client == NULL) return std::vector<std::string>();

	RedisValue retval;
	if (!client->command(index, "KEYS", { String::ansi2utf8(pattern) }, &retval))
	{
		return std::vector<std::string>();
	}

	std::vector<RedisValue> tmp;
	if (retval.isArray()) tmp = retval.toArray();

	vector<std::string> values;
	for (uint32_t i = 0; i < tmp.size(); i++)
	{
		values.push_back(transValue(tmp[i]).readString());
	}

	return std::move(values);
}
bool RedisKey::rename(int index,const std::string& oldkey, const std::string& newkey)
{
	shared_ptr<Redis_Client> client = internal->client;
	if (client == NULL) return false;

	if (!client->command(index, "RENAME", { String::ansi2utf8(oldkey),String::ansi2utf8(newkey) }))
	{
		return false;
	}

	return true;
}
int RedisKey::len(int index,const std::string& key)
{
	shared_ptr<Redis_Client> client = internal->client;
	if (client == NULL) return 0;

	RedisValue retval;
	if (!client->command(index, "STRLEN", { String::ansi2utf8(key) }, &retval))
	{
		return 0;
	}

	return retval.isInt() ? (int)retval.toInt() : 0;
}

int RedisKey::incr(int index,const std::string& key, int val)
{
	shared_ptr<Redis_Client> client = internal->client;
	if (client == NULL) return 0;

	RedisValue retval;
	if (val == 1)
	{
		if (!client->command(index, "INCR", { String::ansi2utf8(key) }, &retval))
		{
			return 0;
		}
	}
	else
	{
		if (!client->command(index, "INCRBY", { String::ansi2utf8(key),Value(val).readString() }, &retval))
		{
			return 0;
		}
	}
	
	return retval.isInt() ? (int)retval.toInt() : 0;
}
int RedisKey::decr(int index,const std::string& key, int val)
{
	shared_ptr<Redis_Client> client = internal->client;
	if (client == NULL) return 0;
	
	RedisValue retval;
	if (val == 1)
	{
		if (!client->command(index, "DECR", { String::ansi2utf8(key) }, &retval))
		{
			return 0;
		}
	}
	else
	{
		if (!client->command(index, "DECRBY", { String::ansi2utf8(key),Value(val).readString() }, &retval))
		{
			return 0;
		}
	}

	return retval.isInt() ? (int)retval.toInt() : 0;
}
}
}