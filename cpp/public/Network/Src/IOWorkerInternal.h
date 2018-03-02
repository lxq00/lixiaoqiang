#ifndef __IOWORKERINTERNAL_H__
#define __IOWORKERINTERNAL_H__
#include "SimpleIO/socket_object.h"

namespace Public{
namespace Network {

struct IOWorker::IOWorkerInternal
{
public:
	Public::Base::shared_ptr<Proactor> proactor;

	Public::Base::shared_ptr<Socket> _buildSocketCallback(int newsock, const NetAddr& otheraaddr)
	{
		return Socket_Object::buildSocketBySock(proactor, newsock, otheraaddr);
	}
};


}
}

#endif //__IOWORKER_H__
