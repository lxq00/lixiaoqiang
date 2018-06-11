#ifndef __SOCKETTCP_H__
#define __SOCKETTCP_H__
#include "ASIOSocketTCPObject.h"
#include "Network/Socket.h"
namespace Public {
namespace Network{

using namespace Public::Base;

class SocketConnection:public Socket
{
protected:
	SocketConnection(){}
public:	
	SocketConnection(boost::shared_ptr<TCPSocketObject>& sockobj);
	virtual ~SocketConnection();
	virtual bool disconnect();
	bool getSocketBuffer(uint32_t& recvSize,uint32_t& sendSize) const;
	bool setSocketBuffer(uint32_t recvSize,uint32_t sendSize);
	bool getSocketTimeout(uint32_t& recvTimeout,uint32_t& sendTimeout) const;
	bool setSocketTimeout(uint32_t recvTimeout,uint32_t sendTimeout);
	bool nonBlocking(bool nonblock);
	virtual bool setDisconnectCallback(const DisconnectedCallback& disconnected);
	virtual bool async_recv(char *buf , uint32_t len,const ReceivedCallback& received);
	virtual bool async_recv(const ReceivedCallback& received, int maxlen);
	virtual bool async_send(const char * buf, uint32_t len,const SendedCallback& sended);
	int recv(char *buf , uint32_t len);
	int send(const char * buf, uint32_t len);
	int getHandle() const ;	
	NetStatus getStatus() const;
	NetType getNetType() const; 
	NetAddr getMyAddr() const;
	NetAddr getOhterAddr() const;	
	bool setSocketOpt(int level, int optname, const void *optval, int optlen);
	bool getSocketOpt(int level, int optname, void *optval, int *optlen) const;
public:
	Mutex								mutex;
	boost::shared_ptr<TCPSocketObject>	internal;
};


};
};

#endif
