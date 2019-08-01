#ifndef __ASIOSOCKET_UDPOBJCET_H__
#define __ASIOSOCKET_UDPOBJCET_H__
#include "ASIOSocketObject.h"

namespace Public{
namespace Network{

///UDPµÄsocket´¦Àí
class ASIOSocketUDP :public ASIOSocketObject<boost::asio::ip::udp, boost::asio::ip::udp::socket>
{
public:
	ASIOSocketUDP(const shared_ptr<IOWorker>& worker) :ASIOSocketObject<boost::asio::ip::udp, boost::asio::ip::udp::socket>(worker,NetType_Udp)
	{
		setStatus(NetStatus_connected);
	}
	virtual ~ASIOSocketUDP() {}
	int sendto(const char* buffer, int len, const NetAddr& otheraddr)
	{
		shared_ptr<boost::asio::ip::udp::socket> sockptr = sock;
		if(sockptr == NULL)
		{
			return false;
		}

		boost::system::error_code er;

		boost::asio::ip::udp::endpoint otehrpoint = boost::asio::ip::udp::endpoint(boost::asio::ip::address::from_string(otheraddr.getIP()), otheraddr.getPort());

		int sendlen = (int)sockptr->send_to(boost::asio::buffer(buffer, len), otehrpoint, 0, er);

		return !er ? sendlen : -1;
	}
	bool setSocketBuffer(uint32_t recvSize, uint32_t sendSize)
	{
		shared_ptr<boost::asio::ip::udp::socket> sockptr = sock;
		if (sockptr == NULL)
		{
			return false;
		}

		boost::asio::socket_base::send_buffer_size sndsize_option(sendSize);
		sockptr->set_option(sndsize_option);

		boost::asio::socket_base::receive_buffer_size recvsize_option(recvSize);
		sockptr->set_option(recvsize_option);

		return true;
	}
	int recvfrom(char* buffer, int maxlen, NetAddr& otheraddr)
	{
		shared_ptr<boost::asio::ip::udp::socket> sockptr = sock;
		if (sockptr == NULL)
		{
			return false;
		}

		boost::system::error_code er;

		boost::asio::ip::udp::endpoint otehrpoint;

		int recvlen = (int)sockptr->receive_from(boost::asio::buffer(buffer, maxlen), otehrpoint, 0, er);

		if (!er)
		{
			try
			{
				otheraddr = NetAddr(otehrpoint.address().to_v4().to_string(), otehrpoint.port());
			}
			catch (const std::exception& e)
			{
				logdebug("%s %d acceptorServer->local_endpoint() std::exception %s\r\n", __FUNCTION__, __LINE__, e.what());
			}
		}

		return !er ? recvlen : -1;
	}
	virtual bool async_recvfrom(boost::shared_ptr<RecvCallbackObject>& recvobj)
	{
		shared_ptr<boost::asio::ip::udp::socket> sockptr = sock;
		if (sockptr == NULL)
		{
			return false;
		}

		try
		{
			sockptr->async_receive_from(boost::asio::buffer(recvobj->bufferaddr, recvobj->bufferlen), recvobj->recvpoint,
				boost::bind(&RecvCallbackObject::_recvCallbackPtr, recvobj, boost::asio::placeholders::error, boost::asio::placeholders::bytes_transferred));

		}
		catch (const std::exception& e)
		{
			logdebug("%s %d sockptr->async_receive_from std::exception %s\r\n", __FUNCTION__, __LINE__, e.what());

			return false;
		}

		return true;
	}
	virtual bool async_recvfrom(char* buf, uint32_t len, const Socket::RecvFromCallback& received)
	{
		boost::shared_ptr<RecvCallbackObject> recvobj = boost::make_shared<RecvCallbackObject>(
			sockobjptr,userthread,received,buf,len);

	
		return async_recvfrom(recvobj);
	}
	virtual bool async_recvfrom(const Socket::RecvFromCallback& received, int maxlen = 1024)
	{
		boost::shared_ptr<RecvCallbackObject> recvobj = boost::shared_ptr<RecvCallbackObject>(new RecvCallbackObject(
			sockobjptr, userthread, received, NULL, maxlen));


		return async_recvfrom(recvobj);
	}
	virtual bool async_sendto(const char* buf, uint32_t len, const NetAddr& toaddr, const Socket::SendedCallback& sended)
	{
		shared_ptr<boost::asio::ip::udp::socket> sockptr = sock;
		if (sockptr == NULL)
		{
			return false;
		}

		boost::shared_ptr<SendCallbackObject> sendobj = boost::make_shared<SendCallbackObject>(sockobjptr,
			userthread, sended, buf, len);
		
		try
		{
			boost::asio::ip::udp::endpoint otehrpoint = boost::asio::ip::udp::endpoint(boost::asio::ip::address::from_string(toaddr.getIP()), toaddr.getPort());

			sockptr->async_send_to(boost::asio::buffer(sendobj->bufferaddr, sendobj->bufferlen), otehrpoint,
				boost::bind(&SendCallbackObject::_sendCallbackPtr, sendobj, boost::asio::placeholders::error, boost::asio::placeholders::bytes_transferred));

		}
		catch (const std::exception& e)
		{
			logdebug("%s %d sockptr->async_send_to std::exception %s\r\n", __FUNCTION__, __LINE__, e.what());

			return false;
		}

		return true;
	}
};

}
}


#endif //__ASIOSOCKET_OBJCET_H__
