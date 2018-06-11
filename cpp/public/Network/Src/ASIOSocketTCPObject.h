#ifndef __ASIOSOCKET_TCPOBJCET_H__
#define __ASIOSOCKET_TCPOBJCET_H__
#include "ASIOSocketObject.h"

namespace Public{
namespace Network{

///TCP的socket处理
class TCPSocketObject:public ISocketObject,public boost::enable_shared_from_this<TCPSocketObject>
{
public:
	TCPSocketObject(IOWorker::IOWorkerInternal* worker,Socket* sock,bool isConnection);
	virtual ~TCPSocketObject();

	void setDisconnectCallback(const Socket::DisconnectedCallback& callback) {disconnectCallback = callback;}

	void socketDisconnect(const char* error)
	{
		NetStatus curstatus;
		{
			Guard locker(mutex);
			curstatus = status;
			status = NetStatus_notconnected;
		}
		if(curstatus == NetStatus_connected)
		{
			disconnectCallback(sock,error);
		}
	}

	boost::asio::ip::tcp::socket* getSocketPtr(){return tcpsock.get();}

	int getHandle();

	bool nonBlocking(bool nonblock);

	bool bind(const NetAddr& point,bool reusedaddr);

	bool create();

	bool destory(); 

	bool startConnect(const Socket::ConnectedCallback& callback,const NetAddr& otherpoint);

	bool connect(const NetAddr& otherpoint);

	int send(const char* buffer,int len);

	int recv(char* buffer,int maxlen);

	NetAddr getMyAddr();

	NetAddr getOtherAddr();

	bool setSocketBuffer(uint32_t readSize,uint32_t sendSize);
	bool getSocketBuffer(uint32_t& readSize,uint32_t& sendSize);
	bool setSocketOpt(int level, int optname, const void *optval, int optlen);
	bool getSocketOpt(int level, int optname, void *optval, int *optlen) const;
	void setStatus(NetStatus _status);
private:
	bool doStartConnect();

	void _connectCallbackPtr(const boost::system::error_code& er);
	bool doStartSend(boost::shared_ptr<SendInternal>& internal);
	bool doPostRecv(boost::shared_ptr<RecvInternal>& internal);
	
	void timerConnectProc(unsigned long param);
	void timerCheckFirstRecvTimeoutProc(unsigned long param);
	void _recvCallbackPtr(const boost::system::error_code& er, std::size_t length);
private:
	boost::asio::ip::tcp::endpoint					connectToEndpoint;
	boost::shared_ptr<boost::asio::ip::tcp::socket>	tcpsock;
	Socket::ConnectedCallback						connectCallback;

	Socket::DisconnectedCallback					disconnectCallback;	///断开回调通知

	shared_ptr<Timer>					checkTimer;   ///用timer不是很好的处理方式，connect和accept的socket都会产生timer，有点浪费，但没想到更好的方式
	uint64_t										connectTime;
};


}
}


#endif //__ASIOSOCKET_OBJCET_H__
