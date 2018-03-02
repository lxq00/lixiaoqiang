#include "SocketInternal.h"
#include "Network/Udp.h"
using namespace std;
namespace Public{
namespace Network{

struct UDP::UDPInternal :public SocketInternal
{
	UDPInternal(const Public::Base::shared_ptr<IOWorker>& worker, const Public::Base::shared_ptr<Socket>& ptr) :SocketInternal
	(worker == NULL ? Public::Base::shared_ptr<Proactor>() : worker->internal->proactor, NetType_TcpClient, ptr) {}
};
Public::Base::shared_ptr<Socket> UDP::create(const Public::Base::shared_ptr<IOWorker>& worker)
{
	Public::Base::shared_ptr<UDP> client(new UDP());
	client->internal = new UDPInternal(worker, client);
	client->internal->init();

	return client;
}
UDP::UDP() :internal(NULL)
{
}
UDP::~UDP()
{
	disconnect();
	SAFE_DELETE(internal);
}
bool UDP::disconnect()
{
	return internal->disconnect();
}
bool UDP::nonBlocking(bool nonblock)
{
	return internal->nonBlocking(nonblock);
}
bool UDP::bind(const NetAddr& addr,bool reusedaddr)
{
	return internal->bind(addr, reusedaddr);
}
bool UDP::setSocketTimeout(uint32_t recvTimeout, uint32_t sendTimeout)
{
	return internal->setSocketTimeout(recvTimeout, sendTimeout);
}

bool UDP::getSocketTimeout(uint32_t& recvTimeout, uint32_t& sendTimeout) const
{
	return internal->getSocketTimeout(recvTimeout, sendTimeout);
}
bool UDP::getSocketBuffer(uint32_t& recvSize, uint32_t& sendSize) const
{
	return internal->getSocketBuffer(recvSize, sendSize);
}
bool UDP::setSocketBuffer(uint32_t recvSize, uint32_t sendSize)
{
	return internal->setSocketBuffer(recvSize, sendSize);
}
int UDP::getHandle() const
{
	return internal->getHandle();
}

NetType UDP::getNetType() const
{
	return internal->getNetType();
}
NetAddr UDP::getMyAddr() const
{
	return internal->getMyAddr();
}

bool UDP::async_recvfrom(char *buf , uint32_t len,const RecvFromCallback& received)
{
	return internal->async_recvfrom(buf,len,received);
}

bool UDP::async_sendto(const char * buf, uint32_t len,const NetAddr& other,const SendedCallback& sended)
{
	return internal->async_sendto(buf,len,other,sended);
}

int UDP::recvfrom(char *buf , uint32_t len,NetAddr& other)
{
	return internal->recvfrom(buf,len,other);
}
int UDP::sendto(const char * buf, uint32_t len,const NetAddr& other)
{
	return internal->sendto(buf,len,other);
}
};
};

