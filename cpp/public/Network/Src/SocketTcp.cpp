#include "SocketTcp.h"
namespace Xunmei{
namespace Network{

SocketConnection::SocketConnection(boost::shared_ptr<TCPSocketObject>& sockobj):internal(sockobj){}
SocketConnection::~SocketConnection()
{
	disconnect();
}
bool SocketConnection::disconnect()
{
	boost::shared_ptr<TCPSocketObject>	sockobj;
	{
		Guard locker(mutex);
		sockobj = internal;
		internal = boost::shared_ptr<TCPSocketObject>();
	}
	if(sockobj != NULL)
	{
		sockobj->destory();
	}

	return true;
}
bool SocketConnection::getSocketBuffer(uint32_t& recvSize,uint32_t& sendSize) const
{
	boost::shared_ptr<TCPSocketObject>	sockinternal;
	{
		//Guard locker(mutex);
		sockinternal = internal;
	}
	if(sockinternal == NULL)
	{
		return false;
	}

	return sockinternal->getSocketBuffer(recvSize,sendSize);
}
bool SocketConnection::setSocketBuffer(uint32_t recvSize,uint32_t sendSize)
{
	boost::shared_ptr<TCPSocketObject>	sockinternal;
	{
		Guard locker(mutex);
		sockinternal = internal;
	}
	if(sockinternal == NULL)
	{
		return false;
	}

	return sockinternal->setSocketBuffer(recvSize,sendSize);
}
bool SocketConnection::setDisconnectCallback(const DisconnectedCallback& disconnected)
{
	boost::shared_ptr<TCPSocketObject>	sockinternal;
	{
		Guard locker(mutex);
		sockinternal = internal;
	}
	if(sockinternal == NULL)
	{
		return false;
	}

	sockinternal->setDisconnectCallback(disconnected);

	return true;
}
bool SocketConnection::getSocketTimeout(uint32_t& recvTimeout,uint32_t& sendTimeout) const
{
	boost::shared_ptr<TCPSocketObject>	sockinternal;
	{
		//Guard locker(mutex);
		sockinternal = internal;
	}
	if(sockinternal == NULL)
	{
		return false;
	}

	return sockinternal->getSocketTimeout(recvTimeout,sendTimeout);
}
bool SocketConnection::setSocketTimeout(uint32_t recvTimeout,uint32_t sendTimeout)
{
	boost::shared_ptr<TCPSocketObject>	sockinternal;
	{
		Guard locker(mutex);
		sockinternal = internal;
	}
	if(sockinternal == NULL)
	{
		return false;
	}

	return sockinternal->setSocketTimeout(recvTimeout,sendTimeout);
}
bool SocketConnection::nonBlocking(bool nonblock)
{
	boost::shared_ptr<TCPSocketObject>	sockinternal;
	{
		Guard locker(mutex);
		sockinternal = internal;
	}
	if(sockinternal == NULL)
	{
		return false;
	}

	return sockinternal->nonBlocking(nonblock);
}
bool SocketConnection::async_recv(char *buf , uint32_t len,const ReceivedCallback& received)
{
	boost::shared_ptr<TCPSocketObject>	sockinternal;
	{
		Guard locker(mutex);
		sockinternal = internal;
	}
	if(sockinternal == NULL || buf == NULL || len == 0 || received == NULL)
	{
		return false;
	}

	boost::shared_ptr<RecvInternal> recvptr = boost::make_shared<TCPRecvInternal>((Socket*)this,buf,len,received);

	return sockinternal->postReceive(recvptr);
}
bool SocketConnection::async_send(const char * buf, uint32_t len,const SendedCallback& sended)
{
	boost::shared_ptr<TCPSocketObject>	sockinternal;
	{
		Guard locker(mutex);
		sockinternal = internal;
	}
	if(sockinternal == NULL || buf == NULL || len == 0 || sended == NULL)
	{
		return false;
	}
	boost::shared_ptr<SendInternal> sendptr = boost::make_shared<RepeatSendInternal>((Socket*)this,buf,len,sended);

	return sockinternal->beginSend(sendptr);
}
int SocketConnection::recv(char *buf , uint32_t len)
{
	boost::shared_ptr<TCPSocketObject>	sockinternal;
	{
		Guard locker(mutex);
		sockinternal = internal;
	}
	if(sockinternal == NULL || buf == NULL || len == 0)
	{
		return -1;
	}

	return sockinternal->recv(buf,len);
}
int SocketConnection::send(const char * buf, uint32_t len)
{
	boost::shared_ptr<TCPSocketObject>	sockinternal;
	{
		Guard locker(mutex);
		sockinternal = internal;
	}
	if(sockinternal == NULL || buf == NULL || len == 0)
	{
		return -1;
	}

	return sockinternal->send(buf,len);
}
int SocketConnection::getHandle() const 
{
	boost::shared_ptr<TCPSocketObject>	sockinternal;
	{
		//Guard locker(mutex);
		sockinternal = internal;
	}
	if(sockinternal == NULL)
	{
		return 0;
	}

	return sockinternal->getHandle();
}
NetStatus SocketConnection::getStatus() const
{
	boost::shared_ptr<TCPSocketObject>	sockinternal;
	{
		//Guard locker(mutex);
		sockinternal = internal;
	}
	if(sockinternal == NULL)
	{
		return NetStatus_error;
	}

	return sockinternal->getStatus();
}
NetType SocketConnection::getNetType() const
{
	return NetType_TcpConnection;
}
NetAddr SocketConnection::getMyAddr() const
{
	boost::shared_ptr<TCPSocketObject>	sockinternal;
	{
		//Guard locker(mutex);
		sockinternal = internal;
	}
	if(sockinternal == NULL)
	{
		return NetStatus_error;
	}

	return sockinternal->getMyAddr();
}
NetAddr SocketConnection::getOhterAddr() const
{
	boost::shared_ptr<TCPSocketObject>	sockinternal;
	{
		//Guard locker(mutex);
		sockinternal = internal;
	}
	if(sockinternal == NULL)
	{
		return NetStatus_error;
	}

	return sockinternal->getOtherAddr();
}
};
};


