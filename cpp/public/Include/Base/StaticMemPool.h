#ifndef __STATICMEMPOOL_H__
#define __STATICMEMPOOL_H__
#include "Base/Defs.h"
#include "Base/IntTypes.h"
#include "Base/Base.h"
namespace Public{
namespace Base{

class BASE_API StaticMemPool:public IMallcFreeMemObjcPtr
{
	struct StaticMemPoolInternal;
public:
	StaticMemPool(char* bufferStartAddr,int bufferSize,ILockerObjcPtr* locker,bool create);
	~StaticMemPool();

	void* Malloc(uint32_t size,uint32_t& realsize);
	
	void Free(void*);

private:
	StaticMemPoolInternal *internal;
};

} // namespace Base
} // namespace Public

#endif //__STATICMEMPOOL_H__
