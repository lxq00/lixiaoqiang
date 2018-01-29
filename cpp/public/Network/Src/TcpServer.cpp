#include "SocketInternal.h"
#include "Network/TcpServer.h"
using namespace std;
namespace Public{
namespace Network{

struct TCPServer::TCPServerInternalPointer
{
	shared_ptr<Socket> sock;
};

TCPServer::TCPServer()
{
	tcpserverinternal = new TCPServerInternalPointer;
}

Public::Base::shared_ptr<Socket> TCPServer::create(const shared_ptr<AsyncIOWorker>& worker)
{
	if (worker == NULL) return Public::Base::shared_ptr<Socket>();

	Public::Base::shared_ptr<TCPServer> sock(new TCPServer());
	sock->tcpserverinternal->sock = new SocketInternal(
		worker == NULL ? Public::Base::shared_ptr<AsyncManager>() :
		worker->internal->manager, sock, NetType_TcpServer);

	return sock;
}
TCPServer::~TCPServer()
{
	disconnect();
	SAFE_DELETE(tcpserverinternal);
}
bool TCPServer::bind(const NetAddr& addr,bool reusedaddr)
{
	Public::Base::shared_ptr<SocketInternal> sockobj = tcpserverinternal->sock;
	if (sockobj == NULL)
	{
		return false;
	}
	
	return sockobj->bind(addr,reusedaddr);
}
int TCPServer::getHandle() const
{
	Public::Base::shared_ptr<SocketInternal> sockobj = tcpserverinternal->sock;
	if (sockobj == NULL)
	{
		return false;
	}

	return sockobj->getHandle();
}

NetType TCPServer::getNetType() const
{
	return NetType_TcpServer;
}
NetAddr TCPServer::getMyAddr() const
{
	Public::Base::shared_ptr<SocketInternal> sockobj = tcpserverinternal->sock;
	if (sockobj == NULL)
	{
		return NetAddr();
	}

	return sockobj->getMyAddr();
}
bool TCPServer::disconnect()
{
	if (tcpserverinternal->sock != NULL)
	{
		tcpserverinternal->sock->disconnect();
		tcpserverinternal->sock = NULL;
	}

	return true;
}
bool TCPServer::async_accept(const AcceptedCallback& accepted)
{
	Public::Base::shared_ptr<SocketInternal> sockobj = tcpserverinternal->sock;
	if (sockobj == NULL)
	{
		return false;
	}

	return sockobj->async_accept(accepted);
}

Public::Base::shared_ptr<Socket> TCPServer::accept()
{
	Public::Base::shared_ptr<SocketInternal> sockobj = tcpserverinternal->sock;
	if (sockobj == NULL)
	{
		return Public::Base::shared_ptr<Socket>();
	}

	return sockobj->accept();
}

bool TCPServer::setSocketTimeout(uint32_t recvTimeout, uint32_t sendTimeout)
{
	Public::Base::shared_ptr<SocketInternal> sockobj = tcpserverinternal->sock;
	if (sockobj == NULL)
	{
		return false;
	}

	return sockobj->setSocketTimeout(recvTimeout, sendTimeout);
}

bool TCPServer::getSocketTimeout(uint32_t& recvTimeout, uint32_t& sendTimeout) const
{
	Public::Base::shared_ptr<SocketInternal> sockobj = tcpserverinternal->sock;
	if (sockobj == NULL)
	{
		return false;
	}

	return sockobj->getSocketTimeout(recvTimeout, sendTimeout);
}

};
};


