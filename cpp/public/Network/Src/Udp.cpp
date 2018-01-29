#include "SocketInternal.h"
#include "Network/Udp.h"

using namespace std;
namespace Public{
namespace Network{

struct UDP::UDPInternalPointer
{
	shared_ptr<SocketInternal> sock;
};
UDP::UDP()
{
	udpinternal = new UDPInternalPointer;
}
Public::Base::shared_ptr<Socket> UDP::create(const  Public::Base::shared_ptr<IOWorker>& worker)
{
	if (worker == NULL) return Public::Base::shared_ptr<Socket>();

	Public::Base::shared_ptr<UDP> udp(new UDP());

	udp->udpinternal->sock = new SocketInternal(worker->internal->manager, udp, NetType_Udp);
	return udp;
}
UDP::~UDP()
{
	if (udpinternal->sock != NULL)
	{
		udpinternal->sock->disconnect();
		udpinternal->sock = NULL;
	}
	SAFE_DELETE(udpinternal);
}
bool UDP::bind(const NetAddr& addr,bool reusedaddr)
{
	Public::Base::shared_ptr<SocketInternal> sockobj = udpinternal->sock;
	if (sockobj == NULL)
	{
		return false;
	}

	return sockobj->bind(addr, reusedaddr);
}

bool UDP::getSocketBuffer(uint32_t& recvSize, uint32_t& sendSize) const
{
	Public::Base::shared_ptr<SocketInternal> sockobj = udpinternal->sock;
	if (sockobj == NULL)
	{
		return false;
	}

	return sockobj->getSocketBuffer(recvSize, sendSize);
}
bool UDP::setSocketBuffer(uint32_t recvSize, uint32_t sendSize)
{
	Public::Base::shared_ptr<SocketInternal> sockobj = udpinternal->sock;
	if (sockobj == NULL)
	{
		return false;
	}

	return sockobj->setSocketBuffer(recvSize, sendSize);
}
int UDP::getHandle() const
{
	Public::Base::shared_ptr<SocketInternal> sockobj = udpinternal->sock;
	if (sockobj == NULL)
	{
		return false;
	}

	return sockobj->getHandle();
}

NetType TCPClient::getNetType() const
{
	return NetType_Udp;
}
NetAddr TCPClient::getMyAddr() const
{
	Public::Base::shared_ptr<SocketInternal> sockobj = udpinternal->sock;
	if (sockobj == NULL)
	{
		return NetAddr();
	}
	return sockobj->getMyAddr();
}

bool UDP::async_recvfrom(char *buf , uint32_t len,const RecvFromCallback& received)
{
	Public::Base::shared_ptr<SocketInternal> sockobj = udpinternal->sock;
	if (sockobj == NULL)
	{
		return false;
	}
	return sockobj->async_recvfrom(buf,len,received);
}

bool UDP::async_sendto(const char * buf, uint32_t len,const NetAddr& other,const SendedCallback& sended)
{
	Public::Base::shared_ptr<SocketInternal> sockobj = udpinternal->sock;
	if (sockobj == NULL)
	{
		return false;
	}
	return sockobj->async_sendto(buf,len,other,sended);
}

int UDP::recvfrom(char *buf , uint32_t len,NetAddr& other)
{
	Public::Base::shared_ptr<SocketInternal> sockobj = udpinternal->sock;
	if (sockobj == NULL)
	{
		return -1;
	}

	return sockobj->recvfrom(buf,len,other);
}
int UDP::sendto(const char * buf, uint32_t len,const NetAddr& other)
{
	Public::Base::shared_ptr<SocketInternal> sockobj = udpinternal->sock;
	if (sockobj == NULL)
	{
		return -1;
	}

	return sockobj->sendto(buf,len,other);
}
};
};

