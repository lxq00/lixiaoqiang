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

class IOWorker::IOWorkerInternal
{
public:
	IOWorkerInternal(uint32_t threadnum);
	~IOWorkerInternal();
	uint32_t threadNum() { return (uint32_t)threadPool.size(); }
private:
	void threadRunProc(Thread* t, void* param);
public:
	boost::shared_ptr<boost::asio::io_service>			ioserver;
private:
	boost::shared_ptr<boost::asio::io_service::work>	worker;
	std::list<shared_ptr<Thread> >						threadPool;
	bool												poolQuit;
};

IOWorker::IOWorker(const ThreadNum& num)
{
	internal = new IOWorkerInternal(num.getNum());
}
IOWorker::~IOWorker()
{
	SAFE_DELETE(internal);
}

shared_ptr<IOWorker> IOWorker::defaultWorker()
{
	static weak_ptr<IOWorker> workerptr;
	static Mutex workermutex;

	Guard locker(workermutex);
	shared_ptr<IOWorker> defaultworker = workerptr.lock();
	if (defaultworker == NULL)
	{
		workerptr = defaultworker = make_shared<IOWorker>(ThreadNum(2));
	}

	return defaultworker;
}

void* IOWorker::getBoostASIOIOServerPtr() const
{
	return internal->ioserver.get();
}
uint32_t IOWorker::threadNum()
{
	return internal->threadNum();
}

IOWorker::IOWorkerInternal::IOWorkerInternal(uint32_t num):poolQuit(false)
{
	ioserver = boost::make_shared<boost::asio::io_service>();
	worker = boost::make_shared<boost::asio::io_service::work>(*ioserver);

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
	worker = NULL;
	ioserver = NULL;
	threadPool.clear();
}
void IOWorker::IOWorkerInternal::threadRunProc(Thread* t,void* param)
{
	while(!poolQuit)
	{
		try
		{
			ioserver->run();
		}
		catch(const std::exception& e)
		{
			logerror("%s %d std::exception %s\r\n",__FUNCTION__,__LINE__,e.what());
		}
	}
}

}
}
