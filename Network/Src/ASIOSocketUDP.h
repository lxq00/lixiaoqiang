#ifndef __ASIOSOCKET_UDPOBJCET_H__
#define __ASIOSOCKET_UDPOBJCET_H__
#include "ASIOSocketObject.h"

namespace Public{
namespace Network{

///UDP的socket处理
class ASIOSocketUDP :public ASIOSocketObject<boost::asio::ip::udp, boost::asio::ip::udp::socket>
{
public:
	ASIOSocketUDP(const shared_ptr<IOWorker>& worker) :ASIOSocketObject<boost::asio::ip::udp, boost::asio::ip::udp::socket>(worker,NetType_Udp)
	{
		setStatus(NetStatus_connected);
	}
	virtual ~ASIOSocketUDP() {}
	bool create()
	{
		boost::shared_ptr<boost::asio::ip::udp::socket> sockptr;
		{
			sockptr = sock;
		}
		if (sockptr == NULL)
		{
			return false;
		}

		try
		{
			sockptr->open(boost::asio::ip::udp::v4());
		}
		catch (const std::exception& e)
		{
			logdebug("%s %d udpsock->open std::exception %s\r\n", __FUNCTION__, __LINE__, e.what());
			return false;
		}

		return true;
	}
	int sendto(const char* buffer, int len, const NetAddr& otheraddr)
	{
		boost::shared_ptr<boost::asio::ip::udp::socket> sockptr;
		{
			Guard locker(mutex);
			if (sock == NULL || mustQuit)///如果socket已经被关闭，停止
			{
				return false;
			}
			sockptr = sock;
		}

		boost::system::error_code er;

		boost::asio::ip::udp::endpoint otehrpoint = boost::asio::ip::udp::endpoint(boost::asio::ip::address::from_string(otheraddr.getIP()), otheraddr.getPort());

		int sendlen = sockptr->send_to(boost::asio::buffer(buffer, len), otehrpoint, 0, er);

		return !er ? sendlen : -1;
	}
	int recvfrom(char* buffer, int maxlen, NetAddr& otheraddr)
	{
		boost::shared_ptr<boost::asio::ip::udp::socket> sockptr;
		{
			Guard locker(mutex);
			if (sock == NULL || mustQuit)///如果socket已经被关闭，停止
			{
				return false;
			}
			sockptr = sock;
		}

		boost::system::error_code er;

		boost::asio::ip::udp::endpoint otehrpoint;

		int recvlen = sockptr->receive_from(boost::asio::buffer(buffer, maxlen), otehrpoint, 0, er);

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
	bool doStartSend(boost::shared_ptr<SendInternal>& internal)
	{
		boost::shared_ptr<boost::asio::ip::udp::socket> sockptr;

		{
			Guard locker(mutex);
			if (sock == NULL || internal == NULL || mustQuit || status != NetStatus_connected)///如果socket已经被关闭，停止
			{
				return false;
			}
			sockptr = sock;
		}

		try
		{
			const NetAddr& toaddr = internal->getSendAddr();

			boost::asio::ip::udp::endpoint otehrpoint = boost::asio::ip::udp::endpoint(boost::asio::ip::address::from_string(toaddr.getIP()), toaddr.getPort());

			sockptr->async_send_to(boost::asio::buffer(internal->getSendBuffer(), internal->getSendBufferLen()), otehrpoint,
				boost::bind(&ASIOSocketObject::_sendCallbackPtr, ASIOSocketObject::shared_from_this(), boost::asio::placeholders::error, boost::asio::placeholders::bytes_transferred));

		}
		catch (const std::exception& e)
		{
			logdebug("%s %d sockptr->async_send_to std::exception %s\r\n", __FUNCTION__, __LINE__, e.what());

			return false;
		}

		return true;
	}
	bool doPostRecv(boost::shared_ptr<RecvInternal>& internal)
	{
		boost::shared_ptr<boost::asio::ip::udp::socket> sockptr;
		{
			Guard locker(mutex);
			if (sock == NULL || internal == NULL || mustQuit || status != NetStatus_connected)///如果socket已经被关闭，停止
			{
				return false;
			}
			sockptr = sock;
		}

		try
		{
			sockptr->async_receive_from(boost::asio::buffer(internal->getRecvBuffer(), internal->getRecvBufferLen()), internal->getRecvPoint(),
				boost::bind(&ASIOSocketObject::_recvCallbackPtr, shared_from_this(), boost::asio::placeholders::error, boost::asio::placeholders::bytes_transferred));

		}
		catch (const std::exception& e)
		{
			logdebug("%s %d sockptr->async_receive_from std::exception %s\r\n", __FUNCTION__, __LINE__, e.what());

			return false;
		}

		return true;
	}
};

}
}


#endif //__ASIOSOCKET_OBJCET_H__
