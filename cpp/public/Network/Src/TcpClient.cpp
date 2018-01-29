#include "SocketInternal.h"
#include "Network/TcpClient.h"
#include <memory>

using namespace std;
namespace Public{
namespace Network{

struct TCPClient::TCPClientInternalPointer
{
	Public::Base::shared_ptr<SocketInternal> sock;
};
TCPClient::TCPClient()
{
	tcpclientinternal = new TCPClientInternalPointer();
}
Public::Base::shared_ptr<Socket> TCPClient::create(const Public::Base::shared_ptr<AsyncIOWorker>& worker)
{
	if (worker == NULL) return Public::Base::shared_ptr<Socket>();

	Public::Base::shared_ptr<TCPClient> client(new TCPClient());
	client->tcpclientinternal->sock = new SocketInternal(
		worker == NULL ? Public::Base::shared_ptr<AsyncManager>():
		worker->internal->manager, client, NetType_TcpClient);

	return client;
}
TCPClient::~TCPClient()
{
	disconnect();
	SAFE_DELETE(tcpclientinternal);
	//shareptr ×Ô¶¯É¾³ý
}
bool TCPClient::bind(const NetAddr& addr,bool reusedaddr)
{
	Public::Base::shared_ptr<SocketInternal> sockobj = tcpclientinternal->sock;
	if(sockobj == NULL)
	{
		return false;
	}

	return sockobj->bind(addr,reusedaddr);
}
bool TCPClient::disconnect()
{
	if (tcpclientinternal->sock != NULL)
	{
		tcpclientinternal->sock->disconnect();
		tcpclientinternal->sock = NULL;
	}
	return true;
}
bool TCPClient::setSocketTimeout(uint32_t recvTimeout, uint32_t sendTimeout)
{
	Public::Base::shared_ptr<SocketInternal> sockobj = tcpclientinternal->sock;
	if (sockobj == NULL)
	{
		return false;
	}

	return sockobj->setSocketTimeout(recvTimeout, sendTimeout);
}

bool TCPClient::getSocketTimeout(uint32_t& recvTimeout, uint32_t& sendTimeout) const
{
	Public::Base::shared_ptr<SocketInternal> sockobj = tcpclientinternal->sock;
	if (sockobj == NULL)
	{
		return false;
	}

	return sockobj->getSocketTimeout(recvTimeout, sendTimeout);
}
bool TCPClient::getSocketBuffer(uint32_t& recvSize,uint32_t& sendSize) const
{
	Public::Base::shared_ptr<SocketInternal> sockobj = tcpclientinternal->sock;
	if (sockobj == NULL)
	{
		return false;
	}

	return sockobj->getSocketBuffer(recvSize,sendSize);
}
bool TCPClient::setSocketBuffer(uint32_t recvSize,uint32_t sendSize)
{
	Public::Base::shared_ptr<SocketInternal> sockobj = tcpclientinternal->sock;
	if (sockobj == NULL)
	{
		return false;
	}

	return sockobj->setSocketBuffer(recvSize,sendSize);
}
int TCPClient::getHandle() const
{
	Public::Base::shared_ptr<SocketInternal> sockobj = tcpclientinternal->sock;
	if (sockobj == NULL)
	{
		return false;
	}

	return sockobj->getHandle();
}

NetType TCPClient::getNetType() const
{
	return NetType_TcpClient;
}
NetAddr TCPClient::getMyAddr() const
{
	Public::Base::shared_ptr<SocketInternal> sockobj = tcpclientinternal->sock;
	if (sockobj == NULL)
	{
		return NetAddr();
	}
	return sockobj->getMyAddr();
}
NetAddr TCPClient::getOhterAddr() const
{
	Public::Base::shared_ptr<SocketInternal> sockobj = tcpclientinternal->sock;
	if (sockobj == NULL)
	{
		return NetAddr();
	}
	return sockobj->getOhterAddr();
}
bool TCPClient::async_connect(const NetAddr& addr,const ConnectedCallback& connected)
{
	Public::Base::shared_ptr<SocketInternal> sockobj = tcpclientinternal->sock;
	if (sockobj == NULL)
	{
		return false;
	}

	return sockobj->async_connect(addr,connected);
}
bool TCPClient::connect(const NetAddr& addr)
{
	Public::Base::shared_ptr<SocketInternal> sockobj = tcpclientinternal->sock;
	if (sockobj == NULL)
	{
		return false;
	}

	return sockobj->connect(addr);
}
bool TCPClient::setDisconnectCallback(const Socket::DisconnectedCallback& disconnected)
{
	Public::Base::shared_ptr<SocketInternal> sockobj = tcpclientinternal->sock;
	if (sockobj == NULL)
	{
		return false;
	}

	return sockobj->setDisconnectCallback(disconnected);
}
bool TCPClient::async_recv(char *buf , uint32_t len,const Socket::ReceivedCallback& received)
{
	Public::Base::shared_ptr<SocketInternal> sockobj = tcpclientinternal->sock;
	if (sockobj == NULL)
	{
		return false;
	}

	return sockobj->async_recv(buf,len,received);
}
bool TCPClient::async_send(const char * buf, uint32_t len,const Socket::SendedCallback& sended)
{
	Public::Base::shared_ptr<SocketInternal> sockobj = tcpclientinternal->sock;
	if (sockobj == NULL)
	{
		return false;
	}

	return sockobj->async_send(buf,len,sended);
}
int TCPClient::recv(char *buf , uint32_t len)
{
	Public::Base::shared_ptr<SocketInternal> sockobj = tcpclientinternal->sock;
	if (sockobj == NULL)
	{
		return false;
	}

	return sockobj->recv(buf,len);
}
int TCPClient::send(const char * buf, uint32_t len)
{
	Public::Base::shared_ptr<SocketInternal> sockobj = tcpclientinternal->sock;
	if (sockobj == NULL)
	{
		return false;
	}

	return sockobj->send(buf,len);
}
};
};



