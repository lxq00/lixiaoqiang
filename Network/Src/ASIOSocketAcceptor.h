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
		if (!point.isValid())
		{
			return false;
		}
		listenAddr = point;


		try
		{
			boost::asio::ip::tcp::endpoint bindaddr = boost::asio::ip::tcp::endpoint(point.getType() == NetAddr::netaddr_ipv4 ? boost::asio::ip::tcp::v4() : boost::asio::ip::tcp::v6(), point.getPort());

			sock = shared_ptr<boost::asio::ip::tcp::acceptor>(new boost::asio::ip::tcp::acceptor(*(boost::asio::io_service*)worker->getBoostASIOIOServerPtr()));
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

	bool async_accept(const Socket::AcceptedCallback& accepted)
	{
		shared_ptr<boost::asio::ip::tcp::acceptor> accepetserptr = sock;
		if (accepetserptr == NULL) return false;

		boost::shared_ptr<AcceptCallbackObject> acceptobj = boost::make_shared<AcceptCallbackObject>(sockobjptr, userthread, accepted);

		{
			acceptobj->newsock = shared_ptr<TCPClient>(new TCPClient(worker));
			acceptobj->newsock->tcpclientinternal->initSocketptr(acceptobj->newsock);
		}
		
		try
		{
			boost::asio::ip::tcp::socket& newsockptr = *(boost::asio::ip::tcp::socket*)acceptobj->newsock->tcpclientinternal->sock.get();
			accepetserptr->async_accept(newsockptr, boost::bind(&AcceptCallbackObject::_acceptCallbackPtr, acceptobj, boost::asio::placeholders::error));

		}
		catch (const std::exception& e)
		{
			logdebug("%s %d accepetserptr->async_accept std::exception %s\r\n", __FUNCTION__, __LINE__, e.what());
			return false;
		}

		return true;
	}
	void acceptErrorCallback(const boost::system::error_code& er, const Socket::AcceptedCallback& callback)
	{
		disconnect();
		create(listenAddr, true);

		async_accept(callback);
	}
	shared_ptr<Socket> accept()
	{
		shared_ptr<boost::asio::ip::tcp::acceptor> accepetserptr = sock;
		if (accepetserptr == NULL) return false;

		shared_ptr<TCPClient> acceptsock = shared_ptr<TCPClient>(new TCPClient(worker));

		boost::system::error_code er;

		boost::asio::ip::tcp::socket& newsockptr = *(boost::asio::ip::tcp::socket*)acceptsock->tcpclientinternal->sock.get();
		boost::system::error_code re = accepetserptr->accept(newsockptr, er);

		
		if (!re && !er)
		{
			acceptsock->socketReady();
		}
		else
		{
			acceptsock = NULL;
		}

		return acceptsock;
	}
public:
	NetAddr													listenAddr;
};

}
}


#endif //__ASIOSOCKET_ACCEPTOBJCET_H__
