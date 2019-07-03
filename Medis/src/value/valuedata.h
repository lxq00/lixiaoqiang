#pragma once

#include "../common/redisstring.h"
#include "Base/Base.h"
using namespace Public::Base;


typedef enum
{
	DataType_None = 0,
	DataType_String = 1,
	DataType_Hash = 2,
	DataType_List = 3,
	DataType_ZSet = 4,
	DataType_Max = DataType_ZSet,
}DataType;



struct ValueHeader;
struct ValueData;
class StoreFactory;

class DataFactory:public enable_shared_from_this<DataFactory>
{
public:
	DataFactory() {}
	virtual ~DataFactory() {}

	virtual shared_ptr<ValueHeader> createHeader(const std::string& key, uint32_t dbindex, DataType _type)
	{
		return make_shared<ValueHeader>(shared_from_this(),key, dbindex,_type);
	}

	virtual void updateHeader(ValueHeader* header){}
	virtual void deleteHeader(ValueHeader* header) {}

	virtual shared_ptr<ValueData> createValueData(const shared_ptr<ValueHeader>& header,const std::string& name="")
	{
		return make_shared<ValueData>(shared_from_this(), header, name);
	}
	virtual void updateValueData(ValueData* data){}
	virtual void deleteValueData(ValueData* header) {}
};

struct ValueHeader
{
	friend class StoreFactory;

	ValueHeader() {}
	ValueHeader(const shared_ptr<DataFactory>& factory, const std::string& key, uint32_t dbindex,DataType _type)
		:m_factory(factory),m_key(key),m_type(_type), m_expire(0), m_dbindex(dbindex)
	{
		m_factory->updateHeader(this);
	}
	virtual ~ValueHeader() 
	{
		m_factory->deleteHeader(this);
	}

	void setkey(const std::string& keyname)
	{
		m_key = keyname;

		m_factory->updateHeader(this);
	}

	DataType type()const { return m_type; }

	void setExpire(uint64_t expire)
	{
		m_expire = expire; 
	
		m_factory->updateHeader(this);
	}
	uint64_t expire()const { return m_expire; }
	const std::string& key() const { return m_key; }
	uint32_t dbindex() const { return m_dbindex; }
	shared_ptr<DataFactory> factory() { return m_factory; }
protected:
	shared_ptr<DataFactory> m_factory;
	DataType				m_type;
	std::string				m_key;
	uint64_t				m_expire;
	uint32_t				m_dbindex;
};

struct ValueData
{
	friend class StoreFactory;

	ValueData() 
	{
		m_factory->updateValueData(this);
	}
	ValueData(const shared_ptr<DataFactory>& factory, const shared_ptr<ValueHeader>& _header,const std::string& _name)
	{
		m_factory = factory;
		m_header = _header;
		m_name = _name;
	}
	virtual ~ValueData() 
	{
		m_factory->deleteValueData(this);
	}
	shared_ptr<ValueHeader> header() { return m_header.lock(); }
	virtual void setData(const RedisString& data)
	{
		m_data = data;

		m_factory->updateValueData(this);
	}
	const RedisString& getData()
	{
		return m_data;
	}
	const std::string name()const { return m_name; }
protected:
	shared_ptr<DataFactory> m_factory;
	weak_ptr<ValueHeader>	m_header;
	std::string				m_name;
	uint32_t				m_len;
	RedisString				m_data;
};
