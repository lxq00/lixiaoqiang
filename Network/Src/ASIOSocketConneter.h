#ifndef __ASIOSOCKET_TCPOBJCET_H__
#define __ASIOSOCKET_TCPOBJCET_H__
#include "ASIOSocketObject.h"
#include "Network/TcpClient.h"
namespace Public{
namespace Network{

///TCP的socket处理
class ASIOSocketConneter:public ASIOSocketObject<boost::asio::ip::tcp, boost::asio::ip::tcp::socket>
{
public:
	ASIOSocketConneter(const shared_ptr<IOWorker>& worker) :ASIOSocketObject<boost::asio::ip::tcp, boost::asio::ip::tcp::socket>(worker,NetType_TcpClient)
	{
		nodelay();
	}
	virtual ~ASIOSocketConneter() {}
	bool create()
	{
		shared_ptr<boost::asio::ip::tcp::socket> sockptr = sock;
		if (sockptr == NULL)
		{
			return false;
		}

		try
		{
			sockptr->open(boost::asio::ip::tcp::v4());
		}
		catch (const std::exception& e)
		{
			logdebug("%s %d sockptr->open std::exception %s\r\n", __FUNCTION__, __LINE__, e.what());
			return false;
		}

		return true;
	}
	bool startConnect(const Socket::ConnectedCallback& callback, const NetAddr& otherpoint)
	{
		shared_ptr<boost::asio::ip::tcp::socket> sockptr = sock;
		if(sockptr == NULL)
		{
			return false;
		}

		boost::asio::ip::tcp::endpoint connectToEndpoint = boost::asio::ip::tcp::endpoint(boost::asio::ip::address::from_string(otherpoint.getIP()), otherpoint.getPort());

		boost::shared_ptr<ConnectCallbackObject> connectobj = boost::make_shared<ConnectCallbackObject>(sockobjptr, userthread, callback);
		try
		{
			sockptr->async_connect(connectToEndpoint, boost::bind(&ConnectCallbackObject::_connectCallbackPtr, connectobj, boost::asio::placeholders::error));

		}
		catch (const std::exception& e)
		{
			logdebug("%s %d sockptr->async_connect std::exception %s\r\n", __FUNCTION__, __LINE__, e.what());
			return false;
		}

		return true;
	}

	bool connect(const NetAddr& otherpoint)
	{
		shared_ptr<boost::asio::ip::tcp::socket> sockptr = sock;
		if (sockptr == NULL)
		{
			return false;
		}

		boost::asio::ip::tcp::endpoint connectToEndpoint = boost::asio::ip::tcp::endpoint(boost::asio::ip::address::from_string(otherpoint.getIP()), otherpoint.getPort());

		boost::system::error_code er;

		boost::system::error_code ret = sockptr->connect(connectToEndpoint, er);

		if (!er && !ret)
		{
			status = NetStatus_connected;
		}

		return  (!er && !ret) ? true : false;
	}
	void nodelay()
	{
		shared_ptr<boost::asio::ip::tcp::socket> sockptr = sock;
		if (sockptr == NULL)
		{
			return;
		}

		//取消TCP的40ms的延时
		boost::asio::ip::tcp::no_delay option(true);
		sockptr->set_option(option);
	}
	int send(const char* buffer, int len)
	{
		shared_ptr<boost::asio::ip::tcp::socket> sockptr = sock;
		if (sockptr == NULL)
		{
			return false;
		}

		boost::system::error_code er;

		int sendlen = (int)sockptr->send(boost::asio::buffer(buffer, len), 0, er);

		return !er ? sendlen : -1;
	}

	int recv(char* buffer, int maxlen)
	{
		shared_ptr<boost::asio::ip::tcp::socket> sockptr = sock;
		if (sockptr == NULL)
		{
			return false;
		}

		boost::system::error_code er;

		int recvlen = (int)sockptr->receive(boost::asio::buffer(buffer, maxlen), 0, er);

		if (er.value() == boost::asio::error::eof || er.value() == boost::asio::error::connection_reset)
		{
			setStatus(NetStatus_disconnected,er.message());
		}

		return !er ? recvlen : -1;
	}
	bool async_send(const char* buf, uint32_t len, const Socket::SendedCallback& sended)
	{
		shared_ptr<boost::asio::ip::tcp::socket> sockptr = sock;
		if (sockptr == NULL)
		{
			return false;
		}

		boost::shared_ptr<SendCallbackObject> sendobj = boost::make_shared<SendCallbackObject>(sockobjptr, userthread, sended, buf, len);

		try
		{
			sockptr->async_write_some(boost::asio::buffer(buf, len), boost::bind(&SendCallbackObject::_sendCallbackPtr, sendobj, boost::asio::placeholders::error, boost::asio::placeholders::bytes_transferred));

		}
		catch (const std::exception& e)
		{
			logdebug("%s %d sockptr->async_write_some std::exception %s\r\n", __FUNCTION__, __LINE__, e.what());

			return false;
		}

		return true;
	}
	bool async_recv(boost::shared_ptr<RecvCallbackObject>& recvobj)
	{
		shared_ptr<boost::asio::ip::tcp::socket> sockptr = sock;
		if (sockptr == NULL)
		{
			return false;
		}

		try
		{
			sockptr->async_read_some(boost::asio::buffer(recvobj->bufferaddr, recvobj->bufferlen),
				boost::bind(&RecvCallbackObject::_recvCallbackPtr, recvobj, boost::asio::placeholders::error, boost::asio::placeholders::bytes_transferred));

		}
		catch (const std::exception& e)
		{
			logdebug("%s %d sockptr->async_read_some std::exception %s\r\n", __FUNCTION__, __LINE__, e.what());

			return false;
		}

		return true;
	}
	virtual bool async_recv(char* buf, uint32_t len, const Socket::ReceivedCallback& received)
	{
		boost::shared_ptr<RecvCallbackObject> recvobj(new RecvCallbackObject(sockobjptr, userthread, received, buf, len));

		return async_recv(recvobj);
	}
	virtual bool async_recv(const Socket::ReceivedCallback& received, int maxlen = 1024)
	{
		boost::shared_ptr<RecvCallbackObject> recvobj(new RecvCallbackObject(sockobjptr, userthread, received, NULL, maxlen));

		return async_recv(recvobj);
	}
};
struct TCPClient::TCPClientInternalPointer:public ASIOSocketConneter
{
	TCPClientInternalPointer(const shared_ptr<IOWorker>& worker) :ASIOSocketConneter(worker) { create(); }
};


}
}


#endif //__ASIOSOCKET_OBJCET_H__
