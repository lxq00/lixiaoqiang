#ifndef __STATICXMMDBUFFER_H__
#define __STATICXMMDBUFFER_H__
#include "Base/Mutex.h"
#include "Base/Semaphore.h"
#include "Base/DynamicMemPool.h"
#include "Base/StaticMemPool.h"
#include "Base/ShareMem.h"
#include "Base/Defs.h"
#include "Base/Func.h"
#include <string>
using namespace  std;
namespace Public{
namespace Base{

///进程间命名互斥锁
class BASE_API NamedMutex:public ILockerObjcPtr
{
	struct NamedMutexInternal;
	NamedMutex(const NamedMutex&);
	NamedMutex& operator=(const NamedMutex&);
public:
	NamedMutex(const std::string& shareName);
	~NamedMutex();
	
	bool tryEnter();
	bool enter();
	bool leave();
private:
	NamedMutexInternal* internal;
};

///进程间命名信号量
class BASE_API NamedSemaphore:public ISemaphoreObjcPtr
{
	NamedSemaphore(const NamedSemaphore&);
	NamedSemaphore& operator=(const NamedSemaphore&);
	struct NamedSemaphoreInternal;
public:
	NamedSemaphore(const std::string& shareName);
	~NamedSemaphore();

	int pend();

	int pend(uint32_t timeout);

	int post();
private:
	NamedSemaphoreInternal* internal;
};

///进程间共享内存
class BASE_API ShareMEMBuffer
{
	struct ShareMEMBufferInternal;
public:
	typedef Function2<void,void*,int>		ReadMEMCallback;
private:
	ShareMEMBuffer(const std::string& shareName,int createBlock,int maxBlock,int rblockSize,int bocknum,int memMaxSize,bool create,void* startAddr,const ReadMEMCallback& callback);
public:
	~ShareMEMBuffer();
	
	static shared_ptr<ShareMEMBuffer> create(const std::string& shareName,int writeBlockSize,int wrteBlockNum,int readBlockSize,int readBlockNum,int memMaxSize,const ReadMEMCallback& callback = NULL);
	static shared_ptr<ShareMEMBuffer> open(const std::string& shareName,int readBlockSize,int readBlockNum,int writeBlockSize,int writeBlockNum,int memMaxSize,void* startAddr,const ReadMEMCallback& callback);
	
	int write(void* block,int size);
	IMallcFreeMemObjcPtr* getMallckFreeObjcPtr();
	char* getShareStartMemBuffer();
private:
	ShareMEMBufferInternal* internal;
};


};
};

#endif //__STATICXMMDBUFFER_H__
