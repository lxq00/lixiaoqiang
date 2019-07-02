#pragma once

#include "Base/Base.h"
#include "Network/Network.h"
#include "valueobject.h"

using namespace Public::Base;
using namespace Public::Network;


class ValueList :public ValueObject
{
public:
	ValueList(const shared_ptr<DataFactory>& factory, const std::string& key, uint32_t dbindex) :ValueObject(factory,key, dbindex,DataType_List) {}
	ValueList(const shared_ptr<ValueHeader>& _header) :ValueObject(_header) {}
	~ValueList() {}

	bool push_back(const RedisString& data)
	{
		shared_ptr<ValueData> node = factory->createValueData(header);
		node->setData(data);

		datalist.push_back(node);

		return true;
	}
	bool push_front(const RedisString& data)
	{
		shared_ptr<ValueData> node = factory->createValueData(header);
		node->setData(data);

		datalist.push_front(node);

		return true;
	}
	bool pop_back(RedisString& data)
	{
		if (datalist.size() <= 0) return false;

		shared_ptr<ValueData> node = datalist.back();
		datalist.pop_back();

		data = node->getData();

		return true;
	}
	bool pop_front(RedisString& data)
	{
		if (datalist.size() <= 0) return false;

		shared_ptr<ValueData> node = datalist.front();
		datalist.pop_front();

		data = node->getData( );

		return true;
	}
	uint32_t len()
	{
		return datalist.size();
	}
	bool range(int32_t start, int32_t stop,std::vector<RedisString>& dataarray)
	{
		start = 0 + start;
		stop = datalist.size() + stop;

		int currpos = 0;
		for (std::list<shared_ptr<ValueData> >::iterator iter = datalist.begin(); iter != datalist.end(); iter++, currpos++)
		{
			if(currpos < start) continue;
			if (currpos > stop) break;

			RedisString datastr = (*iter)->getData();

			dataarray.push_back(datastr);
		}

		return true;
	}
	virtual void addData(const shared_ptr<ValueData>& data)
	{
		
	}
private:
	std::list<shared_ptr<ValueData> > datalist;
};