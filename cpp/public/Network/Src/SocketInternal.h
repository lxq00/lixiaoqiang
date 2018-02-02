#ifndef __SOCKETTCPOBJECT_H__
#define __SOCKETTCPOBJECT_H__
#include "SocketObject.h"
class SocketInternal :public Socket
{
public:
	SocketInternal(const Public::Base::shared_ptr<AsyncManager>& manager, NetType type, const Public::Base::shared_ptr<Socket>& sockptr)
	{
		sock = new SocketObject(manager, type, sockptr);
	}
	SocketInternal(const Public::Base::shared_ptr<AsyncManager>& manager, int _sock, const NetAddr& otheraaddr)
	{
		sock = new SocketObject(manager, _sock, otheraaddr);
	}
	bool init()
	{
		return sock->initSocket();
	}
	~SocketInternal()
	{
		disconnect();
		sock = NULL;
	}
	bool bind(const NetAddr& addr, bool reusedaddr)
	{
		if (sock == NULL) return false;
		return sock->bind(addr,reusedaddr);
	}
	bool listen(int flag)
	{
		if (sock == NULL) return false;
		return sock->listen(flag);
	}
	int getHandle() const
	{
		if (sock == NULL) return -1;
		return sock->getHandle();
	}

	NetType getNetType() const
	{
		if (sock == NULL) return NetType_Udp;
		return sock->getNetType();
	}
	NetAddr getMyAddr() const
	{
		if (sock == NULL) return NetAddr();
		return sock->getMyAddr();
	}
	bool disconnect()
	{
		if (sock == NULL) return false;
		return sock->disconnect();
	}
	bool async_accept(const AcceptedCallback& accepted)
	{
		if (sock == NULL) return false;
		return sock->async_accept(accepted);
	}
	Public::Base::shared_ptr<Socket> accept()
	{
		if (sock == NULL) return Public::Base::shared_ptr<Socket>();
		return sock->accept();
	}
	bool setSocketTimeout(uint32_t recvTimeout, uint32_t sendTimeout)
	{
		if (sock == NULL) return false;
		return sock->setSocketTimeout(recvTimeout, sendTimeout);
	}
	bool getSocketTimeout(uint32_t& recvTimeout, uint32_t& sendTimeout) const
	{
		if (sock == NULL) return false;
		return sock->getSocketTimeout(recvTimeout, sendTimeout);
	}
	bool async_connect(const NetAddr& addr, const ConnectedCallback& connected)
	{
		if (sock == NULL) return false;
		return sock->async_connect(addr, connected);
	}
	bool connect(const NetAddr& addr)
	{
		if (sock == NULL) return false;
		return sock->connect(addr);
	}
	bool setDisconnectCallback(const Socket::DisconnectedCallback& disconnected)
	{
		if (sock == NULL) return false;
		return sock->setDisconnectCallback(disconnected);
	}
	bool async_recv(char *buf, uint32_t len, const Socket::ReceivedCallback& received)
	{
		if (sock == NULL) return false;
		return sock->async_recv(buf, len, received);
	}
	bool async_send(const char * buf, uint32_t len, const Socket::SendedCallback& sended)
	{
		if (sock == NULL) return false;
		return sock->async_send(buf, len, sended);
	}
	int recv(char *buf, uint32_t len)
	{
		if (sock == NULL) return -1;
		return sock->recv(buf, len);
	}
	int send(const char * buf, uint32_t len)
	{
		if (sock == NULL) return -1;
		return sock->send(buf, len);
	}

	bool async_recvfrom(char *buf, uint32_t len, const RecvFromCallback& received)
	{
		if (sock == NULL) return false;
		return sock->async_recvfrom(buf, len, received);
	}

	bool async_sendto(const char * buf, uint32_t len, const NetAddr& other, const SendedCallback& sended)
	{
		if (sock == NULL) return false;
		return sock->async_sendto(buf, len, other, sended);
	}

	int recvfrom(char *buf, uint32_t len, NetAddr& other)
	{
		if (sock == NULL) return -1;
		return sock->recvfrom(buf, len, other);
	}
	int sendto(const char * buf, uint32_t len, const NetAddr& other)
	{
		if (sock == NULL) return -1;
		return sock->sendto(buf, len, other);
	}
private:
	Public::Base::shared_ptr<SocketObject> sock;
};

inline Public::Base::shared_ptr<Socket> buildSocketBySock(const Public::Base::weak_ptr<AsyncManager>& manager, int sock, const NetAddr& otheraaddr)
{
	Public::Base::shared_ptr<AsyncManager> _manager = manager.lock();
	if (_manager == NULL)
	{
		return Public::Base::shared_ptr<Socket>();
	}

	Public::Base::shared_ptr<SocketInternal> sockptr(new SocketInternal(_manager,sock, otheraaddr));
	if (sockptr != NULL)
	{
		sockptr->init();
	}

	return sockptr;
}



#endif
