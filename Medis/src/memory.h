#pragma once

#include "Base/Base.h"
using namespace Public::Base;

#define MAXSHAREDMEMSIZE	128*1024*1024

#define FREENOTUSEDTIMEOUT		60*1000

class Memory
{
	struct MemNode
	{
		shared_ptr<ShareMem>		sharemem;
		shared_ptr<StaticMemPool>	memPool;
		uint64_t					prevusedtime;

		MemNode():prevusedtime(Time::getCurrentMilliSecond())
		{
			char buffer[256];
			snprintf_x(buffer,255, "medis_%06x", this);

			sharemem = ShareMem::create(buffer, MAXSHAREDMEMSIZE);
			if (sharemem == NULL) return;
			memPool = make_shared<StaticMemPool>((char*)sharemem->getbuffer(), MAXSHAREDMEMSIZE, (IMutexInterface*)NULL, true);
		}
	};
public:
	Memory() 
	{
		timer = make_shared<Timer>("Memory");
		timer->start(Timer::Proc(&Memory::onPoolTimerProc, this), 0, 10000);
	}
	~Memory() {}

	void* Malloc(uint32_t size)
	{
		Guard locker(mutex);

		uint32_t realsize = 0;

		for (uint32_t i = 0; i < memlist.size(); i++)
		{
			void* ptr = memlist[i]->memPool->Malloc(size, realsize);
			if (ptr != NULL)
			{
				memlist[i]->prevusedtime = Time::getCurrentMilliSecond();
				return ptr;
			}
		}

		shared_ptr<MemNode> node = make_shared<MemNode>();
		if (node->memPool == NULL)
		{
			assert(0);
			return NULL;
		}
		void* ptr = node->memPool->Malloc(size, realsize);
		if (ptr == NULL)
		{
			assert(0);
			return NULL;
		}
		node->prevusedtime = Time::getCurrentMilliSecond();
		memlist.push_back(node);

		return ptr;
	}
	void Free(void* ptr)
	{
		Guard locker(mutex);
		for (uint32_t i = 0; i < memlist.size(); i++)
		{
			if (memlist[i]->memPool->Free(ptr))
			{
				memlist[i]->prevusedtime = Time::getCurrentMilliSecond();
				return;
			}
		}
	}
	static Memory* instance()
	{
		static Memory memory;
		return &memory;
	}
private:
	void onPoolTimerProc(unsigned long)
	{
		std::list<shared_ptr<MemNode> > freelist;

		{
			Guard locker(mutex);
			uint64_t nowtime = Time::getCurrentMilliSecond();
			for (std::vector<shared_ptr<MemNode> >::iterator iter = memlist.begin(); iter != memlist.end();)
			{
				if (nowtime > (*iter)->prevusedtime && nowtime - (*iter)->prevusedtime > FREENOTUSEDTIMEOUT &&
					(*iter)->memPool->usedBufferSize() == 0)
				{
					freelist.push_back(*iter);
					memlist.erase(iter++);
				}
				else
				{
					iter++;
				}
			}
		}
	}
private:
	shared_ptr<Timer>					timer;
	Mutex								mutex;
	std::vector<shared_ptr<MemNode> >	memlist;
};