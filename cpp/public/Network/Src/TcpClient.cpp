#include "SocketInternal.h"
#include "Network/TcpClient.h"
#include <memory>

using namespace std;
namespace Public{
namespace Network{

struct TCPClient::TCPClientInternal :public SocketInternal
{
	TCPClientInternal(const Public::Base::shared_ptr<IOWorker>& worker,const Public::Base::shared_ptr<Socket>& ptr) :SocketInternal
	(worker == NULL ? Public::Base::shared_ptr<Proactor> (): worker->internal->proactor, NetType_TcpClient,ptr) {}
};
Public::Base::shared_ptr<Socket> TCPClient::create(const Public::Base::shared_ptr<IOWorker>& worker)
{
	Public::Base::shared_ptr<TCPClient> client(new TCPClient());
	client->internal = new TCPClientInternal(worker, client);
	client->internal->init();

	return client;
}
TCPClient::TCPClient():internal(NULL)
{
}
TCPClient::~TCPClient()
{
	disconnect();
	SAFE_DELETE(internal);
}
bool TCPClient::bind(const NetAddr& addr,bool reusedaddr)
{
	return internal->bind(addr,reusedaddr);
}
bool TCPClient::disconnect()
{
	return internal->disconnect();
}
bool TCPClient::setSocketTimeout(uint32_t recvTimeout, uint32_t sendTimeout)
{
	return internal->setSocketTimeout(recvTimeout, sendTimeout);
}

bool TCPClient::getSocketTimeout(uint32_t& recvTimeout, uint32_t& sendTimeout) const
{
	return internal->getSocketTimeout(recvTimeout, sendTimeout);
}
bool TCPClient::getSocketBuffer(uint32_t& recvSize,uint32_t& sendSize) const
{
	return internal->getSocketBuffer(recvSize,sendSize);
}
bool TCPClient::setSocketBuffer(uint32_t recvSize,uint32_t sendSize)
{
	return internal->setSocketBuffer(recvSize,sendSize);
}
int TCPClient::getHandle() const
{
	return internal->getHandle();
}

NetType TCPClient::getNetType() const
{
	return internal->getNetType();
}
NetAddr TCPClient::getMyAddr() const
{
	return internal->getMyAddr();
}
NetAddr TCPClient::getOhterAddr() const
{
	return internal->getOhterAddr();
}
bool TCPClient::async_connect(const NetAddr& addr,const ConnectedCallback& connected)
{
	return internal->async_connect(addr,connected);
}
bool TCPClient::connect(const NetAddr& addr)
{
	return internal->connect(addr);
}
bool TCPClient::setDisconnectCallback(const Socket::DisconnectedCallback& disconnected)
{
	return internal->setDisconnectCallback(disconnected);
}
bool TCPClient::async_recv(char *buf , uint32_t len,const Socket::ReceivedCallback& received)
{
	return internal->async_recv(buf,len,received);
}
bool TCPClient::async_send(const char * buf, uint32_t len,const Socket::SendedCallback& sended)
{
	return internal->async_send(buf,len,sended);
}
int TCPClient::recv(char *buf , uint32_t len)
{
	return internal->recv(buf,len);
}
int TCPClient::send(const char * buf, uint32_t len)
{
	return internal->send(buf,len);
}
bool TCPClient::nonBlocking(bool nonblock)
{
	return internal->nonBlocking(nonblock);
}
NetStatus TCPClient::getStatus() const
{
	return internal->getStatus();
}
};
};



