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

		boost::shared_ptr<AcceptCallbackObject> acceptobj = boost::make_shared<AcceptCallbackObject>(worker,sockobjptr, userthread, accepted);		
		try
		{
			shared_ptr<boost::asio::ip::tcp::socket> acceptsock = make_shared<boost::asio::ip::tcp::socket>(*(boost::asio::io_service*)worker->getBoostASIOIOServerPtr());

			accepetserptr->async_accept(*acceptsock, boost::bind(&AcceptCallbackObject::_acceptCallbackPtr, acceptobj, acceptsock,_1));

		}
		catch (const std::exception& e)
		{
			logdebug("%s %d accepetserptr->async_accept std::exception %s\r\n", __FUNCTION__, __LINE__, e.what());
			return false;
		}

		return true;
	}
	shared_ptr<Socket> accept()
	{
		shared_ptr<boost::asio::ip::tcp::acceptor> accepetserptr = sock;
		if (accepetserptr == NULL) return false;

		shared_ptr< boost::asio::ip::tcp::socket > acceptsock = make_shared<boost::asio::ip::tcp::socket>(*(boost::asio::io_service*)worker->getBoostASIOIOServerPtr());

		boost::system::error_code er;
		boost::system::error_code re = accepetserptr->accept(*acceptsock, er);


		shared_ptr<Socket> newsock = TCPClient::create(worker,&acceptsock);
		if (!re && !er)
		{
			newsock->socketReady();
		}
		else
		{
			newsock = NULL;
		}

		return newsock;
	}
public:
	NetAddr													listenAddr;
};

}
}


#endif //__ASIOSOCKET_ACCEPTOBJCET_H__
