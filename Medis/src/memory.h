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
			sprintf(buffer, "medis_%x", this);

			sharemem = ShareMem::create(buffer, MAXSHAREDMEMSIZE);
			memPool = make_shared<StaticMemPool>((char*)sharemem->getbuffer(), MAXSHAREDMEMSIZE, NULL, true);
		}
	};
public:
	Memory() {}
	~Memory() {}

private:
	void onPoolTimerProc(unsigned long)
	{

	}
private:
	shared_ptr<Timer>					timer;
	Mutex								mutex;
	std::vector<shared_ptr<MemNode> >	memlist;
};