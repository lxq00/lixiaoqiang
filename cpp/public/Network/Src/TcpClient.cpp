#include "SocketTcp.h"
#include "Network/TcpClient.h"
#include <memory>

using namespace std;
namespace Public{
namespace Network{

struct TCPClient::TCPClientInternalPointer:public SocketConnection
{
	TCPClientInternalPointer():SocketConnection(){}
	~TCPClientInternalPointer(){}
};
TCPClient::TCPClient(const shared_ptr<IOWorker>& _worker)
{
	shared_ptr<IOWorker> worker = _worker;
	if (worker == NULL)
	{
		worker = make_shared<IOWorker>(IOWorker::ThreadNum(2));
	}

	tcpclientinternal = new TCPClientInternalPointer();
	tcpclientinternal->internal = boost::make_shared<TCPSocketObject>(worker,this,false);
	tcpclientinternal->internal->create();
}

TCPClient::~TCPClient()
{
	disconnect();
	SAFE_DELETE(tcpclientinternal);
	//shareptr ×Ô¶¯É¾³ý
}
bool TCPClient::bind(const NetAddr& addr,bool reusedaddr)
{
	boost::shared_ptr<TCPSocketObject> sockobj;

	{
		Guard locker(tcpclientinternal->mutex);
		sockobj = tcpclientinternal->internal;
	}
	if(sockobj == NULL)
	{
		return false;
	}

	return sockobj->bind(addr,reusedaddr);
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
int TCPClient::getHandle() const
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
NetAddr TCPClient::getOhterAddr() const
{
	return tcpclientinternal->getOhterAddr();
}
bool TCPClient::async_connect(const NetAddr& addr,const ConnectedCallback& connected)
{
	boost::shared_ptr<TCPSocketObject> sockobj;

	{
		Guard locker(tcpclientinternal->mutex);
		sockobj = tcpclientinternal->internal;
	}
	if(sockobj == NULL || !connected)
	{
		return false;
	}

	return sockobj->startConnect(connected,addr);
}
bool TCPClient::connect(const NetAddr& addr)
{
	boost::shared_ptr<TCPSocketObject> sockobj;

	{
		Guard locker(tcpclientinternal->mutex);
		sockobj = tcpclientinternal->internal;
	}
	if(sockobj == NULL)
	{
		return false;
	}

	return sockobj->connect(addr);
}
bool TCPClient::setDisconnectCallback(const Socket::DisconnectedCallback& disconnected)
{
	return tcpclientinternal->setDisconnectCallback(disconnected);
}
bool TCPClient::async_recv(char *buf , uint32_t len,const Socket::ReceivedCallback& received)
{
	return tcpclientinternal->async_recv(buf,len,received);
}
bool TCPClient::async_recv(const ReceivedCallback& received, int maxlen)
{
	return tcpclientinternal->async_recv(received,maxlen);
}
bool TCPClient::async_send(const char * buf, uint32_t len,const Socket::SendedCallback& sended)
{
	return tcpclientinternal->async_send(buf,len,sended);
}
int TCPClient::recv(char *buf , uint32_t len)
{
	return tcpclientinternal->recv(buf,len);
}
int TCPClient::send(const char * buf, uint32_t len)
{
	return tcpclientinternal->send(buf,len);
}
bool TCPClient::setSocketOpt(int level, int optname, const void *optval, int optlen)
{
	boost::shared_ptr<TCPSocketObject> sockobj;

	{
		Guard locker(tcpclientinternal->mutex);
		sockobj = tcpclientinternal->internal;
	}
	if (sockobj == NULL)
	{
		return false;
	}

	return sockobj->setSocketOpt(level, optname, optval, optlen);
}
bool TCPClient::getSocketOpt(int level, int optname, void *optval, int *optlen) const
{
	boost::shared_ptr<TCPSocketObject> sockobj;
	{
		//Guard locker(mutex);
		sockobj = tcpclientinternal->internal;
	}
	if (sockobj == NULL)
	{
		return false;
	}

	return sockobj->getSocketOpt(level, optname, optval, optlen);
}
};
};



