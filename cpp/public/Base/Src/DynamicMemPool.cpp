#include "Base/DynamicMemPool.h"
#include "Base/IntTypes.h"
#include <stdio.h>
#include <stdlib.h>
#include "Base/PrintLog.h"
#include "Base/Math.h"
#include "Base/Mutex.h"
#include "Base/Guard.h"
#include "Base/BaseTemplate.h"
#include "Base/Thread.h"
#include "Base/Timer.h"
#include "Base/Time.h"
#include "Base/Singleton.h"
#include <map>
#include <list>
using namespace std;
namespace Xunmei{
namespace Base{

class DynamicMemPool::MemPoolInternal
{
#define MAXTIMEOUT			10*1000
private:

	typedef struct _BlackSize
	{
		uint32_t 			idx;
		_BlackSize*			next;
	}BlackSize;

	
	typedef struct _BlackNode
	{
		uint32_t 	idx;
		char*		addr;
		uint64_t 	lastUsedTime;
		_BlackNode*	next;
	}BlackNode;


	Mutex 							mutex;
	BlackSize*						backSiseTable;
	std::map<void*,BlackNode*> 		memList;
	BlackNode*						idleInfoList[32];
	uint64_t						totalMemSize;
	uint64_t						prevPrintMemTime;

	Timer*							freeTimer;
public:
	MemPoolInternal():backSiseTable(NULL),totalMemSize(0),prevPrintMemTime(0)
	{
#define KB			1024
#define MB			1024*1024
#define GB			1024*1024*1024

		addChunkSize(1*KB);
		addChunkSize(2*KB);
		addChunkSize(4*KB);
		addChunkSize(8*KB);
		addChunkSize(16*KB);
		addChunkSize(32*KB);
		addChunkSize(64*KB);
		addChunkSize(128*KB);
		addChunkSize(256*KB);
		addChunkSize(512*KB);
		addChunkSize(1*MB);
		addChunkSize(2*MB);
		addChunkSize(4*MB);
		addChunkSize(8*MB);
		addChunkSize(16*MB);
		addChunkSize(32*MB);
		addChunkSize(64*MB);
		addChunkSize(128*MB);
		addChunkSize(256*MB);
		addChunkSize(512*MB);
		addChunkSize(1*GB);

		for(int i = 0;i < 32;i ++)
		{
			idleInfoList[i] = NULL;
		}

		freeTimer = new Timer("MemPoolInternal free");
		freeTimer->start(Timer::Proc(&MemPoolInternal::freeTimerProc,this),0,MAXTIMEOUT/3);
	}
	
	~MemPoolInternal()
	{
		SAFE_DELETE(freeTimer);
		Guard locker(mutex);

		std::map<void*,BlackNode*>::iterator biter;
		for(biter = memList.begin();biter != memList.end();biter ++)
		{
			delete[] biter->second->addr;
			delete biter->second;
		}

		memList.clear();

		BlackSize* black = backSiseTable;
		while(black != NULL)
		{
			BlackSize* next = black->next;
			
			delete black;
			black = next;
		}
		
		backSiseTable = NULL;
	}
	void* Malloc(uint32_t size,uint32_t& realsize)
	{
		uint32_t idx = log2i(size);
		realsize = getChunkSize(idx);
		if(realsize == 0)
		{
			return NULL;
		}

		Guard locker(mutex);

		if(idleInfoList[idx] == NULL)
		{
			BlackNode* node = new BlackNode;
			node->idx = idx;
			node->addr = new(std::nothrow) char[realsize];
			node->next = NULL;
			if(node->addr == NULL)
			{
				delete node;
				return NULL;
			}
			totalMemSize += realsize;
			memList[node->addr] = node;

			return node->addr;
		}

		BlackNode* node = idleInfoList[idx];
		idleInfoList[idx] = node->next;

		memList[node->addr] = node;

		return node->addr;
	}
	void Free(void* ptr)
	{
		Guard locker(mutex);
		std::map<void*,BlackNode*>::iterator iter = memList.find(ptr);
		if(iter == memList.end())
		{
			return;
		}

		BlackNode* node = iter->second;
		node->next = idleInfoList[node->idx];
		idleInfoList[node->idx] = node;
		node->lastUsedTime = Time::getCurrentMilliSecond();
	}
private:
	void addChunkSize(uint32_t size)
	{
		uint32_t idx = log2i(size);

		BlackSize* black = backSiseTable;
		BlackSize* preBlack = NULL;
		while(black != NULL)
		{
			if(black->idx == idx)
			{
				return;
			}
			preBlack = black;
			black = black->next;
		}
		BlackSize* newblack = new BlackSize;
		newblack->idx = idx;
		newblack->next = NULL;
		if (preBlack)
		{
			preBlack->next = newblack;
		}
		if (backSiseTable == NULL)
		{
			backSiseTable= newblack;
		}
	}
	uint32_t getChunkSize(uint32_t idx)
	{		
		BlackSize* black = backSiseTable;
		while(black != NULL)
		{
			if(idx < black->idx)
			{
				return 1 << black->idx;
			}
			black = black->next;
		}
		
		return 0;
	}
	void freeTimerProc(unsigned long param)
	{
#define MAXPRINTMEMTIMEOUT		5*60*1000

		Guard locker(mutex);

		if(Time::getCurrentMilliSecond() < prevPrintMemTime || Time::getCurrentMilliSecond() - prevPrintMemTime >= MAXPRINTMEMTIMEOUT)
		{
			logtrace("Base DynamicMemPool:%x totalmemsize:%llu totalnodesize:%d",this,totalMemSize,memList.size());

			prevPrintMemTime = Time::getCurrentMilliSecond();
		}

		uint64_t nowtime = Time::getCurrentMilliSecond();

		for(int i = 0;i < 32;i ++)
		{
			while(idleInfoList[i] != NULL && (nowtime < idleInfoList[i]->lastUsedTime || nowtime - idleInfoList[i]->lastUsedTime >= MAXTIMEOUT))
			{
				BlackNode* node = idleInfoList[i];
				idleInfoList[i] = idleInfoList[i]->next;
				memList.erase(node->addr);
				totalMemSize -= getChunkSize(node->idx);
				delete []node->addr;
				delete node;
			}
			
			if(idleInfoList[i] == NULL)
			{
				continue;
			}

			for(BlackNode* node = idleInfoList[i];node != NULL && node->next != NULL;node = node->next)
			{
				BlackNode* next = node->next;
				if(nowtime < next->lastUsedTime || nowtime - next->lastUsedTime >= MAXTIMEOUT)
				{
					node->next = next->next;
					memList.erase(next->addr);
					totalMemSize -= getChunkSize(node->idx);
					delete []next->addr;
					delete next;
				}
			}			
		}
	}
};




//#define UseMempool

DynamicMemPool::DynamicMemPool()
{
#ifdef UseMempool
	internal = new(std::nothrow) DynamicMemPool::MemPoolInternal();
#endif
}
DynamicMemPool::~DynamicMemPool()
{
#ifdef UseMempool
	SAFE_DELETE(internal);
#endif
}
void* DynamicMemPool::Malloc(uint32_t size,uint32_t& realsize)
{
#ifdef UseMempool
	return internal->Malloc(size,realsize);
#else
	void* newbuffer = ::malloc(size);
	realsize = size;

	return newbuffer;
#endif
}
void DynamicMemPool::Free(void* addr)
{
#ifdef UseMempool
	if(addr != NULL)
	{
		internal->Free(addr);
	}	
#else
	if(addr != NULL)
	{
		::free(addr);
	}
#endif
}

static Mutex	mutex;

DynamicMemPool* DynamicMemPool::instance()
{
	Guard locker(mutex);
	static DynamicMemPool mempool;

	return &mempool;
	//return CreateSingleton<DynamicMemPool>(SINGLETON_Levl_Base,3);
}


};
};
