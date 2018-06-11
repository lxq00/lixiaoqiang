#include "ASIOSocketAcceptObject.h"
#include "Network/TcpServer.h"
using namespace std;
namespace Public{
namespace Network{

struct TCPServer::TCPServerInternalPointer
{
	TCPServerInternalPointer(){}
	~TCPServerInternalPointer(){}
	Mutex									mutex;
	boost::shared_ptr<TCPServerSocketObject> internal;
};

TCPServer::TCPServer(const IOWorker& worker,const NetAddr& addr)
{
	tcpserverinternal = new TCPServerInternalPointer;
	tcpserverinternal->internal = boost::make_shared<TCPServerSocketObject>(worker.internal,this);
	
	bind(addr);
}

TCPServer::~TCPServer()
{
	disconnect();
	SAFE_DELETE(tcpserverinternal);
}
bool TCPServer::bind(const NetAddr& addr,bool reusedaddr)
{
	if(!addr.isValid())
	{
		return false;
	}
	boost::shared_ptr<TCPServerSocketObject> sockobj;

	{
		Guard locker(tcpserverinternal->mutex);
		sockobj = tcpserverinternal->internal;
	}
	if(sockobj == NULL)
	{
		return false;
	}
	return sockobj->create(addr,reusedaddr);
}
int TCPServer::getHandle() const
{
	boost::shared_ptr<TCPServerSocketObject> sockobj;

	{
		Guard locker(tcpserverinternal->mutex);
		sockobj = tcpserverinternal->internal;
	}
	if(sockobj == NULL)
	{
		return false;
	}

	return sockobj->getHandle();
}
NetStatus TCPServer::getStatus() const
{
	return NetStatus_notconnected;
}
NetType TCPServer::getNetType() const
{
	return NetType_TcpServer;
}
NetAddr TCPServer::getMyAddr() const
{
	boost::shared_ptr<TCPServerSocketObject> sockobj;

	{
		Guard locker(tcpserverinternal->mutex);
		sockobj = tcpserverinternal->internal;
	}
	if(sockobj == NULL)
	{
		return NetAddr();
	}

	return sockobj->getMyAddr();
}
bool TCPServer::disconnect()
{
	boost::shared_ptr<TCPServerSocketObject> sockobj;

	{
		Guard locker(tcpserverinternal->mutex);
		sockobj = tcpserverinternal->internal;
		tcpserverinternal->internal = boost::shared_ptr<TCPServerSocketObject>();
	}
	if(sockobj != NULL)
	{
		sockobj->destory();
	}

	return true;
}
bool TCPServer::async_accept(const AcceptedCallback& accepted)
{
	boost::shared_ptr<TCPServerSocketObject> sockobj;

	{
		Guard locker(tcpserverinternal->mutex);
		sockobj = tcpserverinternal->internal;
	}
	if(sockobj == NULL || !accepted)
	{
		return false;
	}

	return sockobj->startListen(accepted);
}

Socket* TCPServer::accept()
{
	boost::shared_ptr<TCPServerSocketObject> sockobj;

	{
		Guard locker(tcpserverinternal->mutex);
		sockobj = tcpserverinternal->internal;
	}
	if(sockobj == NULL)
	{
		return NULL;
	}

	return sockobj->accept();
}

bool TCPServer::getSocketTimeout(uint32_t& recvTimeout,uint32_t& sendTimeout) const
{
	boost::shared_ptr<TCPServerSocketObject> sockobj;

	{
		//Guard locker(tcpserverinternal->mutex);
		sockobj = tcpserverinternal->internal;
	}
	if(sockobj == NULL)
	{
		return false;
	}

	return sockobj->getSocketTimeout(recvTimeout,sendTimeout);
}
bool TCPServer::setSocketTimeout(uint32_t recvTimeout,uint32_t sendTimeout)
{
	boost::shared_ptr<TCPServerSocketObject> sockobj;

	{
		Guard locker(tcpserverinternal->mutex);
		sockobj = tcpserverinternal->internal;
	}
	if(sockobj == NULL)
	{
		return false;
	}

	return sockobj->setSocketTimeout(recvTimeout,sendTimeout);
}
bool TCPServer::nonBlocking(bool nonblock)
{
	boost::shared_ptr<TCPServerSocketObject> sockobj;

	{
		Guard locker(tcpserverinternal->mutex);
		sockobj = tcpserverinternal->internal;
	}
	if(sockobj == NULL)
	{
		return false;
	}

	return sockobj->nonBlocking(nonblock);
}
bool TCPServer::setSocketOpt(int level, int optname, const void *optval, int optlen)
{
	boost::shared_ptr<TCPServerSocketObject> sockobj;

	{
		Guard locker(tcpserverinternal->mutex);
		sockobj = tcpserverinternal->internal;
	}
	if (sockobj == NULL)
	{
		return false;
	}

	return sockobj->setSocketOpt(level, optname, optval, optlen);
}
bool TCPServer::getSocketOpt(int level, int optname, void *optval, int *optlen) const
{
	boost::shared_ptr<TCPServerSocketObject> sockobj;
	{
		//Guard locker(mutex);
		sockobj = tcpserverinternal->internal;
	}
	if (sockobj == NULL)
	{
		return false;
	}

	return sockobj->getSocketOpt(level, optname, optval, optlen);
}

};
};


