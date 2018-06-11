#include "ASIOSocketUDPObject.h"

namespace Public{
namespace Network{

UDPSocketObject::UDPSocketObject(IOWorker::IOWorkerInternal* worker,Socket* _sock):ISocketObject(_sock)
{
	type = NetType_Udp;
	status = NetStatus_connected;

	try
	{
		udpsock = boost::shared_ptr<boost::asio::ip::udp::socket>(new boost::asio::ip::udp::socket(worker->getIOServer()));
	}
	catch(const std::exception& e)
	{
		logdebug("%s %d new boost::asio::ip::udp::socket std::exception %s\r\n",__FUNCTION__,__LINE__,e.what());
	}

}
UDPSocketObject::~UDPSocketObject()
{
	destory();
}

bool UDPSocketObject::create()
{
	Guard locker(mutex);

	if(udpsock == NULL)
	{
		return false;
	}

	try
	{
		udpsock->open(boost::asio::ip::udp::v4()); 
	}
	catch(const std::exception& e)
	{
		logdebug("%s %d udpsock->open std::exception %s\r\n",__FUNCTION__,__LINE__,e.what());
		return false;
	}

	return true;
}
bool UDPSocketObject::nonBlocking(bool nonblock)
{
	Guard locker(mutex);

	if(udpsock == NULL)
	{
		return false;
	}

	try
	{
		boost::asio::ip::udp::socket::non_blocking_io nonio(nonblock);
		udpsock->io_control(nonio);  
	}
	catch(const std::exception& e)
	{
		logerror("%s %d std::exception %s\r\n",__FUNCTION__,__LINE__,e.what());
		return false;
	}
	return true;
}

bool UDPSocketObject::bind(const NetAddr& point,bool reusedaddr)
{
	Guard locker(mutex);

	if(udpsock == NULL)
	{
		return false;
	}
	try
	{
		udpsock->set_option(boost::asio::ip::tcp::acceptor::reuse_address(reusedaddr));
		udpsock->bind(boost::asio::ip::udp::endpoint(point.getType() == NetAddr::netaddr_ipv4 ? boost::asio::ip::udp::v4() : boost::asio::ip::udp::v6(),point.getPort()));
	}
	catch(const std::exception& e)
	{
		logdebug("%s %d udpsock->bind std::exception %s\r\n",__FUNCTION__,__LINE__,e.what());
		return false;
	}

	return true;
}

bool UDPSocketObject::destory()
{
	boost::shared_ptr<boost::asio::ip::udp::socket> sockptr;
	ISocketObject::destory();
	{
		Guard locker(mutex);
		sockptr = udpsock;
		udpsock = boost::shared_ptr<boost::asio::ip::udp::socket>();
	}
	if(sockptr != NULL)
	{
		sockptr->close();
	}

	return true;
}
NetAddr UDPSocketObject::getMyAddr()
{
	Guard locker(mutex);
	if(udpsock == NULL)
	{
		return NetAddr();
	}

	try
	{
		NetAddr myaddr(udpsock->local_endpoint().address().to_v4().to_string(),udpsock->local_endpoint().port());

		return myaddr;
	}
	catch(const std::exception& e)
	{
		logdebug("%s %d udpsock->local_endpoint() std::exception %s\r\n",__FUNCTION__,__LINE__,e.what());
	}

	return NetAddr();
}
int UDPSocketObject::getHandle()
{
	boost::shared_ptr<boost::asio::ip::udp::socket> sockptr;
	{
		Guard locker(mutex);
		sockptr = udpsock;
	}
	if(sockptr == NULL)
	{
		return 0;
	}

	return sockptr->native_handle();
}

bool UDPSocketObject::doStartSend(boost::shared_ptr<SendInternal>& internal)
{
	boost::shared_ptr<boost::asio::ip::udp::socket> sockptr;

	{
		Guard locker(mutex);
		if(udpsock == NULL || internal == NULL || mustQuit || status != NetStatus_connected)///如果socket已经被关闭，停止
		{
			return false;
		}
		sockptr = udpsock;
	}

	try
	{
		const NetAddr& toaddr = internal->getSendAddr();

		boost::asio::ip::udp::endpoint otehrpoint = boost::asio::ip::udp::endpoint(boost::asio::ip::address::from_string(toaddr.getIP()),toaddr.getPort());

		sockptr->async_send_to(boost::asio::buffer(internal->getSendBuffer(), internal->getSendBufferLen()),otehrpoint,
			boost::bind(&UDPSocketObject::_sendCallbackPtr,shared_from_this(),boost::asio::placeholders::error,boost::asio::placeholders::bytes_transferred));

	}catch(const std::exception& e)
	{
		logdebug("%s %d sockptr->async_send_to std::exception %s\r\n",__FUNCTION__,__LINE__,e.what());

		return false;
	}	

	return true;
}
bool UDPSocketObject::doPostRecv(boost::shared_ptr<RecvInternal>& internal)
{
	boost::shared_ptr<boost::asio::ip::udp::socket> sockptr;
	{
		Guard locker(mutex);
		if(udpsock == NULL || internal == NULL || mustQuit || status != NetStatus_connected)///如果socket已经被关闭，停止
		{
			return false;
		}
		sockptr = udpsock;
	}

	try
	{
		sockptr->async_receive_from(boost::asio::buffer(internal->getRecvBuffer(), internal->getRecvBufferLen()),internal->getRecvPoint(),boost::bind(&UDPSocketObject::_recvCallbackPtr,shared_from_this(),boost::asio::placeholders::error,boost::asio::placeholders::bytes_transferred));

	}catch(const std::exception& e)
	{
		logdebug("%s %d sockptr->async_receive_from std::exception %s\r\n",__FUNCTION__,__LINE__,e.what());

		return false;
	}		

	return true;
}

bool UDPSocketObject::setSocketBuffer(uint32_t readSize,uint32_t sendSize)
{
	boost::shared_ptr<boost::asio::ip::udp::socket> sockptr;
	{
		Guard locker(mutex);
		sockptr = udpsock;
	}
	if(sockptr == NULL)
	{
		return false;
	}

	boost::asio::socket_base::receive_buffer_size recvsize(readSize);
	boost::asio::socket_base::send_buffer_size sndsize(sendSize);

	try{
		sockptr->set_option(recvsize);
		sockptr->set_option(sndsize);
	}catch(const std::exception& e)
	{
		logdebug("%s %d sockptr->set_option std::exception %s\r\n",__FUNCTION__,__LINE__,e.what());
		return false;
	}

	return true;
}
bool UDPSocketObject::getSocketBuffer(uint32_t& readSize,uint32_t& sendSize)
{
	boost::shared_ptr<boost::asio::ip::udp::socket> sockptr;
	{
		Guard locker(mutex);
		sockptr = udpsock;
	}
	if(sockptr == NULL)
	{
		return false;
	}

	boost::asio::socket_base::receive_buffer_size recvsize(0);
	boost::asio::socket_base::send_buffer_size sndsize(0);

	try{
		sockptr->get_option(recvsize);
		sockptr->get_option(sndsize);
	}catch(const std::exception& e)
	{
		logdebug("%s %d sockptr->get_option std::exception %s\r\n",__FUNCTION__,__LINE__,e.what());
		return false;
	}

	readSize = recvsize.value();
	sendSize = sndsize.value();

	return true;
}
bool UDPSocketObject::setSocketOpt(int level, int optname, const void *optval, int optlen)
{
	boost::shared_ptr<boost::asio::ip::udp::socket>	sockptr;
	{
		Guard locker(mutex);
		sockptr = udpsock;
	}
	if (sockptr == NULL)
	{
		return false;
	}

	return ::setsockopt(sockptr->native_handle(),level, optname, (const char*)optval, optlen);
}
bool UDPSocketObject::getSocketOpt(int level, int optname, void *optval, int *optlen) const
{
	boost::shared_ptr<boost::asio::ip::udp::socket>	sockptr;
	{
		//Guard locker(mutex);
		sockptr = udpsock;
	}
	if (sockptr == NULL)
	{
		return false;
	}

	return ::getsockopt(sockptr->native_handle(),level, optname, (char*)optval,(socklen_t*) optlen);
}
int UDPSocketObject::sendto(const char* buffer,int len,const NetAddr& otheraddr)
{
	boost::shared_ptr<boost::asio::ip::udp::socket> sockptr;
	{
		Guard locker(mutex);
		if(udpsock == NULL || mustQuit)///如果socket已经被关闭，停止
		{
			return false;
		}
		sockptr = udpsock;
	}

	boost::system::error_code er;

	boost::asio::ip::udp::endpoint otehrpoint = boost::asio::ip::udp::endpoint(boost::asio::ip::address::from_string(otheraddr.getIP()),otheraddr.getPort());

	int sendlen = sockptr->send_to(boost::asio::buffer(buffer, len),otehrpoint,0,er);

	return !er ? sendlen : -1;
}
int UDPSocketObject::recvfrom(char* buffer,int maxlen,NetAddr& otheraddr)
{
	boost::shared_ptr<boost::asio::ip::udp::socket> sockptr;
	{
		Guard locker(mutex);
		if(udpsock == NULL || mustQuit)///如果socket已经被关闭，停止
		{
			return false;
		}
		sockptr = udpsock;
	}

	boost::system::error_code er;

	boost::asio::ip::udp::endpoint otehrpoint;

	int recvlen = sockptr->receive_from(boost::asio::buffer(buffer, maxlen),otehrpoint,0,er);

	if(!er)
	{
		try
		{
			otheraddr = NetAddr(otehrpoint.address().to_v4().to_string(),otehrpoint.port());
		}
		catch(const std::exception& e)
		{
			logdebug("%s %d acceptorServer->local_endpoint() std::exception %s\r\n",__FUNCTION__,__LINE__,e.what());
		}
	}

	return !er ? recvlen : -1;
}
}
}

