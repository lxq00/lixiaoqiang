#pragma once

#include "Base/Base.h"
#include "Network/Network.h"
#include "valueobject.h"
using namespace Public::Base;
using namespace Public::Network;


class ValueString:public ValueObject
{
public:
	ValueString(const shared_ptr<DataFactory>& factory,const std::string& key, uint32_t dbindex):ValueObject(factory,key, dbindex,DataType_String)
	{
		value = factory->createValueData(header);
	}
	ValueString(const shared_ptr<ValueHeader>& _header) :ValueObject(_header) {}
	~ValueString(){}

	bool get(std::string& data)
	{
		return value->getData(data);
	}
	bool set(const std::string& data)
	{
		value->setData(data);

		return true;
	}

	uint32_t len()
	{
		return value->len();
	}

	virtual void addData(const shared_ptr<ValueData>& data)
	{
		value = data;
	}
private:
	shared_ptr<ValueData>  value;
};