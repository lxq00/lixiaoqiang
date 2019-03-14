#ifndef __ASIOSOCKET_TCPOBJCET_H__
#define __ASIOSOCKET_TCPOBJCET_H__
#include "ASIOSocketObject.h"
#include "Network/TcpClient.h"
namespace Public{
namespace Network{

///TCP��socket����
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
		boost::shared_ptr<boost::asio::ip::tcp::socket> sockptr;
		{
			sockptr = sock;
		}
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
	virtual bool disconnect()
	{
		checkTimer = NULL;
		return ASIOSocketObject<boost::asio::ip::tcp, boost::asio::ip::tcp::socket>::disconnect();
	}
	bool startConnect(const Socket::ConnectedCallback& callback, const NetAddr& otherpoint)
	{
		{
			Guard locker(mutex);
			if (mustQuit || status == NetStatus_connected || connectCallback != NULL || !otherpoint.isValid())
			{
				return false;
			}
			connectCallback = callback;
			connectToEndpoint = boost::asio::ip::tcp::endpoint(boost::asio::ip::address::from_string(otherpoint.getIP()), otherpoint.getPort());
		}

		return doStartConnect();
	}

	bool connect(const NetAddr& otherpoint)
	{
		boost::shared_ptr<boost::asio::ip::tcp::socket> sockptr;
		{
			Guard locker(mutex);
			if (sock == NULL || mustQuit || status == NetStatus_connected)
			{
				return false;
			}
			sockptr = sock;
			connectToEndpoint = boost::asio::ip::tcp::endpoint(boost::asio::ip::address::from_string(otherpoint.getIP()), otherpoint.getPort());
		}
		boost::system::error_code er;

		boost::system::error_code ret = sockptr->connect(connectToEndpoint, er);

		if (!er && !ret)
		{
			Guard locker(mutex);
			status = NetStatus_connected;
		}

		return  (!er && !ret) ? true : false;
	}
	int send(const char* buffer, int len)
	{
		boost::shared_ptr<boost::asio::ip::tcp::socket> sockptr;
		{
			Guard locker(mutex);
			if (sock == NULL || mustQuit || status != NetStatus_connected)///���socket�Ѿ����رգ�ֹͣ
			{
				return -1;
			}
			sockptr = sock;
		}
		boost::system::error_code er;

		int sendlen = sockptr->send(boost::asio::buffer(buffer, len), 0, er);

		return !er ? sendlen : -1;
	}

	int recv(char* buffer, int maxlen)
	{
		boost::shared_ptr<boost::asio::ip::tcp::socket> sockptr;
		{
			Guard locker(mutex);
			if (sock == NULL || mustQuit || status != NetStatus_connected)///���socket�Ѿ����رգ�ֹͣ
			{
				return -1;
			}
			sockptr = sock;
		}
		boost::system::error_code er;

		int recvlen = sockptr->receive(boost::asio::buffer(buffer, maxlen), 0, er);

		if (er.value() == boost::asio::error::eof || er.value() == boost::asio::error::connection_reset)
		{
			socketDisconnect(er.message().c_str());
		}

		return !er ? recvlen : -1;
	}
	bool doStartSend(boost::shared_ptr<SendInternal>& internal)
	{
		boost::shared_ptr<boost::asio::ip::tcp::socket> sockptr;
		{
			Guard locker(mutex);
			if (sock == NULL || internal == NULL || mustQuit || status != NetStatus_connected)///���socket�Ѿ����رգ�ֹͣ
			{
				return false;
			}
			sockptr = sock;
		}

		try
		{
			sockptr->async_write_some(boost::asio::buffer(internal->getSendBuffer(), internal->getSendBufferLen()), boost::bind(&ASIOSocketObject::_sendCallbackPtr, shared_from_this(), boost::asio::placeholders::error, boost::asio::placeholders::bytes_transferred));

		}
		catch (const std::exception& e)
		{
			logdebug("%s %d sockptr->async_write_some std::exception %s\r\n", __FUNCTION__, __LINE__, e.what());

			return false;
		}

		return true;
	}
	bool doPostRecv(boost::shared_ptr<RecvInternal>& internal)
	{
		boost::shared_ptr<boost::asio::ip::tcp::socket> sockptr;
		{
			Guard locker(mutex);
			if (sock == NULL || internal == NULL || mustQuit || status != NetStatus_connected)///���socket�Ѿ����رգ�ֹͣ
			{
				return false;
			}
			sockptr = sock;
		}

		try
		{
			sockptr->async_read_some(boost::asio::buffer(internal->getRecvBuffer(), internal->getRecvBufferLen()),
				boost::bind(&ASIOSocketObject::_recvCallbackPtr, shared_from_this(), boost::asio::placeholders::error, boost::asio::placeholders::bytes_transferred));

		}
		catch (const std::exception& e)
		{
			logdebug("%s %d sockptr->async_read_some std::exception %s\r\n", __FUNCTION__, __LINE__, e.what());

			return false;
		}

		return true;
	}
	void doStartSendData()
	{
		boost::shared_ptr<SendInternal> needSendData;

		{
			Guard locker(mutex);
			if (mustQuit || status != NetStatus_connected)
			{
				return;
			}
			if (currSendInternal != NULL)
			{
				///��ǰ���˷�������
				return;
			}
			if (sendInternalList.size() <= 0)
			{
				///��ǰû���ݿ��Է���
				return;
			}
			needSendData = sendInternalList.front();
			currSendInternal = needSendData;
		}

		bool ret = doStartSend(needSendData);
		if (!ret)
		{
			///���������������ʧ��,�����ǰ����
			_sendCallbackPtr(boost::system::error_code(), -1);
		}
	}
	bool doStartConnect()
	{
		boost::shared_ptr<boost::asio::ip::tcp::socket> sockptr;
		{
			Guard locker(mutex);
			if (sock == NULL || mustQuit || status == NetStatus_connected || connectCallback == NULL)
			{
				return false;
			}
			sockptr = sock;
		}

		try
		{
			sockptr->async_connect(connectToEndpoint, boost::bind(&ASIOSocketObject::_connectCallbackPtr, ASIOSocketObject::shared_from_this(), boost::asio::placeholders::error));

		}
		catch (const std::exception& e)
		{
			logdebug("%s %d sockptr->async_connect std::exception %s\r\n", __FUNCTION__, __LINE__, e.what());
			return false;
		}

		return true;
	}

	void _connectCallbackPtr(const boost::system::error_code& er)
	{
		NetStatus currStatus = NetStatus_connected;
		{
			Guard locker(mutex);
			if (mustQuit || status == NetStatus_connected)///���socket�Ѿ����رգ�ֹͣ
			{
				return;
			}

			///��¼��ǰ�ص��Ĵ�����Ϣ
			_callbackThreadUsedStart();

			if (!er)
			{
				currStatus = status;
				status = NetStatus_connected;
			}
		}
		if (!er && currStatus == NetStatus_notconnected)
		{
			checkTimer = boost::shared_ptr<Timer>();
			connectCallback(sockobjptr);
		}
		///�����ǰ�ص��ļ�¼��Ϣ
		callbackThreadUsedEnd();
		if (er)
		{
			///����ʧ����Ҫ��������,ʹ�ö�ʱ������������
			if (checkTimer == NULL)
			{
				checkTimer = boost::make_shared<Timer>("TCPSocketObject");
				checkTimer->start(Timer::Proc(&ASIOSocketConneter::timerConnectProc, this), 0, 500);
			}
		}
	}
	void timerConnectProc(unsigned long param)
	{
		doStartConnect();
	}
	void timerCheckFirstRecvTimeoutProc(unsigned long param)
	{
		uint64_t nowTime = Time::getCurrentMilliSecond();

		if (nowTime - connectTime >= 10000) ///10��û���ܵ����ݶϿ�
		{
			checkTimer->stop();

			///�Ͽ�����֪ͨӦ�ò�
			disconnect();

			{
				Guard locker(mutex);
				if (mustQuit)///���socket�Ѿ����رգ�ֹͣ
				{
					return;
				}
				///��¼��ǰ�ص��Ĵ�����Ϣ
				_callbackThreadUsedStart();
			}
			disconnectCallback(sockobjptr, "accept�²�����socket10��δ���ܵ����ݣ��Ͽ�����");

			///�����ǰ�ص��ļ�¼��Ϣ
			callbackThreadUsedEnd();
		}
	}
private:
	boost::asio::ip::tcp::endpoint					connectToEndpoint;
	Socket::ConnectedCallback						connectCallback;

	boost::shared_ptr<Timer>								checkTimer;   ///��timer���ǺܺõĴ���ʽ��connect��accept��socket�������timer���е��˷ѣ���û�뵽���õķ�ʽ
	uint64_t										connectTime;
};
struct TCPClient::TCPClientInternalPointer
{
	boost::shared_ptr<ASIOSocketConneter> sock;

	void initSocketptr(const shared_ptr<Socket>& _sock)
	{
		sock->initSocketptr(_sock);
	}
};


}
}


#endif //__ASIOSOCKET_OBJCET_H__
