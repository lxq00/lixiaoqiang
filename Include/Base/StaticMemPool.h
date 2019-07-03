#ifndef __STATICMEMPOOL_H__
#define __STATICMEMPOOL_H__
#include "Base/Defs.h"
#include "Base/IntTypes.h"
#include "Base/Base.h"
namespace Public{
namespace Base{

#define DefaultMemChunkSize		256

class BASE_API StaticMemPool:public IMempoolInterface
{
	struct StaticMemPoolInternal;
public:
	StaticMemPool(char* bufferStartAddr,int bufferSize,IMutexInterface* locker,bool create,uint32_t chunksize = DefaultMemChunkSize);
	~StaticMemPool();

	void* Malloc(uint32_t size,uint32_t& realsize);
	
	bool Free(void*);

	uint32_t maxBufferSize();

	uint32_t usedBufferSize();
private:
	StaticMemPoolInternal *internal;
};

} // namespace Base
} // namespace Public

#endif //__STATICMEMPOOL_H__
