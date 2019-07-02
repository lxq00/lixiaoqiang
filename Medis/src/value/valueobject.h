#pragma once

#include "Base/Base.h"
#include "valuedata.h"
#include "boost/regex.hpp" 
#include "common/redisvalue.h"
#include "valuedata.h"
using namespace Public::Base;


class ValueObject
{
public:
	ValueObject(const shared_ptr<DataFactory>& _factory,const std::string& key, uint32_t index, DataType type):factory(_factory)
	{
		header = factory->createHeader(key,index,type);
	}
	ValueObject(const shared_ptr<ValueHeader>& _header)
	{
		header = _header;
		factory = header->factory();
	}
	virtual ~ValueObject()
	{
	}
	bool exprise(uint64_t ttl)
	{
		header->setExpire(ttl == 0 ? 0 : Time::getCurrentTime().makeTime() + ttl);

		return true;
	}
	void setkey(const std::string& key)
	{
		header->setkey(key);
	}
	uint64_t expire() { return header->expire(); }
	DataType type() { return header->type(); }

	uint64_t ttl()
	{
		uint64_t ttlval = 0;

		uint64_t nowtime = Time::getCurrentTime().makeTime();
		uint64_t expire = header->expire();
		if (expire == 0 || nowtime > expire) ttlval = 0;
		else ttlval = expire - nowtime;

		return ttlval;
	}
	virtual void addData(const shared_ptr<ValueData>& data) = 0;
protected:
	shared_ptr<ValueHeader> header;
	shared_ptr<DataFactory>	factory;
};