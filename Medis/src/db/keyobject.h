#pragma once

#include "db.h"


class KeyObject
{
public:
	KeyObject(const shared_ptr<IOWorker>& worker, const shared_ptr<DataFactory>& factory)
	{
		for (int i = 0; i < MAXDBINDEX; i++)
		{
			dblist.push_back(make_shared<DB>(worker,factory,i));
		}

		poolTimer = make_shared<Timer>("KeyObject");
		poolTimer->start(Timer::Proc(&KeyObject::onPoolTimerProc, this), 0, 1000);

	}
	~KeyObject()
	{
		poolTimer = NULL;
	}
	bool initKeyObject(const std::vector<shared_ptr<ValueHeader> >& headerlist,const  std::vector<shared_ptr<ValueData> >& datalist)
	{
		for (uint32_t i = 0; i < headerlist.size(); i++)
		{
			if (headerlist[i]->dbindex() < 0 || headerlist[i]->dbindex() > MAXDBINDEX)
			{
				continue;
			}

			dblist[headerlist[i]->dbindex()]->initHeader(headerlist[i]);
		}
		for (uint32_t i = 0; i < datalist.size(); i++)
		{
			shared_ptr<ValueHeader> header = datalist[i]->header();
			if(header == NULL) continue;

			if (header->dbindex() < 0 || header->dbindex() > MAXDBINDEX)
			{
				continue;
			}

			dblist[header->dbindex()]->initData(datalist[i]);
		}

		return true;
	}
	void inputConnectionData(const CmdResultCallback& callback,void* user, int dbindex,const shared_ptr<RedisValue>& value)
	{
		CmdResultCallback callbacktmp = callback;

		if (dbindex < 0 || dbindex >= MAXDBINDEX)
		{
			callbacktmp(user, RedisValue(false, "invalid DB index"));
		}
		else
		{
			dblist[dbindex]->inputCommand(callback, user, value);
		}
	}
	bool info(std::vector<uint32_t>& keyinfo)
	{
		for (uint32_t i = 0; i < MAXDBINDEX; i++)
		{
			keyinfo.push_back(dblist[i]->keyscount());
		}

		return true;
	}
private:
	void onPoolTimerProc(unsigned long)
	{
		for (uint32_t i = 0; i < MAXDBINDEX; i++)
		{
			dblist[i]->poolTimerProc();
		}
	}
private:
	std::vector<shared_ptr<DB> > dblist;
	shared_ptr<Timer>			 poolTimer;
};