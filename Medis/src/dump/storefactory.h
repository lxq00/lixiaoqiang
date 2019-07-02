#pragma once
#include "value/valuedata.h"
#include "storer.h"


#define SAVEMAXNUM				100
#define SAVEMAXTIMEOUT			60*1000

#define NOTUSESPACETIMEOUT		3*SAVEMAXTIMEOUT

#define HEADER_TYPE		"t"
#define HEADER_KEY		"k"
#define HEADER_EXPIRE	"e"
#define HEADER_INDEX	"i"
#define HEADER_NAME		"n"
#define HEADER_FLAG		"h"

class StoreFactory :public DataFactory
{
public:
	StoreFactory(const std::string& savefile):savefilename(savefile), initflag(false)
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
		shared_ptr<Storer> storer = make_shared<Storer>();
		if (storer->open(savefilename, false))
		{
			std::map<std::string, RedisString> headerinfos;
			storer->load(headerinfos);

			for (std::map<std::string, RedisString>::iterator iter = headerinfos.begin(); iter != headerinfos.end(); iter++)
			{
				std::map<std::string, Value> headerarray;
				parseHeaderString(iter->first, headerarray);

				if (getHeaderValue(headerarray, HEADER_FLAG).readBool())
				{
					shared_ptr<ValueHeader> header = parseAndBuildHeader(headerarray);

					headerlist.push_back(header);
				}
				else
				{
					shared_ptr<ValueData> data = parseAndBuildData(headerarray, headerlist, iter->second);
					if (data != NULL)
					{
						datalist.push_back(data);
					}
				}
			}
		}

		initflag = true;
	}

	virtual shared_ptr<ValueHeader> createHeader(const std::string& key, uint32_t dbindex, DataType _type)
	{
		Guard locker(mutex);

		shared_ptr<ValueHeader> header = make_shared<ValueHeader>(shared_from_this(),key,dbindex,_type);
		headerlist[header.get()] = header;

		return header;
	}

	virtual void updateHeader(ValueHeader* header)
	{
		if (!initflag) return;
		Guard locker(mutex);

		headerchangelist.insert(header);
	}
	virtual void deleteHeader(ValueHeader* header)
	{
		Guard locker(mutex);

		headerlist.erase(header);
		headerchangelist.insert(header);
	}

	virtual shared_ptr<ValueData> createValueData(const shared_ptr<ValueHeader>& header, const std::string& name)
	{
		Guard locker(mutex);

		shared_ptr<ValueData> data = make_shared<ValueData>(shared_from_this(), header, name);
		datalist[data.get()] = data;

		return data;
	}
	virtual void updateValueData(ValueData* data)
	{
		if (!initflag) return;
		Guard locker(mutex);

		datachangelist.insert(data);
	}
	virtual void deleteValueData(ValueData* data)
	{
		Guard locker(mutex);

		datachangelist.insert(data);
		datalist.erase(data);
	}
private:
	void onPoolTimerProc(unsigned long)
	{
		if (headerchangelist.size() + datachangelist.size() == 0) return;

		uint64_t nowtime = Time::getCurrentMilliSecond();

		if (headerchangelist.size() + datachangelist.size() > SAVEMAXNUM || nowtime - prevsavetime > SAVEMAXTIMEOUT)
		{
			prevsavetime = nowtime;

			std::map<ValueHeader*, weak_ptr<ValueHeader> > headerlisttmp;
			std::map<ValueData*, weak_ptr<ValueData> > datalisttmp;
		
			{
				Guard locker(mutex);
				headerlisttmp = headerlist;
				datalisttmp = datalist;

				headerchangelist.clear();
				datachangelist.clear();
			}

			shared_ptr<Storer> storer = make_shared<Storer>();
			if (!storer->open(savefilename, true))
			{
				return;
			}
			for (std::map<ValueHeader*, weak_ptr<ValueHeader> >::iterator iter = headerlist.begin(); iter != headerlist.end(); iter++)
			{
				shared_ptr<ValueHeader> header = iter->second.lock();
				if (header == NULL)
				{
					continue;
				}
				saveHeader(storer,header);
			}
			for (std::map<ValueData*, weak_ptr<ValueData> >::iterator iter = datalisttmp.begin(); iter != datalisttmp.end(); iter++)
			{
				shared_ptr<ValueData> data = iter->second.lock();
				if (data == NULL)
				{
					continue;
				}
				saveData(storer, data);
			}
		}
	}
	void saveHeader(shared_ptr<Storer> storer,const shared_ptr<ValueHeader>& header)
	{
		std::string headerstr;
		{
			std::map<std::string, Value> headerarray;
			headerarray[HEADER_TYPE] = (int)header->m_type;
			headerarray[HEADER_KEY] = header->m_key;
			headerarray[HEADER_EXPIRE] = header->m_expire;
			headerarray[HEADER_INDEX] = header->m_dbindex;
			headerarray[HEADER_FLAG] = 1;

			buildHeaderString(headerarray, headerstr);
		}
		storer->write(headerstr, RedisString());
	}
	shared_ptr<ValueHeader> parseAndBuildHeader(const std::map<std::string, Value>& headerarray)
	{
		DataType type = (DataType)getHeaderValue(headerarray, HEADER_TYPE).readInt();
		std::string key = getHeaderValue(headerarray, HEADER_KEY).readString();
		uint64_t expire = getHeaderValue(headerarray, HEADER_EXPIRE).readInt64();
		uint32_t dbindex = getHeaderValue(headerarray, HEADER_INDEX).readInt();

		shared_ptr<ValueHeader> valueheader = createHeader(key, dbindex, type);
		valueheader->setExpire(expire);

		return valueheader;
	}
	void saveData(shared_ptr<Storer> storer, const shared_ptr<ValueData>& data)
	{
		shared_ptr<ValueHeader> header = data->m_header.lock();
		if (header == NULL) return;

		std::string headerstr;
		{
			std::map<std::string, Value> headerarray;
			headerarray[HEADER_TYPE] = (int)header->m_type;
			headerarray[HEADER_KEY] = header->m_key;
			headerarray[HEADER_EXPIRE] = header->m_expire;
			headerarray[HEADER_INDEX] = header->m_dbindex;
			headerarray[HEADER_NAME] = data->m_name;

			buildHeaderString(headerarray, headerstr);
		}
		storer->write(headerstr, data->m_data);
	}
	shared_ptr<ValueData> parseAndBuildData(const std::map<std::string, Value>& headerarray,const std::vector<shared_ptr<ValueHeader> >& headerlist,const RedisString& datastr)
	{
		DataType type = (DataType)getHeaderValue(headerarray, HEADER_TYPE).readInt();
		std::string key = getHeaderValue(headerarray, HEADER_KEY).readString();
		uint64_t expire = getHeaderValue(headerarray, HEADER_EXPIRE).readInt64();
		uint32_t dbindex = getHeaderValue(headerarray, HEADER_INDEX).readInt();
		std::string name = getHeaderValue(headerarray, HEADER_NAME).readString();

		shared_ptr<ValueHeader> header = getDataHeader(headerlist, type, key, dbindex);
		if (header == NULL) return shared_ptr<ValueData>();

		shared_ptr<ValueData> data = createValueData(header, name);
		data->m_data = datastr;

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
	std::string					savefilename;
	Mutex						mutex;
	std::map<ValueHeader*, weak_ptr<ValueHeader> > headerlist;
	std::map<ValueData*, weak_ptr<ValueData> > datalist;

	std::set<ValueHeader*>		headerchangelist;
	std::set<ValueData*>		datachangelist;

	uint64_t					prevsavetime;

	bool						initflag;
};
