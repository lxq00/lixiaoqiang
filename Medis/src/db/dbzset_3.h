#pragma once

#include "dblist_2.h"


class DBZSet: public DBList
{
public:
	DBZSet(const shared_ptr<IOWorker>& ioworker, const shared_ptr<DataFactory>& factory, int dbindex) :DBList(ioworker,factory ,dbindex)
	{
		subCommand("ZADD", CommandCallback(&DBZSet::zaddFunc, this));
		subCommand("ZCARD", CommandCallback(&DBZSet::zcardFunc, this));
		subCommand("ZCOUNT", CommandCallback(&DBZSet::zcountFunc, this));
		subCommand("ZRANGE", CommandCallback(&DBZSet::zrangeFunc, this));
		subCommand("ZRANGEBYSCORE", CommandCallback(&DBZSet::zrangebyscoreFunc, this));
		subCommand("ZREMRANGEBYSCORE", CommandCallback(&DBZSet::zremrangebyscoreFunc, this));
	}
private:
	RedisValue zaddFunc(const std::vector<RedisValue> & val)
	{
		if (val.size() != 4) return RedisValue(false, "wrong number of arguments");

		std::string key = String::tolower(val[1].toString());
		int64_t score = val[2].toInt();
		const RedisString& data = val[3].getString();

		shared_ptr<ValueObject> valueobject;
		std::map<std::string, shared_ptr<ValueObject> >::iterator iter = valuelist.find(key);
		if (iter == valuelist.end())
		{
			valueobject = make_shared<ValueZSet>(factory, key, dbindex);
			valuelist[key] = valueobject;
		}
		else
		{
			valueobject = iter->second;
		}

		if (valueobject->type() != DataType_ZSet) return RedisValue(0);
		ValueZSet* zsetobject = (ValueZSet*)valueobject.get();

		bool ret = zsetobject->add(score, data);

		return RedisValue(ret ? 1 : 0);
	}

	RedisValue zcardFunc(const std::vector<RedisValue> & val)
	{
		if (val.size() != 2) return RedisValue(false, "wrong number of arguments");
		std::string key = String::tolower(val[1].toString());


		shared_ptr<ValueObject> valueobject;
		std::map<std::string, shared_ptr<ValueObject> >::iterator iter = valuelist.find(key);
		if (iter == valuelist.end())
		{
			return RedisValue(0);
		}
		else
		{
			valueobject = iter->second;
		}

		if (valueobject->type() != DataType_ZSet) return RedisValue(0);
		ValueZSet* zsetobject = (ValueZSet*)valueobject.get();

		return RedisValue(zsetobject->card());
	}

	RedisValue zcountFunc(const std::vector<RedisValue> & val)
	{
		if (val.size() != 4) return RedisValue(false, "wrong number of arguments");
		std::string key = String::tolower(val[1].toString());
		int64_t minscore = val[2].toInt();
		int64_t maxscore = val[3].toInt();

		shared_ptr<ValueObject> valueobject;
		std::map<std::string, shared_ptr<ValueObject> >::iterator iter = valuelist.find(key);
		if (iter == valuelist.end())
		{
			return RedisValue(0);
		}
		else
		{
			valueobject = iter->second;
		}

		if (valueobject->type() != DataType_ZSet) return RedisValue(0);
		ValueZSet* zsetobject = (ValueZSet*)valueobject.get();

		return RedisValue(zsetobject->count(minscore,maxscore));
	}

	RedisValue zrangeFunc(const std::vector<RedisValue> & val)
	{
		if (val.size() <= 3) return RedisValue(false, "wrong number of arguments");
		std::string key = String::tolower(val[1].toString());
		int64_t start = val[2].toInt();
		int64_t top = val[3].toInt();

		shared_ptr<ValueObject> valueobject;
		std::map<std::string, shared_ptr<ValueObject> >::iterator iter = valuelist.find(key);
		if (iter == valuelist.end())
		{
			return RedisValue(0);
		}
		else
		{
			valueobject = iter->second;
		}

		if (valueobject->type() != DataType_ZSet) return RedisValue(0);
		ValueZSet* zsetobject = (ValueZSet*)valueobject.get();

		std::map<uint64_t, RedisString> valdata;
		zsetobject->range(start, top, valdata);

		std::vector<RedisValue> dataarray;
		for (std::map<uint64_t, RedisString>::iterator iter = valdata.begin(); iter != valdata.end(); iter++)
		{
			dataarray.push_back(RedisValue(iter->second));
			dataarray.push_back(RedisValue(Value(iter->first).readString()));
		}

		return RedisValue(dataarray);
	}

	RedisValue zrangebyscoreFunc(const std::vector<RedisValue> & val)
	{
		if (val.size() <= 3) return RedisValue(false, "wrong number of arguments");
		std::string key = String::tolower(val[1].toString());
		int64_t minscore = val[2].toInt();
		int64_t maxscore = val[3].toInt();

		shared_ptr<ValueObject> valueobject;
		std::map<std::string, shared_ptr<ValueObject> >::iterator iter = valuelist.find(key);
		if (iter == valuelist.end())
		{
			return RedisValue(0);
		}
		else
		{
			valueobject = iter->second;
		}

		if (valueobject->type() != DataType_ZSet) return RedisValue(0);
		ValueZSet* zsetobject = (ValueZSet*)valueobject.get();

		std::map<uint64_t, RedisString> valdata;

		zsetobject->rangeByScore(minscore, maxscore, valdata);

		std::vector<RedisValue> dataarray;
		for (std::map<uint64_t, RedisString>::iterator iter = valdata.begin(); iter != valdata.end(); iter++)
		{
			dataarray.push_back(RedisValue(iter->second));
			dataarray.push_back(RedisValue(Value(iter->first).readString()));
		}

		return RedisValue(dataarray);
	}

	RedisValue zremrangebyscoreFunc(const std::vector<RedisValue> & val)
	{
		if (val.size() <= 3) return RedisValue(false, "wrong number of arguments");
		std::string key = String::tolower(val[1].toString());
		int64_t minscore = val[2].toInt();
		int64_t maxscore = val[3].toInt();

		shared_ptr<ValueObject> valueobject;
		std::map<std::string, shared_ptr<ValueObject> >::iterator iter = valuelist.find(key);
		if (iter == valuelist.end())
		{
			return RedisValue(0);
		}
		else
		{
			valueobject = iter->second;
		}

		if (valueobject->type() != DataType_ZSet) return RedisValue(0);
		ValueZSet* zsetobject = (ValueZSet*)valueobject.get();

		uint32_t countval = zsetobject->remByScore(minscore, maxscore);

		return RedisValue(countval);
	}
};