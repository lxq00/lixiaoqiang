#include "Base/StaticMemPool.h"
#include "Base/Math.h"

namespace Public{
namespace Base{

struct StaticMemPool::StaticMemPoolInternal
{
	typedef struct _ChunkNode {
		uint32_t idx;
		uint32_t usedIdx;
		_ChunkNode *next;
	}ChunkNode;

	typedef struct NodeIndexList {
		ChunkNode *node;
	}NodeIndexList;

	uint8_t*	bufferStartAddr;
	uint8_t*	realDataBufferAddr;
	uint32_t	bufferMaxSize;
	uint32_t	chunkSize;
	uint32_t	nodeIndexSize;
	uint32_t	minBlockIdx;
	IMutexInterface* locker;

	NodeIndexList*	listHeader;
	ChunkNode*		nodeHeader;

	uint32_t	 bufferUsedSize;
	uint32_t		memChunkSize;
//	shared_ptr<Timer> poolTimer;
private:
	uint32_t getChuckSize(uint32_t _chuckSize)
	{
		if(_chuckSize <= 0)
		{
			return 0;
		}

		int canUsedBlockSize = _chuckSize* memChunkSize;
		chunkSize = _chuckSize;
		nodeIndexSize = log2i(canUsedBlockSize) - minBlockIdx;

		uint32_t usedHeadersize = chunkSize* sizeof(ChunkNode) + (nodeIndexSize  + 1)* sizeof(NodeIndexList);

		if(bufferMaxSize - canUsedBlockSize >= usedHeadersize)
		{
			return _chuckSize;
		}
		uint32_t freeChunkSize = (usedHeadersize - (bufferMaxSize - canUsedBlockSize)) / memChunkSize;
		if(freeChunkSize == 0)
		{
			freeChunkSize = 1;
		}

		return getChuckSize(_chuckSize - freeChunkSize);
	}
public:
	StaticMemPoolInternal(char* addr,int size,IMutexInterface* lock,bool create, uint32_t chunksize):listHeader(NULL),nodeHeader(NULL),memChunkSize(chunksize)
	{
		bufferUsedSize = 0;
		bufferStartAddr = (uint8_t*)addr;
		bufferMaxSize = size;
		locker = lock;

		minBlockIdx = log2i(memChunkSize);


		chunkSize = getChuckSize(bufferMaxSize / memChunkSize);
		if(chunkSize == 0)
		{
			return;
		}
		listHeader = (NodeIndexList*)bufferStartAddr;
		nodeHeader = (ChunkNode*)(bufferStartAddr + (nodeIndexSize  + 1)* sizeof(NodeIndexList));

		realDataBufferAddr = bufferStartAddr + chunkSize* sizeof(ChunkNode) + (nodeIndexSize  + 1)* sizeof(NodeIndexList);

		if(!create)
		{
			return;
		}

		for(uint32_t i = 0;i < chunkSize;i ++)
		{
			nodeHeader[i].idx = i;
			nodeHeader[i].next = NULL;
			nodeHeader[i].usedIdx = 0;
		}
		for(uint32_t i = 0;i < nodeIndexSize;i ++)
		{
			listHeader[i].node = NULL;
		}
		listHeader[nodeIndexSize].node = &nodeHeader[0];
		nodeHeader[0].usedIdx = nodeIndexSize + minBlockIdx;
		
		uint32_t usedSize = 1 << (nodeIndexSize + minBlockIdx);
		uint32_t totalcanUsedSize = memChunkSize * chunkSize - usedSize;
		while(totalcanUsedSize >= memChunkSize)
		{
			int freeidx = log2i(totalcanUsedSize) - minBlockIdx;
			listHeader[freeidx].node = &nodeHeader[usedSize/ memChunkSize];
			nodeHeader[usedSize/ memChunkSize].usedIdx = freeidx + minBlockIdx;

			usedSize += 1 << (freeidx + minBlockIdx);
			totalcanUsedSize = memChunkSize * chunkSize - usedSize;
		}
		//poolTimer = make_shared<Timer>("StaticMemPoolInternal");
		//poolTimer->start(Timer::Proc(&StaticMemPoolInternal::poolTimerProc,this),0,5*60*1000);
	}
	~StaticMemPoolInternal()
	{
	//	poolTimer = NULL;
	}
	void poolTimerProc(unsigned long)
	{
		logtrace("Base StaticMemPool: pool:%x bufferwsize:%llu",this,bufferMaxSize);
	}

	void insertChunk(uint32_t idx,ChunkNode* node)
	{
		uint32_t canAddIdxNext = node->idx + ((1 << (idx + minBlockIdx)) / memChunkSize);

		ChunkNode* pnode = listHeader[idx].node;
		while(pnode != NULL)
		{
			if(pnode->idx == canAddIdxNext)
			{
				insertChunk(idx + 1,node);
				deleteChunk(idx,pnode);
				return;
			}
			else if(pnode->idx == canAddIdxNext)
			{
				insertChunk(idx + 1,pnode);
				deleteChunk(idx,pnode);
				return;
			}
			else
			{
				break;
			}
			pnode = pnode->next;
		}

		node->next = listHeader[idx].node;
		node->usedIdx = idx + minBlockIdx;

		listHeader[idx].node = node;
	}
	void deleteChunk(uint32_t idx,ChunkNode* node)
	{
		if(listHeader[idx].node == node)
		{
			listHeader[idx].node = node->next;
		}
		else
		{
			ChunkNode* pnode = listHeader[idx].node;
			while(pnode != NULL)
			{
				if(pnode->next == node)
				{
					pnode->next = node->next;
					break;
				}
				pnode = pnode->next;
			}
		}
	}
	ChunkNode* getBlockFromParent(uint32_t idx)
	{
		if(idx > nodeIndexSize)
		{
			return NULL;
		}

		ChunkNode* usedNode = listHeader[idx].node;

		if(usedNode == NULL)
		{
			usedNode = getBlockFromParent(idx + 1);
		}
		if(usedNode == NULL)
		{
			return NULL;
		}

		insertChunk(idx - 1,&nodeHeader[usedNode->idx + ((1 << (idx + minBlockIdx - 1)) / memChunkSize)]);
		deleteChunk(idx,usedNode);
		usedNode->usedIdx = idx + minBlockIdx - 1;

		return usedNode;
	}

	void* Malloc(uint32_t size,uint32_t& realsize)
	{
		Guard  autol(locker);

		//当size小于memChunkSize 等于memChunkSize

		if(size <= memChunkSize)
		{
			size = memChunkSize;
		}
		uint32_t vecIdx = Base::log2i(size) - minBlockIdx;

		if (size > (uint32_t)(1 << (vecIdx + minBlockIdx)))
		{
			vecIdx++;
		}


		ChunkNode* usedNode = listHeader[vecIdx].node;
		if(usedNode == NULL)
		{
			usedNode = getBlockFromParent(vecIdx + 1);
		}
		else
		{
			deleteChunk(vecIdx,usedNode);
		}
		if(usedNode == NULL)
		{
			return NULL;
		}

		realsize = 1 << usedNode->usedIdx;

		bufferUsedSize += realsize;

		return realDataBufferAddr +  usedNode->idx * memChunkSize;
	}

	bool Free(void* addr)
	{
		Guard  autol(locker);

		if(addr < realDataBufferAddr || addr >= realDataBufferAddr + chunkSize * memChunkSize)
		{
			return false;
		}

		int vecIdx = ((uint8_t*)addr - realDataBufferAddr)/memChunkSize;
		int idx = nodeHeader[vecIdx].usedIdx - minBlockIdx;

		uint32_t realsize = 1 << nodeHeader[vecIdx].usedIdx;

		bufferUsedSize -= realsize;

		insertChunk(idx,&nodeHeader[vecIdx]);

		return true;
	}
};


StaticMemPool::StaticMemPool(char* bufferStartAddr,int bufferSize,IMutexInterface* locker, bool create, uint32_t chunksize)
{
	if (chunksize == 0) chunksize = DefaultMemChunkSize;

	internal = new StaticMemPoolInternal(bufferStartAddr, bufferSize, locker, create,chunksize);
}

StaticMemPool::~StaticMemPool()
{
	delete internal;
}

void* StaticMemPool::Malloc(uint32_t size,uint32_t& realsize)
{
	return internal->Malloc(size, realsize);
}

bool StaticMemPool::Free(void* pAddr)
{
	return internal->Free(pAddr);
}

uint32_t StaticMemPool::maxBufferSize()
{
	return internal->bufferMaxSize;
}

uint32_t StaticMemPool::usedBufferSize()
{
	return internal->bufferUsedSize;
}

} // namespace Base
} // namespace Public

