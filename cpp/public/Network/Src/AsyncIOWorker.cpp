#include "IOWorkerInternal.h"

namespace Public{
namespace Network {

AsyncIOWorker::ThreadNum::ThreadNum(uint32_t _num) :num(_num) {}
AsyncIOWorker::ThreadNum::ThreadNum(uint32_t cpuCorePerThread, uint32_t minThread, uint32_t maxThread)
{
	num = cpuCorePerThread * Host::getProcessorNum();
	num = num < minThread ? minThread : num;
	num = num > maxThread ? maxThread : num;
}
AsyncIOWorker::ThreadNum::~ThreadNum() {}
uint32_t AsyncIOWorker::ThreadNum::getNum() const { return num; }


AsyncIOWorker::AsyncIOWorker(const ThreadNum& num)
{
	internal = new AsyncIOWorkerInternal;
	internal->manager = new AsyncManager();
	internal->manager->init(num.getNum());
}
AsyncIOWorker::~AsyncIOWorker()
{
	internal->manager = NULL;
	SAFE_DELETE(internal);
}

}
}
