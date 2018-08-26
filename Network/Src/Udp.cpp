#include "ASIOSocketUDP.h"
#include "Network/Udp.h"

using namespace std;
namespace Public{
namespace Network{

struct UDP::UDPInternalPointer
{
	boost::shared_ptr<ASIOSocketUDP>			sock;
};
shared_ptr<Socket> UDP::create(const shared_ptr<IOWorker>& worker)
{
	shared_ptr<UDP> sock = shared_ptr<UDP>(new UDP(worker));
	sock->udpinternal->sock->initSocketptr(sock);

	return sock;
}
UDP::UDP(const shared_ptr<IOWorker>& worker)
{
	udpinternal = new UDPInternalPointer;
	udpinternal->sock = boost::make_shared<ASIOSocketUDP>(worker);
	udpinternal->sock->create();
}
UDP::~UDP()
{
	disconnect();
	SAFE_DELETE(udpinternal);
}
bool UDP::bind(const NetAddr& addr,bool reusedaddr)
{
	boost::shared_ptr<ASIOSocketUDP> sockobj;

	{
		sockobj = udpinternal->sock;
	}
	if(sockobj == NULL)
	{
		return false;
	}

	return sockobj->bind(addr,reusedaddr);
}
bool UDP::disconnect()
{
	boost::shared_ptr<ASIOSocketUDP> sockobj;

	{
		sockobj = udpinternal->sock;
	}
	if(sockobj != NULL)
	{
		sockobj->disconnect();
	}

	return true;
}
bool UDP::getSocketBuffer(uint32_t& recvSize,uint32_t& sendSize) const
{
	boost::shared_ptr<ASIOSocketUDP> sockobj;

	{
		sockobj = udpinternal->sock;
	}
	if(sockobj == NULL)
	{
		return false;
	}

	return sockobj->getSocketBuffer(recvSize,sendSize);
}
bool UDP::setSocketBuffer(uint32_t recvSize,uint32_t sendSize)
{
	boost::shared_ptr<ASIOSocketUDP> sockobj;

	{
		sockobj = udpinternal->sock;
	}
	if(sockobj == NULL)
	{
		return false;
	}

	return sockobj->setSocketBuffer(recvSize,sendSize);
}
bool UDP::getSocketTimeout(uint32_t& recvTimeout,uint32_t& sendTimeout) const
{
	boost::shared_ptr<ASIOSocketUDP> sockobj;

	{
		sockobj = udpinternal->sock;
	}
	if(sockobj == NULL)
	{
		return false;
	}

	return sockobj->getSocketTimeout(recvTimeout,sendTimeout);
}
bool UDP::setSocketTimeout(uint32_t recvTimeout,uint32_t sendTimeout)
{
	boost::shared_ptr<ASIOSocketUDP> sockobj;

	{
		sockobj = udpinternal->sock;
	}
	if(sockobj == NULL)
	{
		return false;
	}

	return sockobj->setSocketTimeout(recvTimeout,sendTimeout);
}
bool UDP::nonBlocking(bool nonblock)
{
	boost::shared_ptr<ASIOSocketUDP> sockobj;

	{
		sockobj = udpinternal->sock;
	}
	if(sockobj == NULL)
	{
		return false;
	}

	return sockobj->nonBlocking(nonblock);
}
int UDP::getHandle() const
{
	boost::shared_ptr<ASIOSocketUDP> sockobj;

	{
		sockobj = udpinternal->sock;
	}
	if(sockobj == NULL)
	{
		return false;
	}

	return sockobj->getHandle();
}
NetType UDP::getNetType() const
{
	return NetType_Udp;
}
NetAddr UDP::getMyAddr() const
{
	boost::shared_ptr<ASIOSocketUDP> sockobj;

	{
		sockobj = udpinternal->sock;
	}
	if(sockobj == NULL)
	{
		return false;
	}

	return sockobj->getMyAddr();
}

bool UDP::async_recvfrom(char *buf , uint32_t len,const RecvFromCallback& received)
{
	boost::shared_ptr<ASIOSocketUDP> sockobj;

	{
		sockobj = udpinternal->sock;
	}
	if(sockobj == NULL || buf == NULL || len == 0 || !received)
	{
		return false;
	}

	return sockobj->async_recvfrom(buf,len,received);
}
bool UDP::async_recvfrom(const RecvFromCallback& received, int maxlen)
{
	boost::shared_ptr<ASIOSocketUDP> sockobj;

	{
		sockobj = udpinternal->sock;
	}
	if (sockobj == NULL || maxlen == 0 || !received)
	{
		return false;
	}
	return sockobj->async_recvfrom(received,maxlen);
}
bool UDP::async_sendto(const char * buf, uint32_t len,const NetAddr& other,const SendedCallback& sended)
{
	boost::shared_ptr<ASIOSocketUDP> sockobj;

	{
		sockobj = udpinternal->sock;
	}
	if(sockobj == NULL || buf == NULL || len == 0 || !sended)
	{
		return false;
	}

	return sockobj->async_sendto(buf,len,other,sended);
}

int UDP::recvfrom(char *buf , uint32_t len,NetAddr& other)
{
	boost::shared_ptr<ASIOSocketUDP> sockobj;

	{
		sockobj = udpinternal->sock;
	}
	if(sockobj == NULL || buf == NULL || len == 0)
	{
		return false;
	}

	return sockobj->recvfrom(buf,len,other);
}
int UDP::sendto(const char * buf, uint32_t len,const NetAddr& other)
{
	boost::shared_ptr<ASIOSocketUDP> sockobj;

	{
		sockobj = udpinternal->sock;
	}
	if(sockobj == NULL || buf == NULL || len == 0)
	{
		return false;
	}

	return sockobj->sendto(buf,len,other);
}
bool UDP::setSocketOpt(int level, int optname, const void *optval, int optlen)
{
	boost::shared_ptr<ASIOSocketUDP> sockobj;

	{
		sockobj = udpinternal->sock;
	}
	if (sockobj == NULL)
	{
		return false;
	}

	return sockobj->setSocketOpt(level, optname, optval,optlen);
}
bool UDP::getSocketOpt(int level, int optname, void *optval, int *optlen) const
{
	boost::shared_ptr<ASIOSocketUDP> sockobj;

	{
		sockobj = udpinternal->sock;
	}
	if (sockobj == NULL)
	{
		return false;
	}

	return sockobj->getSocketOpt(level, optname, optval, optlen);
}

};
};

