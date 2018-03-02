#include "SimpleIO/proactor_iocp.h"
#include "SimpleIO/proactor_reactor.h"
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
#ifdef SIMPLEIO_SUPPORT_IOCP
	internal->proactor = new Proactor_IOCP(
		Proactor::UpdateSocketStatusCallback(Socket_Object::updateSocketStatus),
		Proactor::BuildSocketCallback(&IOWorkerInternal::_buildSocketCallback,internal), num.getNum());
#else
	internal->proactor = new Proactor_Reactor(
		Proactor::UpdateSocketStatusCallback(Socket_Object::updateSocketStatus),
		Proactor::BuildSocketCallback(&IOWorkerInternal::_buildSocketCallback, internal), num.getNum());
#endif
}
IOWorker::~IOWorker()
{
	internal->proactor = NULL;
	SAFE_DELETE(internal);
}

}
}
