#include "ASIOSocketConneter.h"
#include "Network/TcpClient.h"
#include <memory>

using namespace std;
namespace Public{
namespace Network{


shared_ptr<Socket> TCPClient::create(const shared_ptr<IOWorker>& _worker)
{
	shared_ptr<TCPClient> sock = shared_ptr<TCPClient>(new TCPClient(_worker));
	sock->tcpclientinternal->sock->create();
	sock->tcpclientinternal->sock->initSocketptr(sock);

	return sock;
}
TCPClient::TCPClient(const shared_ptr<IOWorker>& worker)
{
	tcpclientinternal = new TCPClient::TCPClientInternalPointer;
	tcpclientinternal->sock = boost::make_shared<ASIOSocketConneter>(worker);
}
TCPClient::~TCPClient()
{
	disconnect();
	SAFE_DELETE(tcpclientinternal);
	//shareptr ×Ô¶¯É¾³ý
}
bool TCPClient::bind(const NetAddr& addr,bool reusedaddr)
{
	boost::shared_ptr<ASIOSocketConneter> sockobj;

	{
		sockobj = tcpclientinternal->sock;
	}
	if(sockobj == NULL)
	{
		return false;
	}

	return sockobj->bind(addr,reusedaddr);
}
bool TCPClient::disconnect()
{
	boost::shared_ptr<ASIOSocketConneter> sockobj;

	{
		sockobj = tcpclientinternal->sock;
	}
	if (sockobj == NULL)
	{
		return false;
	}
	return sockobj->disconnect();
}
bool TCPClient::getSocketBuffer(uint32_t& recvSize,uint32_t& sendSize) const
{
	boost::shared_ptr<ASIOSocketConneter> sockobj;

	{
		sockobj = tcpclientinternal->sock;
	}
	if (sockobj == NULL)
	{
		return false;
	}
	return sockobj->getSocketBuffer(recvSize,sendSize);
}
bool TCPClient::setSocketBuffer(uint32_t recvSize,uint32_t sendSize)
{
	boost::shared_ptr<ASIOSocketConneter> sockobj;

	{
		sockobj = tcpclientinternal->sock;
	}
	if (sockobj == NULL)
	{
		return false;
	}
	return sockobj->setSocketBuffer(recvSize,sendSize);
}
bool TCPClient::getSocketTimeout(uint32_t& recvTimeout,uint32_t& sendTimeout) const
{
	boost::shared_ptr<ASIOSocketConneter> sockobj;

	{
		sockobj = tcpclientinternal->sock;
	}
	if (sockobj == NULL)
	{
		return false;
	}
	return sockobj->getSocketTimeout(recvTimeout,sendTimeout);
}
bool TCPClient::setSocketTimeout(uint32_t recvTimeout,uint32_t sendTimeout)
{
	boost::shared_ptr<ASIOSocketConneter> sockobj;

	{
		sockobj = tcpclientinternal->sock;
	}
	if (sockobj == NULL)
	{
		return false;
	}
	return sockobj->setSocketTimeout(recvTimeout,sendTimeout);
}
bool TCPClient::nonBlocking(bool nonblock)
{
	boost::shared_ptr<ASIOSocketConneter> sockobj;

	{
		sockobj = tcpclientinternal->sock;
	}
	if (sockobj == NULL)
	{
		return false;
	}
	return sockobj->nonBlocking(nonblock);
}
SOCKET TCPClient::getHandle() const
{
	boost::shared_ptr<ASIOSocketConneter> sockobj;

	{
		sockobj = tcpclientinternal->sock;
	}
	if (sockobj == NULL)
	{
		return false;
	}
	return sockobj->getHandle();
}
NetStatus TCPClient::getStatus() const
{
	boost::shared_ptr<ASIOSocketConneter> sockobj;

	{
		sockobj = tcpclientinternal->sock;
	}
	if (sockobj == NULL)
	{
		return NetStatus_notconnected;
	}
	return sockobj->getStatus();
}
NetType TCPClient::getNetType() const
{
	return NetType_TcpClient;
}
NetAddr TCPClient::getMyAddr() const
{
	boost::shared_ptr<ASIOSocketConneter> sockobj;

	{
		sockobj = tcpclientinternal->sock;
	}
	if (sockobj == NULL)
	{
		return NetAddr();
	}
	return sockobj->getMyAddr();
}
NetAddr TCPClient::getOtherAddr() const
{
	boost::shared_ptr<ASIOSocketConneter> sockobj;

	{
		sockobj = tcpclientinternal->sock;
	}
	if (sockobj == NULL)
	{
		return NetAddr();
	}
	return sockobj->getOtherAddr();
}
bool TCPClient::async_connect(const NetAddr& addr,const ConnectedCallback& connected)
{
	boost::shared_ptr<ASIOSocketConneter> sockobj;

	{
		sockobj = tcpclientinternal->sock;
	}
	if(sockobj == NULL || !connected)
	{
		return false;
	}

	return sockobj->startConnect(connected,addr);
}
bool TCPClient::connect(const NetAddr& addr)
{
	boost::shared_ptr<ASIOSocketConneter> sockobj;

	{
		sockobj = tcpclientinternal->sock;
	}
	if(sockobj == NULL)
	{
		return false;
	}

	return sockobj->connect(addr);
}
bool TCPClient::setDisconnectCallback(const Socket::DisconnectedCallback& disconnected)
{
	boost::shared_ptr<ASIOSocketConneter> sockobj;

	{
		sockobj = tcpclientinternal->sock;
	}
	if (sockobj == NULL)
	{
		return false;
	}
	return sockobj->setDisconnectCallback(disconnected);
}
bool TCPClient::async_recv(char *buf , uint32_t len,const Socket::ReceivedCallback& received)
{
	boost::shared_ptr<ASIOSocketConneter> sockobj;

	{
		sockobj = tcpclientinternal->sock;
	}
	if (sockobj == NULL)
	{
		return false;
	}
	return sockobj->async_recv(buf,len,received);
}
bool TCPClient::async_recv(const ReceivedCallback& received, int maxlen)
{
	boost::shared_ptr<ASIOSocketConneter> sockobj;

	{
		sockobj = tcpclientinternal->sock;
	}
	if (sockobj == NULL)
	{
		return false;
	}
	return sockobj->async_recv(received,maxlen);
}
bool TCPClient::async_send(const char * buf, uint32_t len,const Socket::SendedCallback& sended)
{
	boost::shared_ptr<ASIOSocketConneter> sockobj;

	{
		sockobj = tcpclientinternal->sock;
	}
	if (sockobj == NULL)
	{
		return false;
	}
	return sockobj->async_send(buf,len,sended);
}
int TCPClient::recv(char *buf , uint32_t len)
{
	boost::shared_ptr<ASIOSocketConneter> sockobj;

	{
		sockobj = tcpclientinternal->sock;
	}
	if (sockobj == NULL)
	{
		return false;
	}
	return sockobj->recv(buf,len);
}
int TCPClient::send(const char * buf, uint32_t len)
{
	boost::shared_ptr<ASIOSocketConneter> sockobj;

	{
		sockobj = tcpclientinternal->sock;
	}
	if (sockobj == NULL)
	{
		return false;
	}
	return sockobj->send(buf,len);
}
bool TCPClient::setSocketOpt(int level, int optname, const void *optval, int optlen)
{
	boost::shared_ptr<ASIOSocketConneter> sockobj;

	{
		sockobj = tcpclientinternal->sock;
	}
	if (sockobj == NULL)
	{
		return false;
	}

	return sockobj->setSocketOpt(level, optname, optval, optlen);
}
bool TCPClient::getSocketOpt(int level, int optname, void *optval, int *optlen) const
{
	boost::shared_ptr<ASIOSocketConneter> sockobj;

	{
		sockobj = tcpclientinternal->sock;
	}
	if (sockobj == NULL)
	{
		return false;
	}

	return sockobj->getSocketOpt(level, optname, optval, optlen);
}
};
};



