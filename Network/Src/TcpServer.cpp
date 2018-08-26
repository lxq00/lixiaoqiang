#include "ASIOSocketAcceptor.h"
#include "Network/TcpServer.h"
using namespace std;
namespace Public{
namespace Network{

struct TCPServer::TCPServerInternalPointer
{
	boost::shared_ptr<ASIOSocketAcceptor> sock;
};
shared_ptr<Socket> TCPServer::create(const shared_ptr<IOWorker>& _worker,const NetAddr& addr)
{
	shared_ptr<TCPServer> sock = shared_ptr<TCPServer>(new TCPServer(_worker,addr));
	sock->tcpserverinternal->sock->initSocketptr(sock);

	return sock;
}
TCPServer::TCPServer(const shared_ptr<IOWorker>& _worker, const NetAddr& addr)
{
	tcpserverinternal = new TCPServerInternalPointer;
	tcpserverinternal->sock = boost::make_shared<ASIOSocketAcceptor>(_worker);
	tcpserverinternal->sock->create(addr,true);
}
TCPServer::~TCPServer()
{
	disconnect();
	SAFE_DELETE(tcpserverinternal);
}
int TCPServer::getHandle() const
{
	boost::shared_ptr<ASIOSocketAcceptor> sockobj;
	{
		sockobj = tcpserverinternal->sock;
	}
	if (sockobj == NULL)
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
	boost::shared_ptr<ASIOSocketAcceptor> sockobj;
	{
		sockobj = tcpserverinternal->sock;
	}
	if (sockobj == NULL)
	{
		return false;
	}

	return sockobj->getMyAddr();
}
bool TCPServer::disconnect()
{
	boost::shared_ptr<ASIOSocketAcceptor> sockobj;
	{
		sockobj = tcpserverinternal->sock;
	}
	if (sockobj != NULL)
	{
		sockobj->disconnect();
	}

	return true;
}
bool TCPServer::async_accept(const AcceptedCallback& accepted)
{
	boost::shared_ptr<ASIOSocketAcceptor> sockobj;
	{
		sockobj = tcpserverinternal->sock;
	}
	if(sockobj == NULL || !accepted)
	{
		return false;
	}

	return sockobj->startListen(accepted);
}

shared_ptr<Socket> TCPServer::accept()
{
	boost::shared_ptr<ASIOSocketAcceptor> sockobj;
	{
		sockobj = tcpserverinternal->sock;
	}
	if(sockobj == NULL)
	{
		return NULL;
	}

	return sockobj->accept();
}

bool TCPServer::getSocketTimeout(uint32_t& recvTimeout,uint32_t& sendTimeout) const
{
	boost::shared_ptr<ASIOSocketAcceptor> sockobj;
	{
		sockobj = tcpserverinternal->sock;
	}
	if(sockobj == NULL)
	{
		return false;
	}

	return sockobj->getSocketTimeout(recvTimeout,sendTimeout);
}
bool TCPServer::setSocketTimeout(uint32_t recvTimeout,uint32_t sendTimeout)
{
	boost::shared_ptr<ASIOSocketAcceptor> sockobj;
	{
		sockobj = tcpserverinternal->sock;
	}
	if(sockobj == NULL)
	{
		return false;
	}

	return sockobj->setSocketTimeout(recvTimeout,sendTimeout);
}
bool TCPServer::nonBlocking(bool nonblock)
{
	boost::shared_ptr<ASIOSocketAcceptor> sockobj;
	{
		sockobj = tcpserverinternal->sock;
	}
	if(sockobj == NULL)
	{
		return false;
	}

	return sockobj->nonBlocking(nonblock);
}
bool TCPServer::setSocketOpt(int level, int optname, const void *optval, int optlen)
{
	boost::shared_ptr<ASIOSocketAcceptor> sockobj;
	{
		sockobj = tcpserverinternal->sock;
	}
	if (sockobj == NULL)
	{
		return false;
	}

	return sockobj->setSocketOpt(level, optname, optval, optlen);
}
bool TCPServer::getSocketOpt(int level, int optname, void *optval, int *optlen) const
{
	boost::shared_ptr<ASIOSocketAcceptor> sockobj;
	{
		sockobj = tcpserverinternal->sock;
	}
	if (sockobj == NULL)
	{
		return false;
	}

	return sockobj->getSocketOpt(level, optname, optval, optlen);
}

};
};


