#pragma once

#include "dbzset_3.h"


class DBHash :public DBZSet
{
public:
	DBHash(const shared_ptr<IOWorker>& ioworker, const shared_ptr<DataFactory>& factory, int dbindex) :DBZSet(ioworker,factory ,dbindex)
	{
		subCommand("HDEL", CommandCallback(&DBHash::hdelFunc, this));
		subCommand("HSET", CommandCallback(&DBHash::hsetFunc, this));
		subCommand("HSETNX", CommandCallback(&DBHash::hsetnxFunc, this));
		subCommand("HGET", CommandCallback(&DBHash::hgetFunc, this));
		subCommand("HGETALL", CommandCallback(&DBHash::hgetallFunc, this));
		subCommand("HKEYS", CommandCallback(&DBHash::hkeysFunc, this));
		subCommand("HLEN", CommandCallback(&DBHash::hlenFunc, this));
		subCommand("HSCAN", CommandCallback(&DBHash::hscanFunc, this));
	}
private:
	RedisValue hdelFunc(const std::vector<RedisValue> & val)
	{
		if (val.size() <= 2) return RedisValue(false, "Param Error");
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

		if (valueobject->type() != DataType_Hash) return RedisValue(0);
		ValueHash* hashobject = (ValueHash*)valueobject.get();

		for (uint32_t i = 2; i < val.size(); i++)
		{
			std::string filed = String::tolower(val[i].toString());
			if(key == "" || filed == "") continue;

			hashobject->del(filed);
		}

		return RedisValue(1);
	}

	RedisValue hsetFunc(const std::vector<RedisValue> & val)
	{
		if (val.size() != 4) return RedisValue(false, "Param Error");
		std::string key = String::tolower(val[1].toString());
		std::string filed = String::tolower(val[2].toString());
		const std::string data = val[3].toString();

		shared_ptr<ValueObject> valueobject;
		std::map<std::string, shared_ptr<ValueObject> >::iterator iter = valuelist.find(key);
		if (iter == valuelist.end())
		{
			valueobject = make_shared<ValueHash>(factory, key, dbindex);
			valuelist[key] = valueobject;
		}
		else
		{
			valueobject = iter->second;
		}

		if (valueobject->type() != DataType_Hash) return RedisValue(0);
		ValueHash* hashobject = (ValueHash*)valueobject.get();

		hashobject->set(filed ,data);

		return RedisValue(1);
	}

	RedisValue hsetnxFunc(const std::vector<RedisValue> & val)
	{
		if (val.size() != 4) return RedisValue(false, "Param Error");
		std::string key = String::tolower(val[1].toString());
		std::string filed = String::tolower(val[2].toString());
		const std::string data = val[3].toString();

		shared_ptr<ValueObject> valueobject;
		std::map<std::string, shared_ptr<ValueObject> >::iterator iter = valuelist.find(key);
		if (iter == valuelist.end())
		{
			valueobject = make_shared<ValueHash>(factory, key, dbindex);
			valuelist[key] = valueobject;
		}
		else
		{
			valueobject = iter->second;
		}

		if (valueobject->type() != DataType_Hash) return RedisValue(0);
		ValueHash* hashobject = (ValueHash*)valueobject.get();

		bool ret = hashobject->setnx(filed, data);

		return RedisValue(ret ? 1 : 0);
	}

	RedisValue hexistsFunc(const std::vector<RedisValue> & val)
	{
		if (val.size() != 3) return RedisValue(false, "Param Error");
		std::string key = String::tolower(val[1].toString());
		std::string filed = String::tolower(val[2].toString());

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

		if (valueobject->type() != DataType_Hash) return RedisValue(0);
		ValueHash* hashobject = (ValueHash*)valueobject.get();

		bool ret = hashobject->exists(filed);

		return RedisValue(ret ? 1 : 0);
	}

	RedisValue hgetFunc(const std::vector<RedisValue> & val)
	{
		if (val.size() != 3) return RedisValue(false, "Param Error");
		std::string key = String::tolower(val[1].toString());
		std::string filed = String::tolower(val[2].toString());

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

		if (valueobject->type() != DataType_Hash) return RedisValue(0);
		ValueHash* hashobject = (ValueHash*)valueobject.get();

		std::string data;
		hashobject->get(filed, data);

		return RedisValue(data);
	}

	RedisValue hgetallFunc(const std::vector<RedisValue> & val)
	{
		if (val.size() != 2) return RedisValue(false, "Param Error");
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

		if (valueobject->type() != DataType_Hash) return RedisValue(0);
		ValueHash* hashobject = (ValueHash*)valueobject.get();

		std::map<std::string,std::string> data;
		hashobject->getall(data);

		std::vector<RedisValue> dataarray;
		for (std::map<std::string, std::string>::iterator iter = data.begin(); iter != data.end(); iter++)
		{
			dataarray.push_back(RedisValue(iter->first));
			dataarray.push_back(RedisValue(iter->second));
		}

		return RedisValue(dataarray);
	}

	RedisValue hkeysFunc(const std::vector<RedisValue> & val)
	{
		if (val.size() < 2) return RedisValue(false, "Param Error");
		std::string key = String::tolower(val[1].toString());
		std::string filed = val.size() >= 3 ? String::tolower(val[2].toString()) : "";

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

		if (valueobject->type() != DataType_Hash) return RedisValue(0);
		ValueHash* hashobject = (ValueHash*)valueobject.get();

		std::set<std::string> fileds;
		hashobject->hkeys(filed, fileds);

		std::vector<RedisValue> dataarray;
		for (std::set<std::string>::iterator iter = fileds.begin(); iter != fileds.end(); iter++)
		{
			dataarray.push_back(RedisValue(*iter));
		}

		return RedisValue(dataarray);
	}

	RedisValue hlenFunc(const std::vector<RedisValue> & val)
	{
		if (val.size() != 2) return RedisValue(false, "Param Error");
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

		if (valueobject->type() != DataType_Hash) return RedisValue(0);
		ValueHash* hashobject = (ValueHash*)valueobject.get();


		uint32_t lenval = hashobject->len();

		return RedisValue(lenval);
	}

	RedisValue hscanFunc(const std::vector<RedisValue>& val)
	{
		uint32_t cursor = 0;
		std::string pattern,key;
		uint32_t count = -1;

		if (val.size() <= 2) return RedisValue(false, "Param Error");
		//	else if (!val[1].isInt())  return RedisValue(false, "invalid cursor");

		key = val[1].toString();
		cursor = (uint32_t)val[2].toInt();
		

		for (uint32_t i = 3; i < val.size(); i += 2)
		{
			std::string param = String::tolower(val[i].toString());
			if (param == "match" && val.size() > i + 1) pattern = String::tolower(val[i + 1].toString());
			else if (param == "count" && val.size() > i + 1) count = (uint32_t)val[i + 1].toInt();
			else return RedisValue(false, "syntax error");
		}

		std::vector<std::string> keysdata;
		uint32_t newscosor = 0;
		
		shared_ptr<ValueObject> valueobject;
		std::map<std::string, shared_ptr<ValueObject> >::iterator iter = valuelist.find(key);
		if (iter != valuelist.end() && iter->second->type() == DataType_Hash)
		{
			ValueHash* hashobject = (ValueHash*)iter->second.get();
			newscosor = hashobject->scan(cursor, pattern, count, keysdata);
		}
		
		std::vector<RedisValue> keys;
		for (uint32_t i = 0;i < keysdata.size();i ++)
		{
			keys.push_back(keysdata[i]);
		}

		std::vector<RedisValue> retval;
		retval.push_back(RedisValue(newscosor));
		retval.push_back(RedisValue(keys));

		return RedisValue(retval);
	}
};