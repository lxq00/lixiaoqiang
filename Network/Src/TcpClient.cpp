#include "ASIOSocketConneter.h"
#include "Network/TcpClient.h"
#include <memory>

using namespace std;
namespace Public{
namespace Network{


shared_ptr<Socket> TCPClient::create(const shared_ptr<IOWorker>& _worker)
{
	shared_ptr<TCPClient> sock = shared_ptr<TCPClient>(new TCPClient(_worker));
	sock->tcpclientinternal->initSocketptr(sock);

	return sock;
}
TCPClient::TCPClient(const shared_ptr<IOWorker>& worker)
{
	tcpclientinternal = new TCPClient::TCPClientInternalPointer(worker);
}
TCPClient::~TCPClient()
{
	disconnect();
	SAFE_DELETE(tcpclientinternal);
	//shareptr ×Ô¶¯É¾³ý
}
bool TCPClient::bind(const NetAddr& addr,bool reusedaddr)
{
	return tcpclientinternal->bind(addr,reusedaddr);
}
bool TCPClient::disconnect()
{
	return tcpclientinternal->disconnect();
}
bool TCPClient::getSocketBuffer(uint32_t& recvSize,uint32_t& sendSize) const
{
	return tcpclientinternal->getSocketBuffer(recvSize,sendSize);
}
bool TCPClient::setSocketBuffer(uint32_t recvSize,uint32_t sendSize)
{
	return tcpclientinternal->setSocketBuffer(recvSize,sendSize);
}
bool TCPClient::getSocketTimeout(uint32_t& recvTimeout,uint32_t& sendTimeout) const
{
	return tcpclientinternal->getSocketTimeout(recvTimeout,sendTimeout);
}
bool TCPClient::setSocketTimeout(uint32_t recvTimeout,uint32_t sendTimeout)
{
	return tcpclientinternal->setSocketTimeout(recvTimeout,sendTimeout);
}
bool TCPClient::nonBlocking(bool nonblock)
{
	return tcpclientinternal->nonBlocking(nonblock);
}
SOCKET TCPClient::getHandle() const
{
	return tcpclientinternal->getHandle();
}
NetStatus TCPClient::getStatus() const
{
	return tcpclientinternal->getStatus();
}
NetType TCPClient::getNetType() const
{
	return NetType_TcpClient;
}
NetAddr TCPClient::getMyAddr() const
{
	return tcpclientinternal->getMyAddr();
}
NetAddr TCPClient::getOtherAddr() const
{
	return tcpclientinternal->getOtherAddr();
}
bool TCPClient::async_connect(const NetAddr& addr,const ConnectedCallback& connected)
{
	if(!connected)
	{
		return false;
	}

	return tcpclientinternal->startConnect(connected,addr);
}
bool TCPClient::connect(const NetAddr& addr)
{
	return tcpclientinternal->connect(addr);
}
bool TCPClient::setDisconnectCallback(const Socket::DisconnectedCallback& disconnected)
{
	if (!disconnected)
	{
		return false;
	}
	return tcpclientinternal->setDisconnectCallback(disconnected);
}
bool TCPClient::async_recv(char *buf , uint32_t len,const Socket::ReceivedCallback& received)
{
	if (buf == NULL || len <= 0 || !received)
	{
		return false;
	}
	return tcpclientinternal->async_recv(buf,len,received);
}
bool TCPClient::async_recv(const ReceivedCallback& received, int maxlen)
{
	if (maxlen <= 0)
	{
		return false;
	}
	return tcpclientinternal->async_recv(received,maxlen);
}
bool TCPClient::async_send(const char * buf, uint32_t len,const Socket::SendedCallback& sended)
{
	if (buf == NULL || len <= 0 || !sended)
	{
		return false;
	}
	return tcpclientinternal->async_send(buf,len,sended);
}
int TCPClient::recv(char *buf , uint32_t len)
{
	if (buf == NULL || len <= 0)
	{
		return false;
	}
	return tcpclientinternal->recv(buf,len);
}
int TCPClient::send(const char * buf, uint32_t len)
{
	if (buf == NULL || len <= 0)
	{
		return false;
	}
	return tcpclientinternal->send(buf,len);
}
bool TCPClient::setSocketOpt(int level, int optname, const void *optval, int optlen)
{
	if (optval == NULL)
	{
		return false;
	}

	return tcpclientinternal->setSocketOpt(level, optname, optval, optlen);
}
bool TCPClient::getSocketOpt(int level, int optname, void *optval, int *optlen) const
{
	if (optval == NULL)
	{
		return false;
	}

	return tcpclientinternal->getSocketOpt(level, optname, optval, optlen);
}
void TCPClient::socketReady()
{
	tcpclientinternal->setStatus(NetStatus_connected);
	tcpclientinternal->nonBlocking(true);
	tcpclientinternal->nodelay();
}

void TCPClient::socketError(const std::string &errmsg)
{
	tcpclientinternal->setStatus(NetStatus_disconnected,errmsg);
}

};
};



