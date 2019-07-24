#pragma once

#include "Base/Base.h"
#include "Network/Network.h"
#include "value/valuehash.h"
#include "value/valuestring.h"
#include "value/valuezset.h"
#include "value/valuelist.h"
#include "boost/regex.hpp"
using namespace Public::Base;
using namespace Public::Network;


typedef Function1<RedisValue, const std::vector<RedisValue> &> CommandCallback;

class DBKey:public Strand
{
	struct DBStrandData :public StrandData
	{
		shared_ptr<RedisValue> value;
		void*				   user;
		CmdResultCallback	   callback;
	};
public:
	DBKey(const shared_ptr<IOWorker>& ioworker, const shared_ptr<DataFactory>& _factory, uint32_t _dbindex):Strand(ioworker),factory(_factory),dbindex(_dbindex)
	{
		subCommand("DEL", CommandCallback(&DBKey::delFunc, this));
		subCommand("EXISTS", CommandCallback(&DBKey::existsFunc, this));
		subCommand("EXPIRE", CommandCallback(&DBKey::expireFunc, this));
		subCommand("TTL", CommandCallback(&DBKey::ttlFunc, this));
		subCommand("KEYS", CommandCallback(&DBKey::keysFunc, this));
		subCommand("RENAME", CommandCallback(&DBKey::renameFunc, this));
		subCommand("TYPE", CommandCallback(&DBKey::typeFunc, this));
		subCommand("SCAN", CommandCallback(&DBKey::scanFunc, this));
	}
	~DBKey()
	{
	}

	void inputCommand(const CmdResultCallback& callback, void* user, const shared_ptr<RedisValue>& value)
	{
		shared_ptr<DBStrandData> data = make_shared<DBStrandData>();
		data->callback = callback;
		data->user = user;
		data->value = value;

		post(Strand::StrandCallback(&DBKey::strandCallback, this), data);
	}
	void poolTimerProc()
	{
		post(Strand::StrandCallback(&DBKey::timeoutCallback, this), shared_ptr<StrandData>());
	}
private:
	RedisValue delFunc(const std::vector<RedisValue> & val)
	{
		if (val.size() <= 1) return RedisValue(false, "wrong number of arguments");

		uint32_t delcount = 0;
		for (uint32_t i = 1; i < val.size(); i++)
		{
			std::string key = String::tolower(val[i].toString());
			if (key == "") continue;

			std::map<std::string, shared_ptr<ValueObject> >::iterator iter = valuelist.find(key);
			if (iter != valuelist.end())
			{
				delcount++;
				valuelist.erase(iter);
			}
		}

		return RedisValue(delcount);
	}

	RedisValue existsFunc(const std::vector<RedisValue> & val)
	{
		if (val.size() <= 1) return RedisValue(false, "wrong number of arguments");

		uint32_t delcount = 0;
		for (uint32_t i = 1; i < val.size(); i++)
		{
			std::string key = String::tolower(val[i].toString());
			if (key == "") continue;

			std::map<std::string, shared_ptr<ValueObject> >::iterator iter = valuelist.find(key);
			if (iter != valuelist.end())
			{
				delcount++;
			}
		}

		return RedisValue(delcount);
	}

	RedisValue expireFunc(const std::vector<RedisValue> & val)
	{
		if (val.size() != 3) return RedisValue(false, "wrong number of arguments");
		std::string key = String::tolower(val[1].toString());
		uint64_t ttl = val[2].toInt();

		std::map<std::string, shared_ptr<ValueObject> >::iterator iter = valuelist.find(key);
		if (iter == valuelist.end())
		{
			return RedisValue(0);
		}

		iter->second->exprise(ttl);

		return RedisValue(1);
	}

	RedisValue ttlFunc(const std::vector<RedisValue> & val)
	{
		if (val.size() != 2) return RedisValue(false, "wrong number of arguments");
		std::string key = String::tolower(val[1].toString());


		uint64_t ttl = 0;
		std::map<std::string, shared_ptr<ValueObject> >::iterator iter = valuelist.find(key);
		if (iter != valuelist.end())
		{
			ttl = iter->second->ttl();
		}

		return RedisValue(ttl);
	}

	RedisValue keysFunc(const std::vector<RedisValue> & val)
	{
		if (val.size() != 2) return RedisValue(false, "wrong number of arguments");
		std::string pattern = String::tolower(val[1].toString());


		std::vector<RedisValue> keysval;

		boost::regex oRegex(pattern == "*" ? "" : pattern);
		for (std::map<std::string, shared_ptr<ValueObject> >::iterator iter = valuelist.begin(); iter != valuelist.end(); iter++)
		{
			if (pattern != "" && pattern != "*" && !boost::regex_match(iter->first, oRegex))
			{
				continue;
			}

			keysval.push_back(RedisValue(iter->first));
		}
		
		return RedisValue(keysval);
	}

	RedisValue renameFunc(const std::vector<RedisValue> & val)
	{
		if (val.size() != 3) return RedisValue(false, "wrong number of arguments");
		std::string oldkey = String::tolower(val[1].toString());
		std::string newkey = String::tolower(val[2].toString());

		std::map<std::string, shared_ptr<ValueObject> >::iterator iter = valuelist.find(oldkey);
		if (iter == valuelist.end())
		{
			return RedisValue(false, "no such key");
		}

		shared_ptr<ValueObject> valuetmp = iter->second;
		valuelist.erase(iter);

		valuetmp->setkey(newkey);
		valuelist[newkey] = valuetmp;

		return RedisValue(true, "OK");
	}

	RedisValue typeFunc(const std::vector<RedisValue> & val)
	{
		if (val.size() != 2) return RedisValue(false, "wrong number of arguments");
		std::string key = String::tolower(val[1].toString());


		DataType type = DataType_None;
		std::map<std::string, shared_ptr<ValueObject> >::iterator iter = valuelist.find(key);
		if (iter != valuelist.end())
		{
			type = iter->second->type();
		}

		std::string typestr;
		switch (type)
		{
		case DataType_Hash: typestr = "hash"; break;
		case DataType_List: typestr = "list"; break;
		case DataType_String: typestr = "string"; break;
		case DataType_ZSet: typestr = "zset"; break;
		default: typestr = "none"; break;
		}

		return RedisValue(true,typestr);
	}

	RedisValue scanFunc(const std::vector<RedisValue> & val)
	{
		uint32_t cursor = 0;
		std::string pattern;
		uint32_t count = -1;

		if(val.size() == 1) return RedisValue(false, "wrong number of arguments");
	//	else if (!val[1].isInt())  return RedisValue(false, "invalid cursor");
		
		cursor = (uint32_t)val[1].toInt();

		for (uint32_t i = 2; i < val.size(); i+=2)
		{
			std::string param = String::tolower(val[i].toString());
			if (param == "match" && val.size() > i + 1) pattern = String::tolower(val[i + 1].toString());
			else if (param == "count" && val.size() > i + 1) count = (uint32_t)val[i + 1].toInt();
			else return RedisValue(false,"syntax error");
		}


		std::vector<RedisValue> keys;
		uint32_t currcursor = 0;
		boost::regex oRegex(pattern == "*" ? "" : pattern);
		for (std::map<std::string, shared_ptr<ValueObject> >::iterator iter = valuelist.begin(); iter != valuelist.end(); iter++, currcursor++)
		{
			if(currcursor < cursor) continue;
			if (count != -1 && keys.size() > count) break;
			
			if (pattern == "" || pattern == "*" || boost::regex_match(iter->first, oRegex))
			{
				keys.push_back(RedisValue(iter->first));
			}
		}

		std::vector<RedisValue> retval;
		retval.push_back(RedisValue(currcursor >= valuelist.size() ? 0 : currcursor));
		retval.push_back(RedisValue(keys));

		return RedisValue(retval);
	}
private:
	void timeoutCallback(const shared_ptr<StrandData>& data)
	{
		uint64_t nowtime = Time::getCurrentTime().makeTime();
		for (std::map<std::string, shared_ptr<ValueObject> >::iterator iter = valuelist.begin(); iter != valuelist.end();)
		{
			if (iter->second->expire() != 0 && nowtime < iter->second->expire())
			{
				valuelist.erase(iter++);
			}
			else
			{
				iter++;
			}
		}
	}
	void strandCallback(const shared_ptr<StrandData>& data)
	{
		shared_ptr<StrandData> tmp = data;

		if (tmp == NULL) return;

		DBStrandData* stranddata = (DBStrandData*)tmp.get();

		RedisValue val = inputCommand(*stranddata->value.get());

		stranddata->callback(stranddata->user, val);
	}
	RedisValue inputCommand(const RedisValue& value)
	{
		if (!value.isArray()) return RedisValue(false, std::string(std::string("unknown command '") + (std::string)value.getString() + "'"));

		const std::vector<RedisValue> & valuearray = value.getArray();

		std::string cmd = String::tolower(valuearray[0].toString());
		std::map<std::string, CommandCallback>::iterator iter = callbacklist.find(cmd);
		if (iter == callbacklist.end())
		{
			return RedisValue(false, std::string("unknown command '" + cmd + "'"));
		}

		RedisValue retval = iter->second(valuearray);
		if (retval.isStatus() && !retval.getStatus().success)
		{
			retval.getStatus().errmsg = std::string(String::toupper(iter->first) + " " + retval.getStatus().errmsg);
		}

		return retval;
	}
protected:
	void subCommand(const std::string& cmd, const CommandCallback& callback)
	{
		std::string subcmd = String::tolower(cmd);
		callbacklist[subcmd] = callback;
	}
private:
	std::map<std::string, CommandCallback>	callbacklist;
protected:
	uint32_t										dbindex;
	shared_ptr<DataFactory>							factory;
	std::map<std::string, shared_ptr<ValueObject> > valuelist;
};