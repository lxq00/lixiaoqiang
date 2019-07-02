#pragma once

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

	virtual shared_ptr<ValueData> createValueData(const shared_ptr<ValueHeader>& header,const std::string& name="")
	{
		return make_shared<ValueData>(shared_from_this(), header, name);
	}
	virtual void updateValueData(ValueData* data){}
	virtual void readValueData(ValueData* data, std::string& datastr) {}
};

struct ValueHeader
{
	friend class StoreFactory;

	ValueHeader() {}
	ValueHeader(const shared_ptr<DataFactory>& factory, const std::string& key, uint32_t dbindex,DataType _type)
		:m_factory(factory),m_key(key),m_type(_type), m_expire(0), m_dbindex(dbindex)
	{
		update();
	}
	virtual ~ValueHeader() 
	{
	}

	void setkey(const std::string& keyname)
	{
		m_key = keyname;

		update();
	}

	DataType type()const { return m_type; }

	void setExpire(uint64_t expire)
	{
		m_expire = expire; 
	
		update();
	}
	uint64_t expire()const { return m_expire; }
	const std::string& key() const { return m_key; }
	uint32_t dbindex() const { return m_dbindex; }
	shared_ptr<DataFactory> factory() { return m_factory; }
	virtual void update()
	{
		m_factory->updateHeader(this);
	}
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
		update();
	}
	ValueData(const shared_ptr<DataFactory>& factory, const shared_ptr<ValueHeader>& _header,const std::string& _name)
	{
		m_factory = factory;
		m_header = _header;
		m_name = _name;
	}
	virtual ~ValueData() 
	{
	}
	shared_ptr<ValueHeader> header() { return m_header.lock(); }
	virtual void setData(const std::string& data)
	{
		m_len = data.length();
		m_data = data;

		update();
	}
	uint32_t len() const { return m_len; }
	virtual bool getData(std::string& data)
	{
		if (m_len != m_data.length())
		{
			m_factory->readValueData(this,m_data);
		}

		data = m_data;

		return true;
	}
	virtual void update()
	{
		m_factory->updateValueData(this);
	}
	const std::string name()const { return m_name; }
protected:
	shared_ptr<DataFactory> m_factory;
	weak_ptr<ValueHeader>	m_header;
	std::string				m_name;
	uint32_t				m_len;
	std::string				m_data;
};
