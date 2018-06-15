#include "ASIOSocketTCPObject.h"
#include "Base/Base.h"
using namespace Public::Base;

namespace Public{
namespace Network{

TCPSocketObject::TCPSocketObject(const shared_ptr<IOWorker>& worker,Socket* _sock,bool isConnection):ISocketObject(_sock),connectTime(0)
{
	type = isConnection ? NetType_TcpClient : NetType_TcpConnection;
	status = NetStatus_notconnected;

	try
	{
		tcpsock = boost::shared_ptr<boost::asio::ip::tcp::socket>(new boost::asio::ip::tcp::socket(**(shared_ptr<boost::asio::io_service>*)worker->getBoostASIOIOServerSharedptr()));
	}
	catch(const std::exception& e)
	{
		logdebug("%s %d new boost::asio::ip::tcp::socket std::exception %s\r\n",__FUNCTION__,__LINE__,e.what());
	}
}

TCPSocketObject::~TCPSocketObject()
{
	destory();
}

bool TCPSocketObject::create()
{
	Guard locker(mutex);
	if(tcpsock == NULL)
	{
		return false;
	}

	try
	{
		tcpsock->open(boost::asio::ip::tcp::v4()); 
	}
	catch(const std::exception& e)
	{
		logdebug("%s %d tcpsock->open std::exception %s\r\n",__FUNCTION__,__LINE__,e.what());
		return false;
	}

	return true;
}

bool TCPSocketObject::bind(const NetAddr& point,bool reusedaddr)
{
	Guard locker(mutex);
	if(tcpsock == NULL)
	{
		return false;
	}

	try
	{
		tcpsock->set_option(boost::asio::ip::tcp::acceptor::reuse_address(reusedaddr));
		tcpsock->bind(boost::asio::ip::tcp::endpoint(point.getType() == NetAddr::netaddr_ipv4 ? boost::asio::ip::tcp::v4() : boost::asio::ip::tcp::v6(),point.getPort()));
	}
	catch(const std::exception& e)
	{
		logdebug("%s %d tcpsock->bind std::exception %s\r\n",__FUNCTION__,__LINE__,e.what());
		return false;
	}

	return true;
}

bool TCPSocketObject::nonBlocking(bool nonblock)
{
	Guard locker(mutex);
	if(tcpsock == NULL)
	{
		return false;
	}

	try
	{
		boost::asio::ip::tcp::socket::non_blocking_io nonio(nonblock);
		tcpsock->io_control(nonio);  
	}
	catch(const std::exception& e)
	{
		logerror("%s %d std::exception %s\r\n",__FUNCTION__,__LINE__,e.what());
		return false;
	}

	return true;
}
bool TCPSocketObject::destory()
{
	checkTimer = shared_ptr<Timer>();
	boost::shared_ptr<boost::asio::ip::tcp::socket> sockptr;
	ISocketObject::destory();
	{
		Guard locker(mutex);
		connectCallback = NULL;
		sockptr = tcpsock;
		tcpsock = boost::shared_ptr<boost::asio::ip::tcp::socket>();
	}
	if(sockptr != NULL)
	{
		sockptr->close();
	}
	return true;
}

int TCPSocketObject::getHandle()
{
	boost::shared_ptr<boost::asio::ip::tcp::socket> sockptr;
	{
		Guard locker(mutex);
		sockptr = tcpsock;
	}
	if(sockptr == NULL)
	{
		return 0;
	}
	
	return sockptr->native_handle();
}

bool TCPSocketObject::startConnect(const Socket::ConnectedCallback& callback,const NetAddr& otherpoint)
{
	{
		Guard locker(mutex);
		if(mustQuit || status == NetStatus_connected || connectCallback != NULL || !otherpoint.isValid())
		{
			return false;
		}
		connectCallback = callback;
		connectToEndpoint = boost::asio::ip::tcp::endpoint(boost::asio::ip::address::from_string(otherpoint.getIP()),otherpoint.getPort());
	}

	return doStartConnect();
}
bool TCPSocketObject::doStartConnect()
{
	boost::shared_ptr<boost::asio::ip::tcp::socket> sockptr;
	{
		Guard locker(mutex);
		if(tcpsock == NULL || mustQuit || status == NetStatus_connected || connectCallback == NULL)
		{
			return false;
		}
		sockptr = tcpsock;
	}

	try
	{
		sockptr->async_connect(connectToEndpoint,boost::bind(&TCPSocketObject::_connectCallbackPtr,shared_from_this(),boost::asio::placeholders::error));

	}catch(const std::exception& e)
	{
		logdebug("%s %d sockptr->async_connect std::exception %s\r\n",__FUNCTION__,__LINE__,e.what());
		return false;
	}

	return true;
}

void TCPSocketObject::_connectCallbackPtr(const boost::system::error_code& er)
{
	NetStatus currStatus = NetStatus_connected;
	{
		Guard locker(mutex);
		if(mustQuit || status == NetStatus_connected)///如果socket已经被关闭，停止
		{
			return;
		}

		///记录当前回调的处理信息
		_callbackThreadUsedStart();

		if(!er)
		{
			currStatus = status;
			status = NetStatus_connected;
		}
	}
	if(!er && currStatus == NetStatus_notconnected)
	{
		checkTimer = shared_ptr<Timer>();
		connectCallback(sock);
	}	
	///清除当前回调的记录信息
	callbackThreadUsedEnd();
	if(er)
	{
		///连接失败需要继续连接,使用定时器来出发连接
		if(checkTimer == (void*)NULL)
		{
			checkTimer = make_shared<Timer>("TCPSocketObject");
			checkTimer->start(Timer::Proc(&TCPSocketObject::timerConnectProc,this),0,500);
		}
	}
}

bool TCPSocketObject::connect(const NetAddr& otherpoint)
{
	boost::shared_ptr<boost::asio::ip::tcp::socket> sockptr;
	{
		Guard locker(mutex);
		if(tcpsock == NULL || mustQuit || status == NetStatus_connected)
		{
			return false;
		}
		sockptr = tcpsock;
		connectToEndpoint = boost::asio::ip::tcp::endpoint(boost::asio::ip::address::from_string(otherpoint.getIP()),otherpoint.getPort());
	}
	boost::system::error_code er;

	boost::system::error_code ret = sockptr->connect(connectToEndpoint,er);

	if(!er && !ret)
	{
		Guard locker(mutex);
		status = NetStatus_connected;
	}

	return  (!er && !ret) ? true : false;
}
int TCPSocketObject::send(const char* buffer,int len)
{
	boost::shared_ptr<boost::asio::ip::tcp::socket> sockptr;
	{
		Guard locker(mutex);
		if(tcpsock == NULL || mustQuit || status != NetStatus_connected)///如果socket已经被关闭，停止
		{
			return -1;
		}
		sockptr = tcpsock;
	}
	boost::system::error_code er;

	int sendlen = sockptr->send(boost::asio::buffer(buffer, len),0,er);

	return !er ? sendlen : -1;
}

int TCPSocketObject::recv(char* buffer,int maxlen)
{
	boost::shared_ptr<boost::asio::ip::tcp::socket> sockptr;
	{
		Guard locker(mutex);
		if(tcpsock == NULL || mustQuit || status != NetStatus_connected)///如果socket已经被关闭，停止
		{
			return -1;
		}
		sockptr = tcpsock;
	}
	boost::system::error_code er;

	int recvlen = sockptr->receive(boost::asio::buffer(buffer, maxlen),0,er);

	if(er.value() == boost::asio::error::eof || er.value() == boost::asio::error::connection_reset)
	{
		socketDisconnect(er.message().c_str());
	}

	return !er ? recvlen : -1;
}

bool TCPSocketObject::doStartSend(boost::shared_ptr<SendInternal>& internal)
{
	boost::shared_ptr<boost::asio::ip::tcp::socket> sockptr;
	{
		Guard locker(mutex);
		if(tcpsock == NULL || internal == NULL || mustQuit || status != NetStatus_connected)///如果socket已经被关闭，停止
		{
			return false;
		}
		sockptr = tcpsock;
	}

	try
	{
		sockptr->async_write_some(boost::asio::buffer(internal->getSendBuffer(), internal->getSendBufferLen()),boost::bind(&TCPSocketObject::_sendCallbackPtr,shared_from_this(),boost::asio::placeholders::error,boost::asio::placeholders::bytes_transferred));
	
	}catch(const std::exception& e)
	{
		logdebug("%s %d sockptr->async_write_some std::exception %s\r\n",__FUNCTION__,__LINE__,e.what());

		return false;
	}	

	return true;
}
bool TCPSocketObject::doPostRecv(boost::shared_ptr<RecvInternal>& internal)
{
	boost::shared_ptr<boost::asio::ip::tcp::socket> sockptr;
	{
		Guard locker(mutex);
		if(tcpsock == NULL || internal == NULL || mustQuit || status != NetStatus_connected)///如果socket已经被关闭，停止
		{
			return false;
		}
		sockptr = tcpsock;
	}

	try
	{
		sockptr->async_read_some(boost::asio::buffer(internal->getRecvBuffer(), internal->getRecvBufferLen()),boost::bind(&TCPSocketObject::_recvCallbackPtr,shared_from_this(),boost::asio::placeholders::error,boost::asio::placeholders::bytes_transferred));

	}catch(const std::exception& e)
	{
		logdebug("%s %d sockptr->async_read_some std::exception %s\r\n",__FUNCTION__,__LINE__,e.what());

		return false;
	}		

	return true;
}
NetAddr TCPSocketObject::getMyAddr()
{
	Guard locker(mutex);
	if(tcpsock == NULL)
	{
		return NetAddr();
	}

	try
	{
		NetAddr myaddr(tcpsock->local_endpoint().address().to_v4().to_string(),tcpsock->local_endpoint().port());

		return myaddr;
	}
	catch(const std::exception& e)
	{
		logdebug("%s %d tcpsock->local_endpoint() std::exception %s\r\n",__FUNCTION__,__LINE__,e.what());
	}

	return NetAddr();
}
NetAddr TCPSocketObject::getOtherAddr()
{
	Guard locker(mutex);
	if(tcpsock == NULL)
	{
		return NetAddr();
	}

	try
	{
		NetAddr otheraddr(tcpsock->remote_endpoint().address().to_v4().to_string(),tcpsock->remote_endpoint().port());

		return otheraddr;
	}
	catch(const std::exception& e)
	{
		logdebug("%s %d tcpsock->remote_endpoint() std::exception %s\r\n",__FUNCTION__,__LINE__,e.what());
	}

	return NetAddr();
}
bool TCPSocketObject::setSocketBuffer(uint32_t readSize,uint32_t sendSize)
{
	boost::shared_ptr<boost::asio::ip::tcp::socket> sockptr;
	{
		Guard locker(mutex);
		sockptr = tcpsock;
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
bool TCPSocketObject::getSocketBuffer(uint32_t& readSize,uint32_t& sendSize)
{
	boost::shared_ptr<boost::asio::ip::tcp::socket> sockptr;
	{
		Guard locker(mutex);
		sockptr = tcpsock;
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
bool TCPSocketObject::setSocketOpt(int level, int optname, const void *optval, int optlen)
{
	boost::shared_ptr<boost::asio::ip::tcp::socket> sockptr;
	{
		Guard locker(mutex);
		sockptr = tcpsock;
	}
	if (sockptr == NULL)
	{
		return false;
	}
	int handle = sockptr->native_handle();

	return ::setsockopt(handle, level, optname, (const char*)optval, optlen) >= 0;
}
bool TCPSocketObject::getSocketOpt(int level, int optname, void *optval, int *optlen) const
{
	boost::shared_ptr<boost::asio::ip::tcp::socket> sockptr;
	{
		//Guard locker(mutex);
		sockptr = tcpsock;
	}
	if (sockptr == NULL)
	{
		return false;
	}
	int handle = sockptr->native_handle();

	return ::getsockopt(handle, level, optname, (char*)optval, (socklen_t*)optlen) >= 0;
}
void TCPSocketObject::setStatus(NetStatus _status)
{
	connectTime = Time::getCurrentMicroSecond();
	if(checkTimer == (void*)NULL && type == NetType_TcpConnection)
	{
		checkTimer = make_shared<Timer>("TCPSocketObject");
		checkTimer->start(Timer::Proc(&TCPSocketObject::timerCheckFirstRecvTimeoutProc,this),0,5000);
	}

	ISocketObject::setStatus(_status);
}
void TCPSocketObject::timerConnectProc(unsigned long param)
{
	doStartConnect();
}
void TCPSocketObject::timerCheckFirstRecvTimeoutProc(unsigned long param)
{
	uint64_t nowTime = Time::getCurrentMilliSecond();

	if(nowTime - connectTime >= 10000) ///10秒没接受到数据断开
	{
		checkTimer->stop();

		///断开连接通知应用层
		destory();

		{
			Guard locker(mutex);
			if(mustQuit)///如果socket已经被关闭，停止
			{
				return;
			}
			///记录当前回调的处理信息
			_callbackThreadUsedStart();
		}
		disconnectCallback(sock,"accept新产生的socket10秒未接受到数据，断开连接");

		///清除当前回调的记录信息
		callbackThreadUsedEnd();
	}
}
void TCPSocketObject::_recvCallbackPtr(const boost::system::error_code& er, std::size_t length)
{
	if(!er && length > 0)
	{
		checkTimer = shared_ptr<Timer>();
	}
	ISocketObject::_recvCallbackPtr(er,length);
}


}
}

