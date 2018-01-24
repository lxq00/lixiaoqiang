#include "Base/StaticMemPool.h"
#include "Base/Math.h"

namespace Xunmei{
namespace Base{

struct StaticMemPool::StaticMemPoolInternal
{
#define MemChunkSize		256

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
	ILockerObjcPtr*	locker;

	NodeIndexList*	listHeader;
	ChunkNode*		nodeHeader;
	shared_ptr<Timer> poolTimer;
private:
	uint32_t getChuckSize(uint32_t _chuckSize)
	{
		if(_chuckSize <= 0)
		{
			return 0;
		}

		int canUsedBlockSize = _chuckSize*MemChunkSize;
		chunkSize = _chuckSize;
		nodeIndexSize = log2i(canUsedBlockSize) - minBlockIdx;

		uint32_t usedHeadersize = chunkSize* sizeof(ChunkNode) + (nodeIndexSize  + 1)* sizeof(NodeIndexList);

		if(bufferMaxSize - canUsedBlockSize >= usedHeadersize)
		{
			return _chuckSize;
		}
		uint32_t freeChunkSize = (usedHeadersize - (bufferMaxSize - canUsedBlockSize)) / MemChunkSize;
		if(freeChunkSize == 0)
		{
			freeChunkSize = 1;
		}

		return getChuckSize(_chuckSize - freeChunkSize);
	}
public:
	StaticMemPoolInternal(char* addr,int size,ILockerObjcPtr* lock,bool create):listHeader(NULL),nodeHeader(NULL)
	{
		bufferStartAddr = (uint8_t*)addr;
		bufferMaxSize = size;
		locker = lock;

		minBlockIdx = log2i(MemChunkSize);


		chunkSize = getChuckSize(bufferMaxSize / MemChunkSize);
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
		uint32_t totalcanUsedSize = MemChunkSize * chunkSize - usedSize;
		while(totalcanUsedSize >= MemChunkSize)
		{
			int freeidx = log2i(totalcanUsedSize) - minBlockIdx;
			listHeader[freeidx].node = &nodeHeader[usedSize/MemChunkSize];
			nodeHeader[usedSize/MemChunkSize].usedIdx = freeidx + minBlockIdx;

			usedSize += 1 << (freeidx + minBlockIdx);
			totalcanUsedSize = MemChunkSize * chunkSize - usedSize;
		}
		poolTimer = new Timer("StaticMemPoolInternal");
		poolTimer->start(Timer::Proc(&StaticMemPoolInternal::poolTimerProc,this),0,5*60*1000);
	}
	~StaticMemPoolInternal()
	{
		poolTimer = NULL;
	}
	void poolTimerProc(unsigned long)
	{
		logtrace("Base StaticMemPool: pool:%x bufferwsize:%llu",this,bufferMaxSize);
	}

	void insertChunk(uint32_t idx,ChunkNode* node)
	{
		uint32_t canAddIdxNext = node->idx + ((1 << (idx + minBlockIdx)) / MemChunkSize);

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

		insertChunk(idx - 1,&nodeHeader[usedNode->idx + ((1 << (idx + minBlockIdx - 1)) / MemChunkSize)]);
		deleteChunk(idx,usedNode);
		usedNode->usedIdx = idx + minBlockIdx - 1;

		return usedNode;
	}

	void* Malloc(uint32_t size,uint32_t& realsize)
	{
		Guard  autol(locker);

		//当size小于MemChunkSize 等于MemChunkSize

		if(size <= MemChunkSize)
		{
			size = MemChunkSize;
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

		return realDataBufferAddr +  usedNode->idx * MemChunkSize;
	}

	void Delete(void* addr)
	{
		Guard  autol(locker);

		if(addr < realDataBufferAddr || addr >= realDataBufferAddr + chunkSize * MemChunkSize)
		{
			return;
		}

		int vecIdx = ((uint8_t*)addr - realDataBufferAddr)/MemChunkSize;
		int idx = nodeHeader[vecIdx].usedIdx - minBlockIdx;

		insertChunk(idx,&nodeHeader[vecIdx]);
	}
};


StaticMemPool::StaticMemPool(char* bufferStartAddr,int bufferSize,ILockerObjcPtr* locker, bool create)
{
	internal = new StaticMemPoolInternal(bufferStartAddr, bufferSize, locker, create);
}

StaticMemPool::~StaticMemPool()
{
	delete internal;
}

void* StaticMemPool::Malloc(uint32_t size,uint32_t& realsize)
{
	return internal->Malloc(size, realsize);
}

void StaticMemPool::Free(void* pAddr)
{
	internal->Delete(pAddr);
}

} // namespace Base
} // namespace Xunmei

