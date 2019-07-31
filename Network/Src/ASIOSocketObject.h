#ifndef __ASIOSOCKET_OBJCET_H__
#define __ASIOSOCKET_OBJCET_H__
#include "ASIOSocketDefine.h"


namespace Public{
namespace Network{

template<typename SocketType>
struct SocketOthreadAddrObject
{
	static NetAddr getothearaddr(const shared_ptr<SocketType> & sock) { return NetAddr(); }
};

template<>
struct SocketOthreadAddrObject<boost::asio::ip::tcp::socket>
{
	static NetAddr getothearaddr(const shared_ptr<boost::asio::ip::tcp::socket> & sock)
	{
		try
		{
			NetAddr otheraddr(sock->remote_endpoint().address().to_v4().to_string(), sock->remote_endpoint().port());

			return otheraddr;
		}
		catch (const std::exception& e)
		{
			logdebug("%s %d sockptr->remote_endpoint() std::exception %s\r\n", __FUNCTION__, __LINE__, e.what());
		}

		return NetAddr();
	}
};

///asio socket操作的对象，该对象基类
template<typename SocketProtocol,typename SocketType>
class ASIOSocketObject:public Socket,public enable_shared_from_this<ASIOSocketObject<SocketProtocol,SocketType> >
{
public:
	ASIOSocketObject(const shared_ptr<IOWorker>& _worker,NetType _type) :worker(_worker),status(NetStatus_notconnected),type(_type)
	{
		userthread = make_shared<UserThreadInfo>();
		if (worker == NULL) worker = IOWorker::defaultWorker();
		sock = make_shared<SocketType>(*(boost::asio::io_service*)worker->getBoostASIOIOServerPtr());
	}
	virtual ~ASIOSocketObject() {}
	void initSocketptr(const shared_ptr<Socket>& sock)
	{
		sockobjptr = sock;
	}
	virtual bool create() { return true; }
	void nodelay()
	{
		shared_ptr<SocketType> sockptr = sock;
		if (sockptr == NULL)
		{
			return;
		}
		//取消TCP的40ms的延时
		int enable = 1;
		setsockopt(sockptr->native_handle(), IPPROTO_TCP, TCP_NODELAY, (const char*)&enable, sizeof(enable));
	}
	bool setDisconnectCallback(const Socket::DisconnectedCallback& callback) { disconnectCallback = callback; return true; }
	///断开socket
	///true表示可以正常关闭该对象，false表示需要另外线程来释放该对象
	virtual bool disconnect()
	{
		shared_ptr<SocketType> sockptr = sock;
		sock = NULL;
		sockobjptr = weak_ptr<Socket>();
		
		if (userthread)
		{
			userthread->quit();
			///现在关闭应用，做相应的关闭回调处理
			userthread->waitAllOtherCallbackThreadUsedEnd();
		}
		
		userthread = NULL;

		if (sockptr != NULL)
		{
			sockptr->close();
		}

		return true;
	}

	virtual SOCKET getHandle() const
	{
		shared_ptr<SocketType> sockptr = sock;
		if (sockptr == NULL)
		{
			return 0;
		}

		return sockptr->native_handle();
	}

	bool bind(const NetAddr& point, bool reusedaddr)
	{
		shared_ptr<SocketType> sockptr = sock;
		if (sockptr == NULL)
		{
			return false;
		}

		try
		{
			typename SocketProtocol::endpoint sockendpoint(point.getType() == NetAddr::netaddr_ipv4 ? SocketProtocol::v4() : SocketProtocol::v6(), point.getPort());

			sockptr->set_option(boost::asio::ip::tcp::acceptor::reuse_address(reusedaddr));
			sockptr->bind(sockendpoint);
		}
		catch (const std::exception& e)
		{
			logdebug("%s %d sockptr->bind std::exception %s\r\n", __FUNCTION__, __LINE__, e.what());
			return false;
		}

		return true;
	}

	bool nonBlocking(bool nonblock)
	{
		shared_ptr<SocketType> sockptr = sock;
		if (sockptr == NULL)
		{
			return 0;
		}

		try
		{
			typename SocketProtocol::socket::non_blocking_io nonio(nonblock);
			sockptr->io_control(nonio);
		}
		catch (const std::exception& e)
		{
			logerror("%s %d std::exception %s\r\n", __FUNCTION__, __LINE__, e.what());
			return false;
		}

		return true;
	}

	NetType getNetType() const { return type; }
	NetStatus getStatus() const {return status;}

	virtual void setStatus(NetStatus _status){status = _status;}

	bool setSocketTimeout(uint32_t recvTimeout, uint32_t sendTimeout)
	{
		SOCKET sock = getHandle();
		if (sock == -1)
		{
			return false;
		}

		int ret = setsockopt(sock, SOL_SOCKET, SO_SNDTIMEO, (const char*)&sendTimeout, sizeof(sendTimeout));
		ret |= setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, (const char*)&recvTimeout, sizeof(recvTimeout));

		return ret >= 0;
	}
	bool getSocketTimeout(uint32_t& recvTimeout, uint32_t& sendTimeout)
	{
		SOCKET sock = getHandle();
		if (sock == -1)
		{
			return false;
		}

		socklen_t sendlen = sizeof(sendTimeout), recvlen = sizeof(recvTimeout);

		int ret = getsockopt(sock, SOL_SOCKET, SO_SNDTIMEO, (char*)&sendTimeout, (socklen_t*)&sendlen);
		ret |= getsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, (char*)&recvTimeout, (socklen_t*)&recvlen);

		return ret == 0;
	}

	bool setSocketOpt(int level, int optname, const void *optval, int optlen)
	{
		SOCKET sock = getHandle();
		if (sock == -1)
		{
			return false;
		}

		return ::setsockopt(sock, level, optname, (const char*)optval, optlen) >= 0;
	}
	bool getSocketOpt(int level, int optname, void *optval, int *optlen) const
	{
		SOCKET sock = getHandle();
		if (sock == -1)
		{
			return false;
		}

		return ::getsockopt(sock, level, optname, (char*)optval, (socklen_t*)optlen) >= 0;
	}
	NetAddr getMyAddr() const
	{
		shared_ptr<SocketType> sockptr = sock;
		if (sockptr == NULL)
		{
			return NetAddr();
		}

		try
		{
			NetAddr myaddr(sockptr->local_endpoint().address().to_v4().to_string(), sockptr->local_endpoint().port());

			return myaddr;
		}
		catch (const std::exception& e)
		{
			logdebug("%s %d sockptr->local_endpoint() std::exception %s\r\n", __FUNCTION__, __LINE__, e.what());
		}

		return NetAddr();
	}
	NetAddr getOtherAddr() const
	{
		shared_ptr<SocketType> sockptr = sock;
		if (sockptr == NULL)
		{
			return NetAddr(); 
		}

		return SocketOthreadAddrObject<SocketType>::getothearaddr(sockptr);
	}
	void socketErrorCallback(const boost::system::error_code& er)
	{
		NetStatus curstatus;
		{
			curstatus = status;
			status = NetStatus_notconnected;
		}
		if (curstatus == NetStatus_connected)
		{
			shared_ptr<Socket> _sockptr = sockobjptr.lock();
			if(_sockptr != NULL) disconnectCallback(_sockptr, er.message());
		}
	}
	virtual void acceptErrorCallback(const boost::system::error_code& er, const Socket::AcceptedCallback& callback) {}
protected:
	shared_ptr<IOWorker>							worker;
	shared_ptr<UserThreadInfo>						userthread;


	NetType											type;				///socket类型
	NetStatus										status;				///socket状态
	weak_ptr<Socket>								sockobjptr;
	Socket::DisconnectedCallback					disconnectCallback;	///断开回调通知
public:
	shared_ptr<SocketType>							sock;
};

}
}


#endif //__ASIOSOCKET_OBJCET_H__
