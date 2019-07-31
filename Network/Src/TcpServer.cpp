#include "ASIOSocketAcceptor.h"
#include "Network/TcpServer.h"
using namespace std;
namespace Public{
namespace Network{

struct TCPServer::TCPServerInternalPointer:public ASIOSocketAcceptor
{
	TCPServerInternalPointer(const shared_ptr<IOWorker>& _worker, const NetAddr& addr) :ASIOSocketAcceptor(_worker) { create(addr,true); }
};
shared_ptr<Socket> TCPServer::create(const shared_ptr<IOWorker>& _worker,const NetAddr& addr)
{
	shared_ptr<TCPServer> sock = shared_ptr<TCPServer>(new TCPServer(_worker,addr));
   sock->tcpserverinternal->initSocketptr(sock);

	return sock;
}
TCPServer::TCPServer(const shared_ptr<IOWorker>& _worker, const NetAddr& addr)
{
	tcpserverinternal = new TCPServerInternalPointer(_worker,addr);
}
TCPServer::~TCPServer()
{
	disconnect();
	SAFE_DELETE(tcpserverinternal);
}
SOCKET TCPServer::getHandle() const
{
	return tcpserverinternal->getHandle();
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
	return tcpserverinternal->getMyAddr();
}
bool TCPServer::disconnect()
{
	return tcpserverinternal->disconnect();
}
bool TCPServer::async_accept(const AcceptedCallback& accepted)
{
	if(!accepted)
	{
		return false;
	}

	return tcpserverinternal->async_accept(accepted);
}

shared_ptr<Socket> TCPServer::accept()
{
	return tcpserverinternal->accept();
}

bool TCPServer::getSocketTimeout(uint32_t& recvTimeout,uint32_t& sendTimeout) const
{
	return tcpserverinternal->getSocketTimeout(recvTimeout,sendTimeout);
}
bool TCPServer::setSocketTimeout(uint32_t recvTimeout,uint32_t sendTimeout)
{
	return tcpserverinternal->setSocketTimeout(recvTimeout,sendTimeout);
}
bool TCPServer::nonBlocking(bool nonblock)
{
	return tcpserverinternal->nonBlocking(nonblock);
}
bool TCPServer::setSocketOpt(int level, int optname, const void *optval, int optlen)
{
	if (optval == NULL)
	{
		return false;
	}

	return tcpserverinternal->setSocketOpt(level, optname, optval, optlen);
}
bool TCPServer::getSocketOpt(int level, int optname, void *optval, int *optlen) const
{
	if (optval == NULL)
	{
		return false;
	}

	return tcpserverinternal->getSocketOpt(level, optname, optval, optlen);
}

};
};


