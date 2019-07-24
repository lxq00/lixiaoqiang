#pragma once

#include "dbstring_1.h"

class DBList :public DBString
{
public:
	DBList(const shared_ptr<IOWorker>& ioworker, const shared_ptr<DataFactory>& factory, int dbindex) :DBString(ioworker,factory , dbindex)
	{
		subCommand("LPUSH", CommandCallback(&DBList::lpushFunc, this));
		subCommand("RPUSH", CommandCallback(&DBList::rpushFunc, this));
		subCommand("LPOP", CommandCallback(&DBList::lpopFunc, this));
		subCommand("RPOP", CommandCallback(&DBList::rpopFunc, this));
		subCommand("LLEN", CommandCallback(&DBList::llenFunc, this));
		subCommand("LRANGE", CommandCallback(&DBList::lrangeFunc, this));
	}
private:
	RedisValue lpushFunc(const std::vector<RedisValue> & val)
	{
		if (val.size() < 3) return RedisValue(false, "wrong number of arguments");

		std::string key = String::tolower(val[1].toString());

		shared_ptr<ValueObject> valueobject;
		std::map<std::string, shared_ptr<ValueObject> >::iterator iter = valuelist.find(key);
		if (iter == valuelist.end())
		{
			valueobject = make_shared<ValueList>(factory, key, dbindex);
			valuelist[key] = valueobject;
		}
		else
		{
			valueobject = iter->second;
		}
		
		if (valueobject->type() != DataType_List) return RedisValue(0);
		ValueList* listobject = (ValueList*)valueobject.get();

		uint32_t count = 0;
		for (uint32_t i = 2; i < val.size(); i++)
		{
			RedisString data = val[i].toString();
			if(data.length() == 0) continue;

			listobject->push_front(data);

			count++;
		}

		return RedisValue(count);
	}

	RedisValue rpushFunc(const std::vector<RedisValue> & val)
	{
		if (val.size() < 3) return RedisValue(false, "wrong number of arguments");

		std::string key = String::tolower(val[1].toString());

		shared_ptr<ValueObject> valueobject;
		std::map<std::string, shared_ptr<ValueObject> >::iterator iter = valuelist.find(key);
		if (iter == valuelist.end())
		{
			valueobject = make_shared<ValueList>(factory, key, dbindex);
			valuelist[key] = valueobject;
		}
		else
		{
			valueobject = iter->second;
		}

		if (valueobject->type() != DataType_List) return RedisValue(0);
		ValueList* listobject = (ValueList*)valueobject.get();

		uint32_t count = 0;
		for (uint32_t i = 2; i < val.size(); i++)
		{
			RedisString data = val[i].toString();
			if (data.length() == 0) continue;

			listobject->push_back(data);

			count++;
		}

		return RedisValue(count);
	}

	RedisValue lpopFunc(const std::vector<RedisValue> & val)
	{
		if (val.size() != 2) return RedisValue(false, "wrong number of arguments");
		std::string key = String::tolower(val[1].toString());

		std::map<std::string, shared_ptr<ValueObject> >::iterator iter = valuelist.find(key);
		if (iter == valuelist.end())
		{
			return RedisValue(0);
		}

		if (iter->second->type() != DataType_List) return RedisValue(0);
		ValueList* listobject = (ValueList*)iter->second.get();
		
		RedisString data;
		listobject->pop_front(data);

		return RedisValue(data);
	}

	RedisValue rpopFunc(const std::vector<RedisValue> & val)
	{
		if (val.size() != 2) return RedisValue(false, "wrong number of arguments");
		std::string key = String::tolower(val[1].toString());

		std::map<std::string, shared_ptr<ValueObject> >::iterator iter = valuelist.find(key);
		if (iter == valuelist.end())
		{
			return RedisValue(0);
		}

		if (iter->second->type() != DataType_List) return RedisValue(0);
		ValueList* listobject = (ValueList*)iter->second.get();

		RedisString data;
		listobject->pop_back(data);

		return RedisValue(data);
	}

	RedisValue llenFunc(const std::vector<RedisValue> & val)
	{
		if (val.size() != 2) return RedisValue(false, "wrong number of arguments");
		std::string key = String::tolower(val[1].toString());

		std::map<std::string, shared_ptr<ValueObject> >::iterator iter = valuelist.find(key);
		if (iter == valuelist.end())
		{
			return RedisValue(0);
		}

		if (iter->second->type() != DataType_List) return RedisValue(0);
		ValueList* listobject = (ValueList*)iter->second.get();


		uint32_t lenval = listobject->len();

		return RedisValue(lenval);
	}

	RedisValue lrangeFunc(const std::vector<RedisValue>& val)
	{
		if (val.size() != 4) return RedisValue(false, "wrong number of arguments");
		std::string key = String::tolower(val[1].toString());
		int32_t start = (int32_t)val[2].toInt();
		int32_t stop = (int32_t)val[3].toInt();

		std::map<std::string, shared_ptr<ValueObject> >::iterator iter = valuelist.find(key);
		if (iter == valuelist.end())
		{
			return RedisValue(0);
		}

		if (iter->second->type() != DataType_List) return RedisValue(0);
		ValueList* listobject = (ValueList*)iter->second.get();


		std::vector<RedisString> dataarray;
		listobject->range(start, stop, dataarray);

		std::vector<RedisValue> values;
		for (uint32_t i = 0; i < dataarray.size(); i++)
		{
			values.push_back(RedisValue(dataarray[i]));
		}

		return RedisValue(values);
	}
};