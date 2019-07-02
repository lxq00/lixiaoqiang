#pragma once

#include "Base/Base.h"
#include "Network/Network.h"
#include "valueobject.h"
using namespace Public::Base;
using namespace Public::Network;


class ValueZSet :public ValueObject
{
public:
	ValueZSet(const shared_ptr<DataFactory>& factory, const std::string& key, uint32_t dbindex) :ValueObject(factory,key, dbindex,DataType_ZSet) {}
	ValueZSet(const shared_ptr<ValueHeader>& _header):ValueObject(_header){}
	~ValueZSet() {}

	bool add(uint64_t score, const std::string& data)
	{
		shared_ptr<ValueData> node;
		std::map<uint64_t, shared_ptr<ValueData> >::iterator niter = datalist.find(score);
		if (niter == datalist.end())
		{
			node = factory->createValueData(header, Value(score).readString());

			datalist[score] = node;

			indexmap.push_back(score);
			indexmap.sort();
		}
		else
		{
			node = niter->second;
		}
		
		node->setData(data);

		return true;
	}

	uint64_t card()
	{
		return indexmap.size();
	}

	uint32_t count(uint64_t minscore, uint64_t maxscore)
	{
		uint32_t zcount = 0;
		for (std::list<uint64_t>::iterator iter = indexmap.begin(); iter != indexmap.end(); iter++)
		{
			if(*iter < minscore) continue;
			if(*iter > maxscore) break;

			zcount++;
		}
		return zcount;
	}
	bool range(int64_t start, int64_t top, std::map<uint64_t, std::string>& datamap)
	{
		if (start <= -1) start = indexmap.size() + start + 1;
		if(top <= -1) top = indexmap.size() + top + 1;

		int readpos = 0;
		for (std::list<uint64_t>::iterator iter = indexmap.begin(); iter != indexmap.end(); iter++, readpos++)
		{
			if(readpos < start) continue;
			if (readpos > top) break;

			std::string datastr;
			datalist[*iter]->getData(datastr);

			datamap[*iter] = datastr;
		}

		return true;
	}

	bool rangeByScore(uint64_t minscore, uint64_t maxscore, std::map<uint64_t, std::string>& datamap)
	{
		for (std::list<uint64_t>::iterator iter = indexmap.begin(); iter != indexmap.end(); iter++)
		{
			if (*iter < minscore) continue;
			if (*iter > maxscore) break;

			std::string datastr;
			datalist[*iter]->getData(datastr);

			datamap[*iter] = datastr;
		}

		return true;
	}
	uint32_t remByScore(uint64_t minscore, uint64_t maxscore)
	{
		uint32_t count = 0;
		for (std::list<uint64_t>::iterator iter = indexmap.begin(); iter != indexmap.end();)
		{
			if (*iter < minscore)
			{
				iter++;
				continue;
			}
			if (*iter > maxscore) break;

			datalist.erase(*iter);
			indexmap.erase(iter++);
			
			count++;
		}

		return count;
	}
	virtual void addData(const shared_ptr<ValueData>& data)
	{
		uint64_t score = Value(data->name()).readInt64();

		indexmap.push_back(score);
		datalist[score] = data;

		indexmap.sort();
	}
private:
	std::list<uint64_t>					      indexmap;
	std::map<uint64_t, shared_ptr<ValueData> > datalist;
};