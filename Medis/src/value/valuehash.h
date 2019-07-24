#pragma once

#include "Base/Base.h"
#include "Network/Network.h"
#include "valueobject.h"
using namespace Public::Base;
using namespace Public::Network;


class ValueHash :public ValueObject
{
public:
	ValueHash(const shared_ptr<DataFactory>& factory, const std::string& key, uint32_t dbindex) :ValueObject(factory,key, dbindex,DataType_Hash){}
	ValueHash(const shared_ptr<ValueHeader>& _header) :ValueObject(_header) {}
	~ValueHash(){}

	bool exists(const std::string& field)
	{
		std::map<std::string, shared_ptr<ValueData> >::iterator niter = datalist.find(field);
		
		return niter != datalist.end();
	}
	bool del(const std::string& field)
	{
		datalist.erase(field);

		return true;
	}
	bool set(const std::string& field, const RedisString& data)
	{
		shared_ptr<ValueData> node = factory->createValueData(header, field);
		node->setData(data);

		datalist[field] = node;
	
		return true;

	}
	bool setnx(const std::string& field, const RedisString& data)
	{
		if (exists(field)) return false;

		return set(field,data);

	}
	RedisString get(const std::string& field)
	{
		std::map<std::string, shared_ptr<ValueData> >::iterator niter = datalist.find(field);
		if (niter == datalist.end()) return RedisString();

		return niter->second->getData();
	}
	bool getall(std::map<std::string, RedisString>& data)
	{
		for (std::map<std::string, shared_ptr<ValueData> >::iterator niter = datalist.begin(); niter != datalist.end(); niter++)
		{
			RedisString datastr = niter->second->getData();
			data[niter->first] = datastr;
		}

		return true;
	}
	bool hkeys(const std::string& field, std::set<std::string>& fields)
	{
		boost::regex oRegex(field == "*" ? "" : field);
		for (std::map<std::string, shared_ptr<ValueData> >::iterator niter = datalist.begin(); niter != datalist.end(); niter++)
		{
			if (field == "" || field == "*" || boost::regex_match(niter->first, oRegex))
			{
				fields.insert(niter->first);
			}
		}

		return true;
	}
	uint32_t len()
	{
		return datalist.size();
	}
	virtual void addData(const shared_ptr<ValueData>& data)
	{
		datalist[data->name()] = data;
	}
	uint32_t scan(uint32_t cursor, const std::string& pattern, uint32_t count, std::vector<RedisString>& keys)
	{
		uint32_t currcursor = 0;
		boost::regex oRegex(pattern == "*" ? "" : pattern);
		for (std::map<std::string, shared_ptr<ValueData> >::iterator iter = datalist.begin(); iter != datalist.end(); iter++, currcursor++)
		{
			if (currcursor < cursor) continue;
			if (count != -1 && keys.size() > count) break;

			if (pattern == "" || pattern == "*" || boost::regex_match(iter->first, oRegex))
			{
				keys.push_back(iter->first);

				RedisString datastr = iter->second->getData();

				keys.push_back(datastr);
			}
		}

		return (count == -1 || keys.size() < count) ? 0 : currcursor;
	}
private:
	std::map<std::string, shared_ptr<ValueData> > datalist;
};