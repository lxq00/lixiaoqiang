#ifndef __ASIOSERVERPOOL_H__
#define __ASIOSERVERPOOL_H__
#include "boost/asio.hpp"
#include "boost/bind.hpp"
#include "boost/enable_shared_from_this.hpp"
#include "boost/shared_ptr.hpp"
#include "boost/make_shared.hpp"
#include "Base/Base.h"
#include "Network/Socket.h"

using namespace Public::Base;

namespace Public{
namespace Network{

class IOWorker::IOWorkerInternal
{
public:
	IOWorkerInternal(uint32_t threadnum);
	~IOWorkerInternal();

	boost::asio::io_service& getIOServer(){return ioserver;}
private:
	void threadRunProc(Thread* t, void* param);
private:
	Mutex												mutex;
	boost::asio::io_service								ioserver;
	boost::shared_ptr<boost::asio::io_service::work>	worker;
	std::list<shared_ptr<Thread> >		threadPool;
	bool												poolQuit;
};

}
}


#endif //__ASIOSERVERPOOL_H__
