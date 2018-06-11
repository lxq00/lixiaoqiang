#ifndef __ASIOSOCKET_ACCEPTOBJCET_H__
#define __ASIOSOCKET_ACCEPTOBJCET_H__
#include "ASIOSocketObject.h"
#include "ASIOSocketTCPObject.h"
namespace Public{
namespace Network{
class TCPServerSocketObject:public UserThreadInfo,public boost::enable_shared_from_this<TCPServerSocketObject>
{
public:
	TCPServerSocketObject(IOWorker::IOWorkerInternal* worker,Socket* sock);
	~TCPServerSocketObject();

	virtual int getHandle();

	virtual NetAddr getMyAddr();

	virtual bool create(const NetAddr& point,bool reusedaddr);

	virtual bool destory(); 

	bool startListen(const Socket::AcceptedCallback& accepted);

	Socket* accept();

	bool setSocketTimeout(uint32_t recvTimeout,uint32_t sendTimeout);
	bool getSocketTimeout(uint32_t& recvTimeout,uint32_t& sendTimeout);

	bool setSocketOpt(int level, int optname, const void *optval, int optlen);
	bool getSocketOpt(int level, int optname, void *optval, int *optlen) const;

	bool nonBlocking(bool nonblock);
private:
	void recreate();
	void _acceptCallbackPtr(const boost::system::error_code& er);
	bool startDoAccept();
public:
	boost::shared_ptr<boost::asio::ip::tcp::acceptor>		acceptorServer;
	Socket::AcceptedCallback								acceptCallback;
	boost::shared_ptr<TCPSocketObject>						acceptPtr;
	Socket*													sock;
	NetAddr													listenAddr;
	IOWorker::IOWorkerInternal*								worker;
};

}
}


#endif //__ASIOSOCKET_ACCEPTOBJCET_H__
