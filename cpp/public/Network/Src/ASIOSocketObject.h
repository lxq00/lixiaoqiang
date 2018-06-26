#ifndef __ASIOSOCKET_OBJCET_H__
#define __ASIOSOCKET_OBJCET_H__
#include "ASIOSocketDefine.h"


namespace Public{
namespace Network{

///asio socket操作的对象，该对象基类
template<typename SocketProtocol,typename SocketType>
class ASIOSocketObject:public UserThreadInfo,public Socket,public boost::enable_shared_from_this<ASIOSocketObject<SocketProtocol,SocketType> >
{
public:
	ASIOSocketObject(const shared_ptr<IOWorker>& _worker,NetType _type) :worker(_worker),status(NetStatus_notconnected),type(_type)
	{
		if (worker == NULL) worker = IOWorker::defaultWorker();
		sock = boost::make_shared<SocketType>(**(boost::shared_ptr<boost::asio::io_service>*)worker->getBoostASIOIOServerSharedptr());
	}
	virtual ~ASIOSocketObject() {}
	void initSocketptr(const shared_ptr<Socket>& sock)
	{
		sockobjptr = sock;
	}
	virtual bool create() { return true; }
	void nodelay()
	{
		boost::shared_ptr<SocketType> sockptr;
		{
			sockptr = sock;
		}
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
		boost::shared_ptr<SocketType> sockptr;
		{
			sockptr = sock;
			sock = NULL;
			sockobjptr = weak_ptr<Socket>();
			quit();
		}
		///现在关闭应用，做相应的关闭回调处理
		waitAllOtherCallbackThreadUsedEnd();

		if (sockptr != NULL)
		{
			sockptr->close();
		}

		return true;
	}

	virtual int getHandle() const
	{
		boost::shared_ptr<SocketType> sockptr;
		{
			sockptr = sock;
		}
		if (sockptr == NULL)
		{
			return 0;
		}

		return sockptr->native_handle();
	}

	bool bind(const NetAddr& point, bool reusedaddr)
	{
		boost::shared_ptr<SocketType> sockptr;
		{
			sockptr = sock;
		}
		if (sockptr == NULL)
		{
			return false;
		}

		try
		{
			sockptr->set_option(boost::asio::ip::tcp::acceptor::reuse_address(reusedaddr));
			sockptr->bind(SocketProtocol::endpoint(point.getType() == NetAddr::netaddr_ipv4 ? SocketProtocol::v4() : SocketProtocol::v6(), point.getPort()));
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
		boost::shared_ptr<SocketType> sockptr;
		{
			sockptr = sock;
		}
		if (sockptr == NULL)
		{
			return 0;
		}

		try
		{
			SocketProtocol::socket::non_blocking_io nonio(nonblock);
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

	bool async_recv(char *buf, uint32_t len, const ReceivedCallback& received)
	{
		if (buf == NULL || len == 0 || !received)
		{
			return false;
		}

		boost::shared_ptr<RecvInternal> recvptr = boost::make_shared<TCPRecvInternal>(buf, len, received);

		return postReceive(recvptr);
	}
	bool async_recv(const ReceivedCallback& received, int maxlen)
	{
		if (maxlen == 0 || !received)
		{
			return false;
		}

		boost::shared_ptr<RecvInternal> recvptr = boost::make_shared<TCPRecvInternal>(maxlen, received);

		return postReceive(recvptr);
	}
	bool async_send(const char * buf, uint32_t len, const SendedCallback& sended)
	{
		if (buf == NULL || len == 0 || !sended)
		{
			return false;
		}
		boost::shared_ptr<SendInternal> sendptr = boost::make_shared<RepeatSendInternal>(buf, len, sended);

		return beginSend(sendptr);
	}

	bool async_recvfrom(char *buf, uint32_t len, const RecvFromCallback& received)
	{
		if (buf == NULL || len == 0 || !received)
		{
			return false;
		}
		boost::shared_ptr<RecvInternal> recvptr = boost::make_shared<udpRecvInternal>(buf, len, (RecvFromCallback&)received);

		return postReceive(recvptr);
	}
	bool async_recvfrom(const RecvFromCallback& received, int maxlen)
	{
		if (maxlen == 0 || !received)
		{
			return false;
		}
		boost::shared_ptr<RecvInternal> recvptr = boost::make_shared<udpRecvInternal>(maxlen, (RecvFromCallback&)received);

		return postReceive(recvptr);
	}
	bool async_sendto(const char * buf, uint32_t len, const NetAddr& other, const SendedCallback& sended)
	{
		if (buf == NULL || len == 0 || !sended)
		{
			return false;
		}
		boost::shared_ptr<SendInternal> sendptr = boost::make_shared<SendInternal>(buf, len, sended, other);

		return beginSend(sendptr);
	}

	virtual int send(const char* buffer, int len) { return -1; }
	virtual int recv(char* buffer, int maxlen) { return -1; }
	virtual int sendto(const char* buffer, int len, const NetAddr& otheraddr) { return -1; }
	virtual int recvfrom(char* buffer, int maxlen, NetAddr& otheraddr) { return -1; }
	bool setSocketTimeout(uint32_t recvTimeout, uint32_t sendTimeout)
	{
		int sock = getHandle();
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
		int sock = getHandle();
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
		int sock = getHandle();
		if (sock == -1)
		{
			return false;
		}

		return ::setsockopt(sock, level, optname, (const char*)optval, optlen) >= 0;
	}
	bool getSocketOpt(int level, int optname, void *optval, int *optlen) const
	{
		int sock = getHandle();
		if (sock == -1)
		{
			return false;
		}

		return ::getsockopt(sock, level, optname, (char*)optval, (socklen_t*)optlen) >= 0;
	}
	NetAddr getMyAddr()
	{
		boost::shared_ptr<SocketType> sockptr;
		{
			sockptr = sock;
		}
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
	NetAddr getOtherAddr()
	{
		boost::shared_ptr<SocketType> sockptr;
		{
			sockptr = sock;
		}
		if (sockptr == NULL)
		{
			return NetAddr();
		}

		try
		{
			NetAddr otheraddr(sockptr->remote_endpoint().address().to_v4().to_string(), sockptr->remote_endpoint().port());

			return otheraddr;
		}
		catch (const std::exception& e)
		{
			logdebug("%s %d sockptr->remote_endpoint() std::exception %s\r\n", __FUNCTION__, __LINE__, e.what());
		}

		return NetAddr();
	}
protected:
	virtual void socketDisconnect(const char* error)
	{
		NetStatus curstatus;
		{
			curstatus = status;
			status = NetStatus_notconnected;
		}
		if (curstatus == NetStatus_connected)
		{
			shared_ptr<Socket> _sockptr = sockobjptr.lock();
			if(_sockptr != NULL) disconnectCallback(_sockptr, error == NULL ? "disconnect":error);
		}
	}
	///启动接收
	bool postReceive(boost::shared_ptr<RecvInternal>& internal)
	{
		{
			Guard locker(mutex);
			if (internal == NULL || mustQuit || currRecvInternal != NULL || status != NetStatus_connected)
			{
				return false;
			}
			currRecvInternal = internal;
			_callbackThreadUsedStart();
		}

		bool ret = doPostRecv(currRecvInternal);

		callbackThreadUsedEnd();

		if (!ret)
		{
			///发送失败，告诉该节点 失败了
			_recvCallbackPtr(boost::system::error_code(), -1);

			return false;
		}

		return true;
	}

	///启动发送
	bool beginSend(boost::shared_ptr<SendInternal>& internal)
	{
		{
			Guard locker(mutex);
			if (internal == NULL || mustQuit || status != NetStatus_connected)
			{
				return false;
			}

			sendInternalList.push_back(internal);

			_callbackThreadUsedStart();
		}

		doStartSendData();

		callbackThreadUsedEnd();

		return true;
	}
public:
	virtual void _recvCallbackPtr(const boost::system::error_code& er, std::size_t length)
	{
		boost::shared_ptr<RecvInternal> needDoRecvInternal;
		{
			Guard locker(mutex);
			if (mustQuit || status != NetStatus_connected)///如果socket已经被关闭，停止
			{
				return;
			}
			///获取当前出发接收的节点
			needDoRecvInternal = currRecvInternal;
			currRecvInternal = boost::shared_ptr<RecvInternal>();

			///记录当前回调的处理信息
			_callbackThreadUsedStart();
		}
		if (er && type != NetType_Udp)
		{
			///接收失败，tcp都是断开连接，udp也不会进来，断开了
			socketDisconnect(er.message().c_str());
		}
		else if (!er && needDoRecvInternal != NULL)
		{
			shared_ptr<Socket> _sockptr = sockobjptr.lock();
			if (_sockptr != NULL) needDoRecvInternal->setRecvResult(_sockptr, length);
		}

		///清除当前回调的记录信息
		callbackThreadUsedEnd();
	}
	virtual void _sendCallbackPtr(const boost::system::error_code& er, std::size_t length)
	{
		boost::shared_ptr<SendInternal> needDoSendInternal;
		{
			Guard locker(mutex);
			if (mustQuit || status != NetStatus_connected)///如果socket已经被关闭，停止
			{
				return;
			}
			///获取当前出发接收的节点
			needDoSendInternal = currSendInternal;

			///记录当前回调的处理信息
			_callbackThreadUsedStart();
		}
		bool sendover = true;
		if (needDoSendInternal != NULL)
		{
			shared_ptr<Socket> _sockptr = sockobjptr.lock();
			if (_sockptr != NULL) sendover = needDoSendInternal->setSendResultAndCheckIsOver(_sockptr, !er ? length : 0);
		}

		{
			///清除当前回调的记录信息
			Guard locker(mutex);

			if (sendover && sendInternalList.size() > 0)
			{
				boost::shared_ptr<SendInternal> internal = sendInternalList.front();
				if (needDoSendInternal == internal)
				{
					sendInternalList.pop_front();
				}
			}

			currSendInternal = boost::shared_ptr<SendInternal>();
		}

		doStartSendData();

		callbackThreadUsedEnd();
	}
protected:
	virtual bool doStartSend(boost::shared_ptr<SendInternal>& internal) = 0;
	virtual bool doPostRecv(boost::shared_ptr<RecvInternal>& internal) = 0;
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
				///当前有人发送数据
				return;
			}
			if (sendInternalList.size() <= 0)
			{
				///当前没数据可以发送
				return;
			}
			needSendData = sendInternalList.front();
			currSendInternal = needSendData;
		}

		bool ret = doStartSend(needSendData);
		if (!ret)
		{
			///如果数据启动发送失败,清除当前数据
			_sendCallbackPtr(boost::system::error_code(), -1);
		}
	}
public:
	virtual void _acceptCallbackPtr(const boost::system::error_code& er) {}
	virtual void _connectCallbackPtr(const boost::system::error_code& er) {}
protected:
	shared_ptr<IOWorker>							worker;
	boost::shared_ptr<RecvInternal>						currRecvInternal;	///当前正在接收的节点
	std::list<boost::shared_ptr<SendInternal> >			sendInternalList;	///需要发送的节点列表
	boost::shared_ptr<SendInternal>						currSendInternal;	///当前正在发送的节点

	NetType											type;				///socket类型
	NetStatus										status;				///socket状态
	weak_ptr<Socket>								sockobjptr;
	Socket::DisconnectedCallback					disconnectCallback;	///断开回调通知
public:
	boost::shared_ptr<SocketType>							sock;
};

}
}


#endif //__ASIOSOCKET_OBJCET_H__
