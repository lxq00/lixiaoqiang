#include "SocketInternal.h"
#include "Network/TcpServer.h"
using namespace std;
namespace Public{
namespace Network{

struct TCPServer::TCPServerInternal :public SocketInternal
{
	TCPServerInternal(const Public::Base::shared_ptr<IOWorker>& worker, const Public::Base::shared_ptr<Socket>& ptr) :SocketInternal
	(worker == NULL ? Public::Base::shared_ptr<Proactor>() : worker->internal->proactor, NetType_TcpClient, ptr) {}
};
Public::Base::shared_ptr<Socket> TCPServer::create(const Public::Base::shared_ptr<IOWorker>& worker)
{
	Public::Base::shared_ptr<TCPServer> client(new TCPServer());
	client->internal = new TCPServerInternal(worker, client);
	client->internal->init();

	return client;
}
TCPServer::TCPServer() :internal(NULL)
{
}
TCPServer::~TCPServer()
{
	disconnect();
	SAFE_DELETE(internal);
}
bool TCPServer::nonBlocking(bool nonblock)
{
	return internal->nonBlocking(nonblock);
}

bool TCPServer::bind(const NetAddr& addr,bool reusedaddr)
{
	bool ret = internal->bind(addr,reusedaddr);
	if (ret)
	{
		internal->listen(100);
	}

	return ret;
}
int TCPServer::getHandle() const
{
	return internal->getHandle();
}

NetType TCPServer::getNetType() const
{
	return internal->getNetType();
}
NetAddr TCPServer::getMyAddr() const
{
	return internal->getMyAddr();
}
bool TCPServer::disconnect()
{
	return internal->disconnect();
}
bool TCPServer::async_accept(const AcceptedCallback& accepted)
{
	return internal->async_accept(accepted);
}

Public::Base::shared_ptr<Socket> TCPServer::accept()
{
	return internal->accept();
}

bool TCPServer::setSocketTimeout(uint32_t recvTimeout, uint32_t sendTimeout)
{
	return internal->setSocketTimeout(recvTimeout, sendTimeout);
}

bool TCPServer::getSocketTimeout(uint32_t& recvTimeout, uint32_t& sendTimeout) const
{
	return internal->getSocketTimeout(recvTimeout, sendTimeout);
}

};
};

