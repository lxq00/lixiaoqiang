#include "SocketInternal.h"
#include "Network/TcpServer.h"
using namespace std;
namespace Public{
namespace Network{

struct TCPServer::TCPServerInternal :public SocketInternal
{
	TCPServerInternal(const Public::Base::shared_ptr<AsyncIOWorker>& worker, const Public::Base::shared_ptr<Socket>& sockptr)
		:SocketInternal(worker == NULL ? Public::Base::shared_ptr<AsyncManager>() : worker->internal->manager, NetType_TcpClient, sockptr) {}
};

TCPServer::TCPServer()
{
	internal = NULL;
}

Public::Base::shared_ptr<Socket> TCPServer::create(const shared_ptr<AsyncIOWorker>& worker)
{
	Public::Base::shared_ptr<TCPServer> sock(new TCPServer());
	sock->internal = new TCPServerInternal(worker, sock);
	sock->internal->init();

	return sock;
}
TCPServer::~TCPServer()
{
	disconnect();
	SAFE_DELETE(internal);
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

