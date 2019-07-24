#pragma once

#include "dbhash_4.h"


class DB :public DBHash
{
public:
	DB(const shared_ptr<IOWorker>& ioworker, const shared_ptr<DataFactory>& factory, int index) :DBHash(ioworker,factory ,index)
	{
	}
	uint32_t keyscount()
	{
		return valuelist.size();
	}
	void initHeader(const shared_ptr<ValueHeader>& header)
	{
		std::string key = header->key();
		shared_ptr<ValueObject> object;
		
		if (header->type() == DataType_List)
		{
			object = make_shared<ValueList>(header);
		}
		else if (header->type() == DataType_Hash)
		{
			object = make_shared<ValueHash>(header);
		}
		else if (header->type() == DataType_String)
		{
			object = make_shared<ValueString>(header);
		}
		else if (header->type() == DataType_ZSet)
		{
			object = make_shared<ValueZSet>(header);
		}
		else
		{
			return;
		}

		valuelist[key] = object;
	}
	void initData(const shared_ptr<ValueData>& data)
	{
		shared_ptr<ValueHeader> header = data->header();
		std::string key = header->key();

		std::map<std::string, shared_ptr<ValueObject> >::iterator iter = valuelist.find(key);
		if (iter == valuelist.end()) return;

		iter->second->addData(data);
	}
};