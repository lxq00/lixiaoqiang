#include "ASIOSocketAcceptObject.h"
#include "SocketTcp.h"

namespace Public{
namespace Network{

TCPServerSocketObject::TCPServerSocketObject(IOWorker::IOWorkerInternal* _worker,Socket* _sock):sock(_sock),worker(_worker)
{
}
TCPServerSocketObject::~TCPServerSocketObject()
{
	destory();
}

int TCPServerSocketObject::getHandle()
{
	boost::shared_ptr<boost::asio::ip::tcp::acceptor> acceptptr;
	{
		Guard locker(mutex);
		acceptptr = acceptorServer;
	}
	if(acceptptr == NULL)
	{
		return 0;
	}

	return acceptptr->native_handle();
}
NetAddr TCPServerSocketObject::getMyAddr()
{
	Guard locker(mutex);
	if(acceptorServer == NULL)
	{
		return NetAddr();
	}

	try
	{
		NetAddr myaddr(acceptorServer->local_endpoint().address().to_v4().to_string(),acceptorServer->local_endpoint().port());

		return myaddr;
	}
	catch(const std::exception& e)
	{
		logdebug("%s %d acceptorServer->local_endpoint() std::exception %s\r\n",__FUNCTION__,__LINE__,e.what());
	}

	return NetAddr();
}
bool TCPServerSocketObject::create(const NetAddr& point,bool reusedaddr)
{
	Guard locker(mutex);
	if(!point.isValid() || acceptorServer != NULL)
	{
		return false;
	}
	listenAddr = point;


	try
	{
		boost::asio::ip::tcp::endpoint bindaddr = boost::asio::ip::tcp::endpoint(point.getType() == NetAddr::netaddr_ipv4 ? boost::asio::ip::tcp::v4() : boost::asio::ip::tcp::v6(),point.getPort());
			
		acceptorServer = boost::shared_ptr<boost::asio::ip::tcp::acceptor>(new boost::asio::ip::tcp::acceptor(worker->getIOServer()));
		acceptorServer->open(bindaddr.protocol());
		acceptorServer->set_option(boost::asio::ip::tcp::acceptor::reuse_address(reusedaddr));
		acceptorServer->bind(bindaddr);
		acceptorServer->listen();
	}
	catch(const std::exception& e)
	{
		logdebug("%s %d std::exception %s reusedaddr %d port %d\r\n",__FUNCTION__,__LINE__,e.what(),reusedaddr,listenAddr.getPort());
		return false;
	}

	return true;
}

bool TCPServerSocketObject::destory()
{
	boost::shared_ptr<boost::asio::ip::tcp::acceptor> acceptsvrptr;
	{
		Guard locker(mutex);
		mustQuit = true;
		acceptsvrptr = acceptorServer;
		acceptorServer = boost::shared_ptr<boost::asio::ip::tcp::acceptor>();
	}
	acceptPtr = boost::shared_ptr<TCPSocketObject>();
	acceptsvrptr = boost::shared_ptr<boost::asio::ip::tcp::acceptor>();

	waitAllOtherCallbackThreadUsedEnd();

	return true;
}

bool TCPServerSocketObject::startListen(const Socket::AcceptedCallback& accepted)
{
	{
		Guard locker(mutex);
		if(acceptorServer == NULL || mustQuit || acceptCallback != NULL)
		{
			return false;
		}
		acceptCallback = accepted;
	}
	

	return startDoAccept();
}
void TCPServerSocketObject::recreate()
{
	{
		Guard locker(mutex);
		acceptorServer = boost::shared_ptr<boost::asio::ip::tcp::acceptor>();
	}
	create(listenAddr,true);
}
void TCPServerSocketObject::_acceptCallbackPtr(const boost::system::error_code& er)
{
	boost::shared_ptr<TCPSocketObject> socketobj;
	Socket::AcceptedCallback callback;
	{
		Guard locker(mutex);
		if(acceptorServer == NULL || mustQuit)
		{
			return;
		}
		socketobj = acceptPtr;
		acceptPtr = boost::shared_ptr<TCPSocketObject>();
		callback = acceptCallback;

		_callbackThreadUsedStart();
	}
	if(!er && socketobj != NULL && callback != NULL)
	{
		Socket* newsock = new SocketConnection(socketobj);

		socketobj->setSocketPtr(newsock);
		socketobj->setStatus(NetStatus_connected);
		socketobj->nonBlocking(true);

		callback(sock,newsock);
	}
	callbackThreadUsedEnd();

	if(er)
	{
		///socket出异常了，重置下socket
		recreate();
	}

	startDoAccept();
}
bool TCPServerSocketObject::startDoAccept()
{
	boost::shared_ptr<TCPSocketObject> acceptsock;
	boost::shared_ptr<boost::asio::ip::tcp::acceptor> accepetserptr;
	{
		Guard locker(mutex);
		if(acceptorServer == NULL || mustQuit || acceptCallback == NULL || acceptPtr != NULL)
		{
			return false;
		}
		acceptPtr = boost::make_shared<TCPSocketObject>(worker,(Public::Network::Socket *)NULL,true);
		acceptsock = acceptPtr;
		accepetserptr = acceptorServer;
	}

	try
	{
		boost::asio::ip::tcp::socket& newsock = *(boost::asio::ip::tcp::socket*)acceptsock->getSocketPtr();
		accepetserptr->async_accept(newsock,boost::bind(&TCPServerSocketObject::_acceptCallbackPtr,shared_from_this(),boost::asio::placeholders::error));

	}catch(const std::exception& e)
	{
		logdebug("%s %d accepetserptr->async_accept std::exception %s\r\n",__FUNCTION__,__LINE__,e.what());
		return false;
	}

	return true;
}

Socket* TCPServerSocketObject::accept()
{
	boost::shared_ptr<TCPSocketObject> acceptsock;
	boost::shared_ptr<boost::asio::ip::tcp::acceptor> accepetserptr;
	{
		Guard locker(mutex);
		if(acceptorServer == NULL || mustQuit || acceptPtr != NULL)
		{
			return NULL;
		}
		acceptPtr = boost::make_shared<TCPSocketObject>(worker,(Public::Network::Socket *)NULL,true);
		acceptsock = acceptPtr;
		accepetserptr = acceptorServer;
	}

	boost::system::error_code er;

	boost::asio::ip::tcp::socket& newsockptr = *(boost::asio::ip::tcp::socket*)acceptsock->getSocketPtr();
	boost::system::error_code re = accepetserptr->accept(newsockptr,er);


	Socket* newsock = NULL;
	if(!re && !er)
	{
		newsock = new SocketConnection(acceptsock);

		acceptsock->setSocketPtr(newsock);
		acceptsock->setStatus(NetStatus_connected);
		acceptsock->nonBlocking(true);
	}

	{
		Guard locker(mutex);
		acceptPtr = boost::shared_ptr<TCPSocketObject>();
	}	

	return newsock;
}

bool TCPServerSocketObject::setSocketTimeout(uint32_t recvTimeout,uint32_t sendTimeout)
{
	if(getHandle() == -1)
	{
		return false;
	}

	int ret = setsockopt(getHandle(),SOL_SOCKET,SO_SNDTIMEO,(const char*)&sendTimeout,sizeof(sendTimeout));
	ret |= setsockopt(getHandle(),SOL_SOCKET,SO_RCVTIMEO,(const char*)&recvTimeout,sizeof(recvTimeout));

	return ret >= 0;
}

bool TCPServerSocketObject::getSocketTimeout(uint32_t& recvTimeout,uint32_t& sendTimeout)
{
	if(getHandle() == -1)
	{
		return false;
	}

	socklen_t sendlen = sizeof(sendTimeout),recvlen = sizeof(recvTimeout);

	int ret = getsockopt(getHandle(),SOL_SOCKET,SO_SNDTIMEO,(char*)&sendTimeout,&sendlen);
	ret |= getsockopt(getHandle(),SOL_SOCKET,SO_RCVTIMEO,(char*)&recvTimeout,&recvlen);

	return ret == 0;
}
bool TCPServerSocketObject::setSocketOpt(int level, int optname, const void *optval, int optlen)
{
	if (getHandle() == -1)
	{
		return false;
	}
	return ::setsockopt(getHandle(), level, optname, (const char*)optval, optlen) >= 0;
}
bool TCPServerSocketObject::getSocketOpt(int level, int optname, void *optval, int *optlen) const
{
	boost::shared_ptr<boost::asio::ip::tcp::acceptor> acceptptr;
	{
		acceptptr = acceptorServer;
	}
	if (acceptptr == NULL)
	{
		return 0;
	}

	return ::getsockopt(acceptptr->native_handle(), level, optname, (char*)optval, (socklen_t*)optlen) >= 0;
}
bool TCPServerSocketObject::nonBlocking(bool nonblock)
{
	Guard locker(mutex);
	if(acceptorServer == NULL)
	{
		return false;
	}

	try
	{
		boost::asio::ip::tcp::socket::non_blocking_io nonio(nonblock);
		acceptorServer->io_control(nonio);  
	}
	catch(const std::exception& e)
	{
		logerror("%s %d std::exception %s\r\n",__FUNCTION__,__LINE__,e.what());
		return false;
	}

	return true;
}
}
}