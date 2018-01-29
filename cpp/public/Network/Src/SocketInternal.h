#ifndef __SOCKETTCP_H__
#define __SOCKETTCP_H__
#include "Async/AsyncManager.h"
#include "Network/Socket.h"
#include "IOWorkerInternal.h"
class SocketInternal:public Socket,public Public::Base::enable_shared_from_this<SocketInternal>
{
public:
	static bool init()
	{
		static bool initStatus = false;
		if (!initStatus)
		{
#ifdef WIN32
			WSADATA wsaData;
			WORD wVersionRequested;

			wVersionRequested = MAKEWORD(2, 2);
			int errorCode = WSAStartup(wVersionRequested, &wsaData);
			if (errorCode != 0)
			{
				return false;
			}

			if (LOBYTE(wsaData.wVersion) != 2 || HIBYTE(wsaData.wVersion) != 2)
			{
				WSACleanup();
				return false;
			}
#else
			signal(SIGPIPE, SIG_IGN);
#endif
		}
		initStatus = true;
		return true;
	}
public:	
	SocketInternal::SocketInternal(const Public::Base::shared_ptr<AsyncManager>& _manager,const Public::Base::shared_ptr<Socket>& _sock, NetType _type)
		:type(_type),sockfd(-1),sock(_sock), manager(_manager)
	{
#ifdef WIN32
		int domain = (_type != NetType_Udp) ? SOCK_STREAM : SOCK_DGRAM;
		int protocol = (_type != NetType_Udp) ? IPPROTO_TCP : IPPROTO_UDP;
		sockfd = WSASocket(AF_INET, domain, protocol, NULL, 0, WSA_FLAG_OVERLAPPED);
		if (sockfd <= 0)
		{
			perror("WSASocket");
			return;
		}
#else
		int domain = (_type != NetType_Udp) ? SOCK_STREAM : SOCK_DGRAM;
		sockfd = socket(AF_INET, domain, 0);
		if (sockfd <= 0)
		{
			perror("socket");
			return;
		}

		int flags = fcntl(sockfd, F_GETFL, 0);
		fcntl(sockfd, F_SETFL, flags | O_NONBLOCK);
#endif
		shared_ptr<AsyncManager> _manager = _manager;
		async = _manager->addSocket(sockfd);
	}
	SocketInternal::SocketInternal(const Public::Base::shared_ptr<AsyncManager>& _manager,int _sockfd,const NetAddr& _otheraddr)
		: type(NetType_TcpConnection),sockfd(_sockfd), otheraddr(_otheraddr), manager(_manager)
	{
		Public::Base::shared_ptr<AsyncManager> _manager = _manager;
		async = _manager->addSocket(sockfd);
	}
	~SocketInternal()
	{
		disconnect();
	}
	bool disconnect()
	{
		if (async != NULL && sockfd != -1)
		{
			async->deleteSocket(sockfd);
		}
		if (sockfd != -1)
		{
#ifdef WIN32
			closesocket(sockfd);
#else
			close(sockfd);
#endif
			sockfd = -1;
		}
		async = NULL;
		return true;
	}
	
	bool bind(const NetAddr& addr, bool reusedAddr)
	{
		if (sockfd == -1) return false;
		if (reusedAddr)
		{
			int reuseaddr = 1;
			setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, (const char *)&reuseaddr, sizeof(int));
		}
		if (::bind(sockfd, (struct sockaddr*)addr.getAddr(), addr.getAddrLen()) < 0) 
		{
			return false;
		}

		myaddr = addr;

		return true;
	}
	bool listen(int flag)
	{
		if (sockfd == -1 || type != NetType_TcpServer) return false;

		if (::listen(sockfd, flag) < 0)
		{
			return false;
		}

		return true;
	}
	bool connect(const NetAddr& addr)
	{
		if (sockfd == -1) return false;

		int ret = ::connect(sockfd, addr.getAddr(), addr.getAddrLen());
		if (ret >= 0)
		{
			otheraddr = addr;
		}

		return ret >= 0;
	}
	Public::Base::shared_ptr<Socket> accept()
	{
		Public::Base::shared_ptr<AsyncManager> _manager = manager.lock();
		if (_manager == NULL)
		{
			return Public::Base::shared_ptr<Socket>();
		}
		if (sockfd == -1) return Public::Base::shared_ptr<Socket>();

		sockaddr_in conaddr;
		int len = sizeof(sockaddr_in);
		memset(&conaddr, 0, len);
		conaddr.sin_family = AF_INET;

		int newsock = ::accept(sockfd, (sockaddr*)&conaddr, &len);
		if (newsock <= 0)
		{
			return Public::Base::shared_ptr<Socket>();
		}
		NetAddr otheraaddr = NetAddr(*(SockAddr*)&conaddr);

		return buildSocketBySock(_manager, newsock, otheraaddr);
	}
	bool setDisconnectCallback(const DisconnectedCallback& disconnected)
	{
		Public::Base::shared_ptr<Socket> socktmp = sock.lock();
		if (socktmp == NULL)
		{
			socktmp = shared_from_this();
		}

		return async->addDisconnectEvent(socktmp, disconnected);
	}
	bool getSocketBuffer(uint32_t& recvSize, uint32_t& sendSize) const
	{
		if (sockfd == -1) return false;
		int optlen = sizeof(sendSize);
		int err = getsockopt(sockfd, SOL_SOCKET, SO_SNDBUF, (char*)&sendSize, &optlen);
		if (err<0) {
			return false;
		}
		optlen = sizeof(recvSize);
		err = getsockopt(sockfd, SOL_SOCKET, SO_RCVBUF, (char*)&recvSize, &optlen);
		if (err<0) {
			return false;
		}

		return true;
	}
	bool setSocketBuffer(uint32_t recvSize, uint32_t sendSize)
	{
		if (sockfd == -1 || recvSize == 0 || sendSize == 0) return false;
		if (recvSize != 0)
		{
			int optlen = sizeof(recvSize);
			int err = setsockopt(sockfd, SOL_SOCKET, SO_RCVBUF, (char *)&recvSize, optlen);
			if (err < 0) {
				return false;
			}
		}
		if (sendSize != 0)
		{
			int optlen = sizeof(sendSize);
			int err = setsockopt(sockfd, SOL_SOCKET, SO_RCVBUF, (char *)&sendSize, optlen);
			if (err < 0) {
				return false;
			}
		}
		return true;
	}
	bool async_recv(char *buf, uint32_t len, const ReceivedCallback& received)
	{
		if (sockfd == -1 || buf == NULL || len == 0 || type == NetType_Udp) return false;

		Public::Base::shared_ptr<Socket> socktmp = sock.lock();
		if (socktmp == NULL)
		{
			socktmp = shared_from_this();
		}

		return async->addRecvEvent(socktmp, buf, len, received);
	}
	bool async_send(const char * buf, uint32_t len, const SendedCallback& sended)
	{
		if (sockfd == -1 || buf == NULL || len == 0 || type == NetType_Udp) return false;

		Public::Base::shared_ptr<Socket> socktmp = sock.lock();
		if (socktmp == NULL)
		{
			socktmp = shared_from_this();
		}

		return async->addSendEvent(socktmp, buf, len, sended);
	}
	int recv(char *buf, uint32_t len)
	{
		if (sockfd == -1 || buf == NULL || len == 0)
		{
			return -1;
		}

		return ::recv(sockfd,buf,len,0);
	}
	int send(const char * buf, uint32_t len)
	{
		if (sockfd == NULL || buf == NULL || len == 0)
		{
			return -1;
		}

		return ::send(sockfd,buf,len,0);
	}
	int getHandle() const
	{
		return sockfd;
	}
	
	NetType getNetType() const
	{
		return type;
	}
	NetAddr getMyAddr() const
	{
		return myaddr;
	}
	NetAddr getOhterAddr() const
	{
		return otheraddr;
	}
public:
	NetType					type;
	int						sockfd;
	Public::Base::weak_ptr<Socket>		sock;
	Public::Base::shared_ptr<AsyncObject> async;
	NetAddr					myaddr;
	NetAddr					otheraddr;

	Public::Base::weak_ptr<AsyncManager> manager;
};

inline Public::Base::shared_ptr<Socket> buildSocketBySock(const Public::Base::weak_ptr<AsyncManager>& manager, int sock, const NetAddr& otheraaddr)
{
	Public::Base::shared_ptr<AsyncManager> _manager = manager.lock();
	if (_manager == NULL)
	{
		return Public::Base::shared_ptr<Socket>();
	}

	return Public::Base::shared_ptr<Socket>(new SocketInternal(_manager, sock, otheraaddr));
}


#endif
