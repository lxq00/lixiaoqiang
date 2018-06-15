#ifndef __ASIOSOCKET_UDPOBJCET_H__
#define __ASIOSOCKET_UDPOBJCET_H__
#include "ASIOSocketObject.h"

namespace Public{
namespace Network{

///UDPµÄsocket´¦Àí
class UDPSocketObject:public ISocketObject,public boost::enable_shared_from_this<UDPSocketObject>
{
public:
	UDPSocketObject(const shared_ptr<IOWorker>& worker,Socket* sock);
	virtual ~UDPSocketObject();

	virtual boost::asio::ip::udp::socket* getSocketPtr(){return udpsock.get();}

	virtual int getHandle();

	virtual bool nonBlocking(bool nonblock);

	virtual bool bind(const NetAddr& point,bool reusedaddr);

	virtual bool create();

	virtual bool destory(); 

	virtual NetAddr getMyAddr();

	int sendto(const char* buffer,int len,const NetAddr& otheraddr);

	int recvfrom(char* buffer,int maxlen,NetAddr& otheraddr);

	bool setSocketBuffer(uint32_t readSize,uint32_t sendSize);
	bool getSocketBuffer(uint32_t& readSize,uint32_t& sendSize);
	bool setSocketOpt(int level, int optname, const void *optval, int optlen);
	bool getSocketOpt(int level, int optname, void *optval, int *optlen) const;
private:
	virtual bool doStartSend(boost::shared_ptr<SendInternal>& internal) ;
	virtual bool doPostRecv(boost::shared_ptr<RecvInternal>& internal) ;
private:
	boost::shared_ptr<boost::asio::ip::udp::socket>		udpsock;
};

}
}


#endif //__ASIOSOCKET_OBJCET_H__
