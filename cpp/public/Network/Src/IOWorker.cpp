#include "IOWorkerInternal.h"

namespace Public{
namespace Network {

IOWorker::ThreadNum::ThreadNum(uint32_t _num) :num(_num) {}
IOWorker::ThreadNum::ThreadNum(uint32_t cpuCorePerThread, uint32_t minThread, uint32_t maxThread)
{
	num = cpuCorePerThread * Host::getProcessorNum();
	num = num < minThread ? minThread : num;
	num = num > maxThread ? maxThread : num;
}
IOWorker::ThreadNum::~ThreadNum() {}
uint32_t IOWorker::ThreadNum::getNum() const { return num; }


IOWorker::IOWorker(const ThreadNum& num)
{
	internal = new IOWorkerInternal;
	internal->manager = new AsyncManager();
	internal->manager->init(num.getNum());
}
IOWorker::~IOWorker()
{
	internal->manager = NULL;
	SAFE_DELETE(internal);
}

}
}
