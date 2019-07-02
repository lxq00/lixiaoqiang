#pragma once

#include "dbkey_0.h"

class DBString :public DBKey
{
public:
	DBString(const shared_ptr<IOWorker>& ioworker, const shared_ptr<DataFactory>& factory, int dbindex):DBKey(ioworker,factory, dbindex)
	{
		subCommand("SET", CommandCallback(&DBString::setFunc, this));
		subCommand("GET", CommandCallback(&DBString::getFunc, this));
		subCommand("SETNX", CommandCallback(&DBString::setnxFunc, this));
		subCommand("STRLEN", CommandCallback(&DBString::strlenFunc, this));
	}
private:
	RedisValue getFunc(const std::vector<RedisValue> & val)
	{
		if (val.size() != 2) return RedisValue(false, "Param Error");
		std::string key = String::tolower(val[1].toString());
		
		shared_ptr<ValueObject> valueobj;
		std::map<std::string, shared_ptr<ValueObject> >::iterator iter = valuelist.find(key);
		if (iter == valuelist.end())
		{
			return RedisValue("");
		}
		else
		{
			valueobj = iter->second;
		}

		if (valueobj->type() != DataType_String) return RedisValue(0);
		ValueString* stringobject = (ValueString*)valueobj.get();
		
		RedisString datastr = stringobject->get();

		return RedisValue(datastr);
	}
	RedisValue setFunc(const std::vector<RedisValue> & val)
	{
		if (val.size() != 3) return RedisValue(false, "Param Error");
		std::string key = String::tolower(val[1].toString());
		const RedisString& data = val[2].toString();

		shared_ptr<ValueObject> valueobj;
		std::map<std::string, shared_ptr<ValueObject> >::iterator iter = valuelist.find(key);
		if (iter == valuelist.end())
		{
			valueobj = make_shared<ValueString>(factory,key, dbindex);
			valuelist[key] = valueobj;
		}
		else
		{
			valueobj = iter->second;
		}

		if (valueobj->type() != DataType_String) return RedisValue(0);
		ValueString* stringobject = (ValueString*)valueobj.get();
		stringobject->set(data);

		return RedisValue(true,"OK");
	}

	RedisValue setnxFunc(const std::vector<RedisValue> & val)
	{
		if (val.size() != 3) return RedisValue(false, "Param Error");
		std::string key = String::tolower(val[1].toString());
		const RedisString& data = val[2].toString();

		shared_ptr<ValueObject> valueobj;
		std::map<std::string, shared_ptr<ValueObject> >::iterator iter = valuelist.find(key);
		if (iter == valuelist.end())
		{
			valueobj = make_shared<ValueString>(factory,key, dbindex);
			valuelist[key] = valueobj;
		}
		else
		{
			return RedisValue(0);
		}

		if (valueobj->type() != DataType_String) return RedisValue(0);
		ValueString* stringobject = (ValueString*)valueobj.get();
		stringobject->set(data);

		return RedisValue(true,"OK");
	}
	RedisValue strlenFunc(const std::vector<RedisValue> & val)
	{
		if (val.size() != 2) return RedisValue(false, "Param Error");
		std::string key = String::tolower(val[1].toString());

		shared_ptr<ValueObject> valueobj;
		std::map<std::string, shared_ptr<ValueObject> >::iterator iter = valuelist.find(key);
		if (iter == valuelist.end())
		{
			return RedisValue(0);
		}
		else
		{
			valueobj = iter->second;
		}

		if (valueobj->type() != DataType_String) return RedisValue(0);
		ValueString* stringobject = (ValueString*)valueobj.get();

		return RedisValue(stringobject->len());
	}
};