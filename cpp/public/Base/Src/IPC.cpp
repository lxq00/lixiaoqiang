#include "Base/IPC.h"
#include "Base/PrintLog.h"
#include "Base/BaseTemplate.h"
#include "Base/Thread.h"
namespace Public{
namespace Base{

#ifdef WIN32
#define INCREMENT_AMOUNT	1
#define LONG_MAX      2147483647L   /* maximum (signed) long value */
#include <windows.h>
struct NamedMutex::NamedMutexInternal
{
	HANDLE mutex;

	NamedMutexInternal(const std::string& shareName):mutex(NULL)
	{
		mutex = CreateMutexA(NULL,FALSE,shareName.c_str());
		if(mutex == NULL)
		{
			logerror("%s error %d\r\n","CreateMutexA",GetLastError());
		}
	}
	~NamedMutexInternal()
	{
		if(mutex != NULL)
		{
			CloseHandle(mutex);
		}
	}
	void lock()
	{
		if(mutex == NULL)
		{
			return;
		}

		DWORD ret = WaitForSingleObject(mutex,INFINITE);
		if(ret == WAIT_OBJECT_0)
		{
			return;
		}
		else
		{
			logerror("WaitForSingleObject error %d\r\n",GetLastError());
		}
	}
	bool trylock()
	{
		if(mutex == NULL)
		{
			return false;
		}
		DWORD ret = WaitForSingleObject(mutex,0);
		if(ret != WAIT_OBJECT_0)
		{
			return false;
		}

		return true;
	}
	void unlock()
	{
		if(mutex == NULL)
		{
			return;
		}

		ReleaseMutex(mutex);
	}
};
struct NamedSemaphore::NamedSemaphoreInternal
{
	HANDLE sem;

	NamedSemaphoreInternal(const std::string& shareName):sem(NULL)
	{
		sem = CreateSemaphore(NULL, 0, LONG_MAX, shareName.c_str());
		if(sem == NULL)
		{
			logerror("%s error %d\r\n","CreateSemaphore",GetLastError());
		}
	}
	~NamedSemaphoreInternal()
	{
		if(sem != NULL)
		{
			CloseHandle(sem);
		}
	}

	int pend()
	{
		return WaitForSingleObject(sem, INFINITE) == WAIT_OBJECT_0;
	}

	int pend(uint32_t timeout)
	{
		DWORD ret = WaitForSingleObject(sem, timeout);
		if (ret == WAIT_OBJECT_0)
			return 0;
		else 
			return -1;
	}

	int post()
	{
		return ReleaseSemaphore(sem, INCREMENT_AMOUNT, NULL);
	}
};
#else
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <semaphore.h>
struct NamedMutex::NamedMutexInternal
{
	sem_t* sem;
	NamedMutexInternal(const std::string& shareName):sem(0)
	{
		sem = sem_open(shareName.c_str(),O_CREAT,S_IRWXU | S_IRWXG | S_IRWXO,1);
		if(sem == SEM_FAILED)
		{
			sem = 0;
		}
	}
	~NamedMutexInternal()
	{
		if(sem != 0)
		{
			sem_close(sem);
		}
	}
	void lock()
	{
		if(sem == 0)
		{
			return;
		}
		int err;
		do 
		{
			err = sem_wait(sem);
		} while (err && errno == EINTR);
	}
	bool trylock()
	{
		if(sem == 0)
		{
			return false;
		}
		return sem_trywait(sem) == 0;
	}
	void unlock()
	{
		if(sem == 0)
		{
			return;
		}
		sem_post(sem);
	}
};
struct NamedSemaphore::NamedSemaphoreInternal
{
	int sem;
	NamedSemaphoreInternal(const std::string& shareName):sem(0)
	{
		key_t id = ftok(shareName.c_str(),1);
		sem = semget(id,1,0666|IPC_CREAT);
		
		int val = 1;
		semctl(sem,0,SETVAL,val);
		
	}
	~NamedSemaphoreInternal()
	{
		int semv = 1;
		semctl(sem,0,IPC_RMID,semv);
	}

	int pend()
	{
		struct sembuf semb;
		semb.sem_num = 0;
		semb.sem_op = -1;
		semb.sem_flg = SEM_UNDO;

		semop(sem,&semb,1);

		return 0;
	}

	int pend(uint32_t timeout)
	{
		int ret = 0;
		struct timespec timeSpec;
		clock_gettime(CLOCK_REALTIME,&timeSpec);
		timeSpec.tv_sec += (timeout < 1000)?1:(timeout/1000);

		struct sembuf semb;
		semb.sem_num = 0;
		semb.sem_op = -1;
		semb.sem_flg = SEM_UNDO;

		ret = semtimedop(sem,&semb,1,&timeSpec);

		return ret;
	}

	int post()
	{
		struct sembuf semb;
		semb.sem_num = 0;
		semb.sem_op = 1;
		semb.sem_flg = SEM_UNDO;

		semop(sem,&semb,1);

		return 0;
	}
};
#endif


NamedMutex::NamedMutex(const std::string& shareName)
{
	internal = new NamedMutexInternal(shareName);
}
NamedMutex::~NamedMutex()
{
	SAFE_DELETE(internal);
}

bool NamedMutex::enter()
{
	if(internal == NULL)
	{
		return false;
	}
	internal->lock();

	return true;
}
bool NamedMutex::tryEnter()
{
	if(internal == NULL)
	{
		return false;
	}

	return internal->trylock();
}
bool NamedMutex::leave()
{
	if(internal == NULL)
	{
		return false;
	}
	internal->unlock();

	return true;
}

NamedSemaphore::NamedSemaphore(const std::string& shareName)
{
	internal = new NamedSemaphoreInternal(shareName);
}
NamedSemaphore::~NamedSemaphore()
{
	SAFE_DELETE(internal);
}

int NamedSemaphore::pend()
{
	if(internal == NULL)
	{
		return -1;
	}

	return internal->pend();
}

int NamedSemaphore::pend(uint32_t timeout)
{
	if(internal == NULL)
	{
		return -1;
	}

	return internal->pend(timeout);
}

int NamedSemaphore::post()
{
	if(internal == NULL)
	{
		return -1;
	}

	return internal->post();
}

struct ShareMEMBuffer::ShareMEMBufferInternal:public Thread
{
	shared_ptr<ShareMem>		sharemem;
	shared_ptr<StaticMemPool>	memPool;
	shared_ptr<NamedMutex>		memmutex;


	char*						createWriteList;
	int*						createWritePos;
	int*						createReadPos;
	int							createMaxSize;
	int							createBlockSize;
	shared_ptr<NamedMutex>		createMutex;
	shared_ptr<NamedSemaphore>  createSem;
	

	char*						readWriteList;
	int*						readWritePos;
	int*						readReadPos;
	int							readMaxSize;
	int							readBlockSize;
	shared_ptr<NamedMutex>		readMutex;
	shared_ptr<NamedSemaphore>	readSem;

	bool						create;
	
	ShareMEMBuffer::ReadMEMCallback	callback;

	ShareMEMBufferInternal(const std::string& shareName,int createBlock,int maxBlock,int rblockSize,int maxrblock,int memMaxSize,bool _create,void* startAddr,const ReadMEMCallback& _callback)
		:Thread("ShareMEMBufferInternal"),createWriteList(NULL),createWritePos(NULL),createReadPos(NULL)
		,createMaxSize(0),createBlockSize(0),readWriteList(NULL),readWritePos(NULL)
		,readReadPos(NULL),readMaxSize(0),readBlockSize(0),create(_create),callback(_callback)
	{
		createMaxSize = createBlock;
		createBlockSize = maxBlock;
		readMaxSize = rblockSize;
		readBlockSize = maxrblock;


		uint32_t maxcommnusize = createMaxSize*createBlockSize + readMaxSize*readBlockSize + sizeof(int)*4;
		uint32_t maxmemSize = maxcommnusize + memMaxSize;

		if(create)
		{
			sharemem = ShareMem::create(shareName,maxmemSize);
		}
		else
		{
			sharemem = ShareMem::open(shareName,maxmemSize,startAddr);
		}

		if (sharemem == NULL)
		{
			return;
		}

		memmutex = new NamedMutex(shareName + "_mempool.lock");
		memPool = new StaticMemPool((char*)sharemem->getbuffer() + maxcommnusize,memMaxSize - maxcommnusize,memmutex.get(),create);


		createWriteList = (char*)sharemem->getbuffer();
		createWritePos = (int*)(createWriteList + createMaxSize*createBlockSize);
		createReadPos = (int*)((char*)createWritePos + sizeof(int));

		createMutex = new NamedMutex(shareName + "_writelist.lock");
		createSem = new NamedSemaphore(shareName + "_writelist.sem");


		readWriteList = (char*)createReadPos + sizeof(int);
		readWritePos = (int*)(readWriteList + readMaxSize*readBlockSize);
		readReadPos = (int*)((char*)readWritePos + sizeof(int));

		readMutex = new NamedMutex(shareName + "_readlist.lock");
		readSem = new NamedSemaphore(shareName + "_readlist.sem");

		createThread();
	}

	~ShareMEMBufferInternal()
	{
		destroyThread();
	}
	void threadProc()
	{
		while(looping())
		{
			if(create)
			{
				readReadList();
			}
			else
			{
				readCreateList();
			}
		}
	}

	void readCreateList()
	{
		if(createSem->pend(100) < 0)
		{
			return;
		}

		{
			Guard locker(createMutex);
			if(*createReadPos == *createWritePos)
			{
				return;
			}
		}
		callback(createWriteList + *createReadPos * createBlockSize,createBlockSize);
		{
			Guard locker(createMutex);

			if(++(*createReadPos) == createMaxSize)
			{
				*createReadPos = 0;
				if ((*createWritePos) == createMaxSize)
				{
					(*createWritePos) = 0;
				}
			}
		}
	}

	void readReadList()
	{
		if(readSem->pend(100) < 0)
		{
			return;
		}

		{
			Guard locker(readMutex);
			if(*readReadPos == *readWritePos)
			{
				return;
			}
		}
		callback(readWriteList + *readReadPos * readBlockSize,readBlockSize);
		{
			Guard locker(readMutex);

			if(++(*readReadPos) == readMaxSize)
			{
				*readReadPos = 0;
				if ((*readWritePos) == readMaxSize)
				{
					(*readWritePos) = 0;
				}
			}
		}
	}

	int createWriteData(void* data,int size)
	{
		if(size != createBlockSize)
		{
			return -1;
		}

		Guard locker(createMutex);
		if (*createWritePos >= createMaxSize)
		{
			if (*createReadPos == createMaxSize - 1)
			{
				*createWritePos = 0;
			}
			else
			{
				return 0;
			}
		}

		memcpy(createWriteList + *createWritePos*createBlockSize,data,createBlockSize);
		++(*createWritePos);
		createSem->post();
		return size;
	}

	int readWriteData(void* data,int size)
	{
		if(size != readBlockSize)
		{
			return -1;
		}

		Guard locker(readMutex);
		if (*readWritePos >= readMaxSize)
		{
			if (*readReadPos == readMaxSize - 1)
			{
				*readWritePos = 0;
			}
			else
			{
				return 0;
			}
		}

		memcpy(readWriteList + *readWritePos*readBlockSize,data,readBlockSize);
		++(*readWritePos);
		readSem->post();
		return size;
	}
};

ShareMEMBuffer::ShareMEMBuffer(const std::string& shareName,int createBlock,int maxBlock,int rblockSize,int bocknum,int memMaxSize,bool create,void* startAddr,const ReadMEMCallback &callback)
{
	internal = new ShareMEMBufferInternal(shareName,createBlock,maxBlock,rblockSize,bocknum,memMaxSize,create,startAddr,callback);
}
ShareMEMBuffer::~ShareMEMBuffer()
{
	SAFE_DELETE(internal);
}

shared_ptr<ShareMEMBuffer> ShareMEMBuffer::create(const std::string& shareName,int writeBlockSize,int wrteBlockNum,int readBlockSize,int readBlockNum,int memMaxSize,const ReadMEMCallback& callback)
{
	shared_ptr<ShareMEMBuffer> membuf;
	membuf = new ShareMEMBuffer(shareName,writeBlockSize,wrteBlockNum,readBlockSize,readBlockNum,memMaxSize,true,NULL,callback);
	return membuf;
}
shared_ptr<ShareMEMBuffer> ShareMEMBuffer::open(const std::string& shareName,int readBlockSize,int readBlockNum,int writeBlockSize,int writeBlockNum,int memMaxSize,void* startAddr,const ReadMEMCallback& callback)
{
	shared_ptr<ShareMEMBuffer> membuf;
	membuf = new ShareMEMBuffer(shareName, readBlockSize, readBlockNum, writeBlockSize, writeBlockNum, memMaxSize, false, startAddr, callback);
	return membuf;
}

int ShareMEMBuffer::write(void* block,int size)
{
	if(internal == NULL || block == NULL || internal->memPool == NULL)
	{
		return -1;
	}

	if(internal->create)
	{
		return internal->createWriteData(block,size);
	}
	return internal->readWriteData(block,size);
}
IMallcFreeMemObjcPtr* ShareMEMBuffer::getMallckFreeObjcPtr()
{
	if(internal == NULL || internal->memPool == NULL)
	{
		return NULL;
	}

	return internal->memPool.get();
}
char* ShareMEMBuffer::getShareStartMemBuffer()
{
	if(internal == NULL || internal->memPool == NULL)
	{
		return NULL;
	}

	return (char*)internal->sharemem->getbuffer();
}


};
};
