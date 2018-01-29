#ifndef __IOWORKERINTERNAL_H__
#define __IOWORKERINTERNAL_H__
#include "Async/AsyncManager.h"
#include "Network/Socket.h"

namespace Public{
namespace Network {

struct AsyncIOWorker::AsyncIOWorkerInternal
{
	Public::Base::shared_ptr<AsyncManager> manager;
};

}
}

#endif //__IOWORKER_H__
