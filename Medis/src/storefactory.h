#pragma once
#include "valuedata.h"
#include "storer.h"


#define SAVEMAXNUM				100
#define SAVEMAXTIMEOUT			60*1000

#define NOTUSESPACETIMEOUT		3*SAVEMAXTIMEOUT

#define HEADERFLAG			"isheader"

struct StoreValueHeader : public ValueHeader
{
	StoreValueHeader(const shared_ptr<DataFactory>& factory, const std::string& key, uint32_t dbindex, DataType _type)
		:ValueHeader(factory, key, dbindex, _type), filepos(-1) {}
	~StoreValueHeader();

	int		filepos;
};

struct StoreValueData :public ValueData
{
	StoreValueData(const shared_ptr<DataFactory>& factory, const shared_ptr<ValueHeader>& _header, const std::string& _name)
		:ValueData(factory, _header, _name), filepos(-1), prevsavetime(Time::getCurrentMilliSecond()), prevusedtime(Time::getCurrentMilliSecond()) {}

	~StoreValueData();

	int			filepos;
	uint64_t	prevsavetime;
	uint64_t	prevusedtime;
};

class StoreFactory :public DataFactory
{
public:
	StoreFactory(const shared_ptr<Storer>& _storer):storer(_storer)
	{
		timer = make_shared<Timer>("StoreFactory");
		timer->start(Timer::Proc(&StoreFactory::onPoolTimerProc, this), 0, 1000);
		prevsavetime = Time::getCurrentMilliSecond();
	}
	virtual ~StoreFactory()
	{
		timer = NULL;
	}

	void loadDBInfo(std::vector<shared_ptr<ValueHeader> >& headerlist, std::vector<shared_ptr<ValueData> >& datalist)
	{
		std::map<uint32_t,std::string> headerinfos;
		storer->scanHeader(headerinfos);

		for(std::map<uint32_t, std::string>::iterator iter = headerinfos.begin();iter != headerinfos.end();iter ++)
		{
			std::map<std::string, Value> headerarray;
			parseHeaderString(iter->second, headerarray);

			if (getHeaderValue(headerarray, HEADERFLAG).readBool())
			{
				shared_ptr<ValueHeader> header = parseAndBuildHeader(iter->first,headerarray);

				headerlist.push_back(header);
			}
		}

		for (std::map<uint32_t, std::string>::iterator iter = headerinfos.begin(); iter != headerinfos.end(); iter++)
		{
			std::map<std::string, Value> headerarray;
			parseHeaderString(iter->second, headerarray);

			if (!getHeaderValue(headerarray, HEADERFLAG).readBool())
			{
				shared_ptr<ValueData> data = parseAndBuildData(iter->first, headerlist, headerarray);
				if (data != NULL)
				{
					datalist.push_back(data);
				}
			}
		}
	}

	virtual shared_ptr<ValueHeader> createHeader(const std::string& key, uint32_t dbindex, DataType _type)
	{
		Guard locker(mutex);

		shared_ptr<ValueHeader> header = make_shared<StoreValueHeader>(shared_from_this(),key,dbindex,_type);
		headerlist[header.get()] = header;

		return header;
	}

	virtual void updateHeader(ValueHeader* header)
	{
		Guard locker(mutex);

		headerchangelist.insert(header);
	}
	virtual void deleteHeader(ValueHeader* header,uint32_t pos)
	{
		Guard locker(mutex);

		if (pos != -1)
		{
			headerlist.erase(header);
			deleteposlist.insert(pos);
		}
	}

	virtual shared_ptr<ValueData> createValueData(const shared_ptr<ValueHeader>& header, const std::string& name)
	{
		Guard locker(mutex);

		shared_ptr<ValueData> data = make_shared<StoreValueData>(shared_from_this(), header, name);
		datalist[data.get()] = data;

		return data;
	}
	virtual void updateValueData(ValueData* data)
	{
		Guard locker(mutex);

		datachangelist.insert(data);
	}
	virtual void deleteValueData(ValueData* data, uint32_t pos)
	{
		Guard locker(mutex);

		datachangelist.erase(data);
		if (pos != -1)
		{
			deleteposlist.insert(pos);
		}
	}
	virtual void readValueData(ValueData* data,std::string& datastr)
	{
		StoreValueData* valuedata = (StoreValueData*)data;

		storer->read(valuedata->filepos, data->len(), datastr);
	}
private:
	void onPoolTimerProc(unsigned long)
	{
		//freespace
		{
			std::set<uint32_t> deletetmp;
			{
				Guard locker(mutex);

				//删除父节点已经被清除的节点
				for (std::map<ValueData*, weak_ptr<ValueData> >::iterator iter = datalist.begin(); iter != datalist.end();)
				{
					shared_ptr<ValueData> valuedata = iter->second.lock();
					if (valuedata == NULL)
					{
						datalist.erase(iter++);
						continue;
					}

					shared_ptr<ValueHeader> header = valuedata->m_header.lock();
					if (header != NULL)
					{
						iter++;
						continue;
					}

					StoreValueData* data = (StoreValueData*)valuedata.get();

					if (data->filepos != -1)
					{
						deleteposlist.insert(data->filepos);
					}
					datalist.erase(iter++);
				}

				deletetmp = deleteposlist;
				deleteposlist.clear();
			}

			for (std::set<uint32_t>::iterator iter = deletetmp.begin(); iter != deletetmp.end(); iter++)
			{
				storer->remove(*iter);
			}
			if (deletetmp.size() > 0) storer->mergeFreeSpace();
		}

		//free not use space
		{
			Guard locker(mutex);

			uint64_t nowtime = Time::getCurrentMilliSecond();
			for (std::map<ValueData*, weak_ptr<ValueData> >::iterator iter = datalist.begin(); iter != datalist.end(); iter++)
			{
				shared_ptr< ValueData> datatmp = iter->second.lock();
				if (datatmp == NULL) continue;

				StoreValueData* valuedata = (StoreValueData*)datatmp.get();

				if (valuedata->filepos != -1 && valuedata->m_len == valuedata->m_data.length() && valuedata->m_len > 0 &&
					nowtime > valuedata->prevsavetime && nowtime - valuedata->prevsavetime > NOTUSESPACETIMEOUT &&
					nowtime > valuedata->prevusedtime && nowtime - valuedata->prevusedtime > NOTUSESPACETIMEOUT)
				{
					std::string emtpystr;
					valuedata->m_data.swap(emtpystr);
				}
			}
		}

		//begin save
		{
			std::map<ValueHeader*, shared_ptr<ValueHeader> > headerlisttmp;
			std::map<ValueData*, shared_ptr<ValueData> > datalisttmp;
			{
				Guard locker(mutex);

				uint64_t nowtime = Time::getCurrentMilliSecond();
				
				if (headerchangelist.size() > 0 || nowtime - prevsavetime > SAVEMAXTIMEOUT || datachangelist.size() > SAVEMAXNUM)
				{
					for (std::map<ValueData*, weak_ptr<ValueData> >::iterator iter = datalist.begin(); iter != datalist.end(); iter++)
					{
						shared_ptr<ValueData> valuedata = iter->second.lock();
						if (valuedata == NULL)
						{
							continue;
						}

						shared_ptr<ValueHeader> header = valuedata->m_header.lock();
						if (header == NULL)
						{
							continue;
						}

						//父类有变
						std::set<ValueHeader*>::iterator hiter = headerchangelist.find(header.get());
						if (hiter != headerchangelist.end())
						{
							datalisttmp[valuedata.get()] = valuedata;
							headerlisttmp[header.get()] = header;
						}

						//数据有变
						std::set<ValueData*>::iterator aiter = datachangelist.find(valuedata.get());
						if(aiter != datachangelist.end())
						{
							datalisttmp[valuedata.get()] = valuedata;
						}
					}

					datachangelist.clear();
					headerchangelist.clear();
					prevsavetime = nowtime;
				}
				
			}
			for(std::map<ValueHeader*, shared_ptr<ValueHeader> >::iterator iter = headerlisttmp.begin();iter != headerlisttmp.end();iter ++)
			{
				saveHeader(iter->second);
			}
			for(std::map<ValueData*, shared_ptr<ValueData> >::iterator iter = datalisttmp.begin();iter != datalisttmp.end();iter ++)
			{
				saveData(iter->second);
			}
		}
	}
	void saveHeader(const shared_ptr<ValueHeader>& headertmp)
	{
		StoreValueHeader* header = (StoreValueHeader*)headertmp.get();

		std::string headerstr;
		{
			std::map<std::string, Value> headerarray;
			headerarray["type"] = (int)header->m_type;
			headerarray["key"] = header->m_key;
			headerarray["expire"] = header->m_expire;
			headerarray["dbindex"] = header->m_dbindex;
			headerarray[HEADERFLAG] = true;

			buildHeaderString(headerarray, headerstr);
		}
		header->filepos = storer->write(headerstr, "", header->filepos);
	}
	shared_ptr<ValueHeader> parseAndBuildHeader(uint32_t filepos,const std::map<std::string, Value>& headerarray)
	{
		DataType type = (DataType)getHeaderValue(headerarray, "type").readInt();
		std::string key = getHeaderValue(headerarray, "key").readString();
		uint64_t expire = getHeaderValue(headerarray, "expire").readInt64();
		uint32_t dbindex = getHeaderValue(headerarray, "dbindex").readInt();

		shared_ptr<ValueHeader> valueheader = createHeader(key, dbindex, type);
		valueheader->setExpire(expire);
		
		StoreValueHeader* header = (StoreValueHeader*)valueheader.get();
		header->filepos = filepos;

		return valueheader;
	}
	void saveData(const shared_ptr<ValueData>& datatmp)
	{
		StoreValueData* data = (StoreValueData*)datatmp.get();
		shared_ptr<ValueHeader> valueheader = data->m_header.lock();
		if (valueheader == NULL) return;

		if (data->m_len != data->m_data.length()) return;

		StoreValueHeader* header = (StoreValueHeader*)valueheader.get();

		std::string headerstr;
		{
			std::map<std::string, Value> headerarray;
			headerarray["type"] = (int)header->m_type;
			headerarray["key"] = header->m_key;
			headerarray["expire"] = header->m_expire;
			headerarray["dbindex"] = header->m_dbindex;
			headerarray["name"] = data->m_name;
			headerarray["datalen"] = data->len();

			buildHeaderString(headerarray, headerstr);
		}
		std::string datastr;
		data->getData(datastr);

		data->filepos = storer->write(headerstr, datastr, data->filepos);
		if (data->filepos != -1)
		{
			data->prevsavetime = Time::getCurrentMilliSecond();

			std::string emtpystr;
			data->m_data.swap(emtpystr);
		}
	}
	shared_ptr<ValueData> parseAndBuildData(uint32_t filepos,const std::vector<shared_ptr<ValueHeader> >& headerlist,const std::map<std::string, Value>& headerarray)
	{
		DataType type = (DataType)getHeaderValue(headerarray, "type").readInt();
		std::string key = getHeaderValue(headerarray, "key").readString();
		uint64_t expire = getHeaderValue(headerarray, "expire").readInt64();
		uint32_t dbindex = getHeaderValue(headerarray, "dbindex").readInt();
		std::string name = getHeaderValue(headerarray, "name").readString();
		uint32_t datalen = getHeaderValue(headerarray, "datalen").readInt();

		shared_ptr<ValueHeader> header = getDataHeader(headerlist, type, key, dbindex);
		if (header == NULL) return shared_ptr<ValueData>();

		shared_ptr<ValueData> data = createValueData(header, name);
		data->m_len = datalen;

		StoreValueData* valuedata = (StoreValueData*)data.get();
		valuedata->filepos = filepos;

		return data;
	}
	shared_ptr<ValueHeader> getDataHeader(const std::vector<shared_ptr<ValueHeader> >& headerlist,DataType type,const std::string& key,uint32_t dbindex)
	{
		for (uint32_t i = 0; i < headerlist.size(); i++)
		{
			if (headerlist[i]->m_type == type && headerlist[i]->m_key == key && headerlist[i]->m_dbindex == dbindex)
			{
				return headerlist[i];
			}
		}

		return shared_ptr<ValueHeader>();
	}
	void buildHeaderString(const std::map<std::string, Value>& headerarray, std::string& headerstr)
	{
		for (std::map<std::string, Value>::const_iterator iter = headerarray.begin(); iter != headerarray.end(); iter++)
		{
			headerstr += iter->first + "=" + iter->second.readString() + ";";
		}
	}
	void parseHeaderString(const std::string& headerstr, std::map<std::string, Value>& headerarray)
	{
		std::vector<std::string> flagarray = String::split(headerstr, ";");
		for (uint32_t i = 0; i < flagarray.size(); i++)
		{
			std::vector<std::string> keyarray = String::split(flagarray[i], "=");
			if(keyarray.size() != 2) continue;

			headerarray[keyarray[0]] = keyarray[1];
		}
	}
	Value getHeaderValue(const std::map<std::string, Value>& headerarray, const std::string& headerid)
	{
		std::map<std::string, Value>::const_iterator iter = headerarray.find(headerid);
		if (iter == headerarray.end()) return Value();

		return iter->second;
	}
private:
	shared_ptr<Timer>			timer;
	shared_ptr<Storer>			storer;
	Mutex						mutex;
	std::map<ValueHeader*, weak_ptr<ValueHeader> > headerlist;
	std::map<ValueData*, weak_ptr<ValueData> > datalist;

	std::set<ValueHeader*>		headerchangelist;
	std::set<ValueData*>		datachangelist;
	std::set<uint32_t>			deleteposlist;

	uint64_t					prevsavetime;
};

StoreValueHeader::~StoreValueHeader()
{
	StoreFactory* factory = (StoreFactory*)m_factory.get();
	factory->deleteHeader(this, filepos);
}	

StoreValueData::~StoreValueData()
{
	StoreFactory* factory = (StoreFactory*)m_factory.get();
	factory->deleteValueData(this,filepos);
}
	