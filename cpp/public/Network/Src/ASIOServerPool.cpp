#include "ASIOServerPool.h"
#include "Base/Base.h"
#ifndef WIN32 
#include <unistd.h>
#endif

#define BOOST_SYSTEM_SOURCE

#include <boost/system/error_code.hpp>

#ifndef BOOST_ERROR_CODE_HEADER_ONLY
#include <boost/system/detail/error_code.ipp>
#endif


using namespace Public::Base;

namespace Public{
namespace Network{

IOWorker::ThreadNum::ThreadNum(uint32_t _num):num(_num){}
IOWorker::ThreadNum::ThreadNum(uint32_t cpuCorePerThread,uint32_t minThread,uint32_t maxThread)
{
	num = cpuCorePerThread * Host::getProcessorNum();
	num = num < minThread ? minThread : num;
	num = num > maxThread ? maxThread : num;
}
IOWorker::ThreadNum::~ThreadNum(){}
uint32_t IOWorker::ThreadNum::getNum() const {return num;}


IOWorker::IOWorker(const ThreadNum& num)
{
	internal = new IOWorkerInternal(num.getNum());
}
IOWorker::~IOWorker()
{
	SAFE_DELETE(internal);
}

IOWorker::IOWorkerInternal::IOWorkerInternal(uint32_t num):poolQuit(false)
{
	worker = boost::shared_ptr<boost::asio::io_service::work>(new boost::asio::io_service::work(ioserver));

	if(num == 0)
	{
		num = 1;
	}

	for(uint32_t i = 0;i < num;i ++)
	{
		shared_ptr<Thread> thread = ThreadEx::creatThreadEx("IOServerRun",ThreadEx::Proc(&IOWorker::IOWorkerInternal::threadRunProc,this),NULL,Thread::priorTop,Thread::policyRealtime);
		thread->createThread();
		threadPool.push_back(thread);
	}
}

IOWorker::IOWorkerInternal::~IOWorkerInternal()
{
	poolQuit = true;
	worker = boost::shared_ptr<boost::asio::io_service::work>();
	threadPool.clear();
}
void IOWorker::IOWorkerInternal::threadRunProc(Thread* t,void* param)
{
	while(!poolQuit)
	{
		try
		{
			ioserver.run();
		}
		catch(const std::exception& e)
		{
			logerror("%s %d std::exception %s\r\n",__FUNCTION__,__LINE__,e.what());
		}
	}
}

}
}
