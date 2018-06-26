#ifndef __ASIOSOCKET_ACCEPTOBJCET_H__
#define __ASIOSOCKET_ACCEPTOBJCET_H__
#include "ASIOSocketObject.h"
#include "Network/TcpClient.h"
#include "ASIOSocketConneter.h"
namespace Public{
namespace Network{
class ASIOSocketAcceptor:public ASIOSocketObject<boost::asio::ip::tcp, boost::asio::ip::tcp::acceptor>
{
public:
	ASIOSocketAcceptor(const shared_ptr<IOWorker>& _worker):ASIOSocketObject<boost::asio::ip::tcp, boost::asio::ip::tcp::acceptor>(_worker,NetType_TcpServer){}
	~ASIOSocketAcceptor() {}

	virtual bool create(const NetAddr& point, bool reusedaddr)
	{
		Guard locker(mutex);

		if (!point.isValid())
		{
			return false;
		}
		listenAddr = point;


		try
		{
			boost::asio::ip::tcp::endpoint bindaddr = boost::asio::ip::tcp::endpoint(point.getType() == NetAddr::netaddr_ipv4 ? boost::asio::ip::tcp::v4() : boost::asio::ip::tcp::v6(), point.getPort());

			sock = boost::shared_ptr<boost::asio::ip::tcp::acceptor>(new boost::asio::ip::tcp::acceptor(**(boost::shared_ptr<boost::asio::io_service>*)worker->getBoostASIOIOServerSharedptr()));
			sock->open(bindaddr.protocol());
			sock->set_option(boost::asio::ip::tcp::acceptor::reuse_address(reusedaddr));
			sock->bind(bindaddr);
			sock->listen();
		}
		catch (const std::exception& e)
		{
			logdebug("%s %d std::exception %s reusedaddr %d port %d\r\n", __FUNCTION__, __LINE__, e.what(), reusedaddr, listenAddr.getPort());
			return false;
		}

		return true;
	}

	bool startListen(const Socket::AcceptedCallback& accepted)
	{
		{
			Guard locker(mutex);
			if (sock == NULL || mustQuit || acceptCallback != NULL)
			{
				return false;
			}
			acceptCallback = accepted;
		}


		return startDoAccept();
	}

	shared_ptr<Socket> accept()
	{
		shared_ptr<TCPClient> acceptsock = shared_ptr<TCPClient>(new TCPClient(worker));

		boost::shared_ptr<boost::asio::ip::tcp::acceptor> acceptPtr;
		{
			Guard locker(mutex);
			if (sock == NULL || mustQuit || acceptsock != NULL)
			{
				return NULL;
			}
			acceptPtr = sock;
		}

		boost::system::error_code er;

		boost::asio::ip::tcp::socket& newsockptr = *(boost::asio::ip::tcp::socket*)acceptsock->tcpclientinternal->sock->sock.get();
		boost::system::error_code re = acceptPtr->accept(newsockptr, er);

		
		if (!re && !er)
		{
			acceptsock->tcpclientinternal->sock->setStatus(NetStatus_connected);
			acceptsock->tcpclientinternal->sock->nonBlocking(true);
			acceptsock->tcpclientinternal->sock->nodelay();
		}
		else
		{
			acceptsock = NULL;
		}

		return acceptsock;
	}
private:
	virtual bool doStartSend(boost::shared_ptr<SendInternal>& internal) { return false; }
	virtual bool doPostRecv(boost::shared_ptr<RecvInternal>& internal) { return false; }
	void recreate()
	{
		disconnect();
		create(listenAddr, true);
	}
	void _acceptCallbackPtr(const boost::system::error_code& er)
	{
		shared_ptr<TCPClient> socketobj;
		Socket::AcceptedCallback callback;
		{
			Guard locker(mutex);
			if (sock == NULL || mustQuit)
			{
				return;
			}
			socketobj = acceptSocket;
			acceptSocket = NULL;
			callback = acceptCallback;

			_callbackThreadUsedStart();
		}
		if (!er && socketobj != NULL && callback != NULL)
		{
			socketobj->tcpclientinternal->sock->setStatus(NetStatus_connected);
			socketobj->tcpclientinternal->sock->nonBlocking(true);
			socketobj->tcpclientinternal->sock->nodelay();

			shared_ptr<Socket> _sockptr = sockobjptr.lock();
			if (_sockptr != NULL) callback(_sockptr, socketobj);
		}
		callbackThreadUsedEnd();

		if (er)
		{
			///socket出异常了，重置下socket
			recreate();
		}

		startDoAccept();
	}
	bool startDoAccept()
	{
		shared_ptr<TCPClient> socketobj;
		boost::shared_ptr<boost::asio::ip::tcp::acceptor> accepetserptr;
		{
			Guard locker(mutex);
			if (sock == NULL || mustQuit || acceptCallback == NULL || acceptSocket != NULL)
			{
				return false;
			}
			socketobj = acceptSocket = shared_ptr<TCPClient>(new TCPClient(worker));
			accepetserptr = sock;
		}

		try
		{
			boost::asio::ip::tcp::socket& newsockptr = *(boost::asio::ip::tcp::socket*)socketobj->tcpclientinternal->sock->sock.get();
			accepetserptr->async_accept(newsockptr, boost::bind(&ASIOSocketObject::_acceptCallbackPtr, ASIOSocketAcceptor::shared_from_this(), boost::asio::placeholders::error));

		}
		catch (const std::exception& e)
		{
			logdebug("%s %d accepetserptr->async_accept std::exception %s\r\n", __FUNCTION__, __LINE__, e.what());
			return false;
		}

		return true;
	}
public:
	Socket::AcceptedCallback								acceptCallback;
	shared_ptr<TCPClient>									acceptSocket;
	NetAddr													listenAddr;
};

}
}


#endif //__ASIOSOCKET_ACCEPTOBJCET_H__
