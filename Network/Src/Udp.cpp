#include "ASIOSocketUDP.h"
#include "Network/Udp.h"

using namespace std;
namespace Public{
namespace Network{

struct UDP::UDPInternalPointer:public ASIOSocketUDP
{
	UDPInternalPointer(const shared_ptr<IOWorker>& worker) :ASIOSocketUDP(worker) { create(); }
};
shared_ptr<Socket> UDP::create(const shared_ptr<IOWorker>& worker)
{
	shared_ptr<UDP> sock = shared_ptr<UDP>(new UDP(worker));
	sock->udpinternal->initSocketptr(sock);

	return sock;
}
UDP::UDP(const shared_ptr<IOWorker>& worker)
{
	udpinternal = new UDPInternalPointer(worker);
}
UDP::~UDP()
{
	disconnect();
	SAFE_DELETE(udpinternal);
}
bool UDP::bind(const NetAddr& addr,bool reusedaddr)
{
	return udpinternal->bind(addr,reusedaddr);
}
bool UDP::disconnect()
{
	udpinternal->disconnect();

	return true;
}
bool UDP::getSocketBuffer(uint32_t& recvSize,uint32_t& sendSize) const
{
	return udpinternal->getSocketBuffer(recvSize,sendSize);
}
bool UDP::setSocketBuffer(uint32_t recvSize,uint32_t sendSize)
{
	return udpinternal->setSocketBuffer(recvSize,sendSize);
}
bool UDP::getSocketTimeout(uint32_t& recvTimeout,uint32_t& sendTimeout) const
{
	return udpinternal->getSocketTimeout(recvTimeout,sendTimeout);
}
bool UDP::setSocketTimeout(uint32_t recvTimeout,uint32_t sendTimeout)
{
	return udpinternal->setSocketTimeout(recvTimeout,sendTimeout);
}
bool UDP::nonBlocking(bool nonblock)
{
	return udpinternal->nonBlocking(nonblock);
}
SOCKET UDP::getHandle() const
{
	return udpinternal->getHandle();
}
NetType UDP::getNetType() const
{
	return NetType_Udp;
}
NetAddr UDP::getMyAddr() const
{
	return udpinternal->getMyAddr();
}

bool UDP::async_recvfrom(char *buf , uint32_t len,const RecvFromCallback& received)
{
	if (buf == NULL || len == 0 || !received)
	{
		return false;
	}

	return udpinternal->async_recvfrom(buf, len, received);
}
bool UDP::async_recvfrom(const RecvFromCallback& received, int maxlen)
{
	if (maxlen == 0 || !received)
	{
		return false;
	}
	return udpinternal->async_recvfrom(received,maxlen);
}
bool UDP::async_sendto(const char * buf, uint32_t len,const NetAddr& other,const SendedCallback& sended)
{
	if(buf == NULL || len == 0 || !sended)
	{
		return false;
	}

	return udpinternal->async_sendto(buf,len,other,sended);
}

int UDP::recvfrom(char *buf , uint32_t len,NetAddr& other)
{
	if(buf == NULL || len == 0)
	{
		return false;
	}

	return udpinternal->recvfrom(buf,len,other);
}
int UDP::sendto(const char * buf, uint32_t len,const NetAddr& other)
{
	if(buf == NULL || len == 0)
	{
		return false;
	}

	return udpinternal->sendto(buf,len,other);
}
bool UDP::setSocketOpt(int level, int optname, const void *optval, int optlen)
{
	if (optval == NULL)
	{
		return false;
	}

	return udpinternal->setSocketOpt(level, optname, optval,optlen);
}
bool UDP::getSocketOpt(int level, int optname, void *optval, int *optlen) const
{
	if (optval == NULL)
	{
		return false;
	}

	return udpinternal->getSocketOpt(level, optname, optval, optlen);
}

};
};

