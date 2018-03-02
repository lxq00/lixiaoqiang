#ifndef __SOCKETTCPOBJECT_H__
#define __SOCKETTCPOBJECT_H__
#include "proactor_iocp.h"
#include "proactor_reactor.h"

class Socket_Object:public Socket,public Public::Base::enable_shared_from_this<Socket_Object>
{
public:	
	Socket_Object(const Public::Base::shared_ptr<Proactor>& _proactor,NetType _type,const Public::Base::shared_ptr<Socket>& ptr)
		:type(_type),sockfd(-1), netstatus(NetStatus_notconnected),proactor(_proactor), sockobj(ptr),sockptr(ptr.get())
	{
		initNetwork();		
		
		if (proactor != NULL)
		{
			sockfd = proactor->createSocket(type);
		}
		
		if (_type == NetType_Udp)
		{
			netstatus = NetStatus_connected;
		}
	}
	Socket_Object(const Public::Base::shared_ptr<Proactor>& _proactor, int _sockfd, const NetAddr& _otheraddr)
		:type(NetType_TcpConnection), sockfd(_sockfd), otheraddr(_otheraddr), netstatus(NetStatus_connected), proactor(_proactor), sockptr(NULL){}
	bool initSocket()
	{
		UsingLocker::LockerManager::instance()->addSock(sockptr == NULL ? this : sockptr);
		if (proactor != NULL)
		{
			proactorObj = proactor->addSocket(sockptr == NULL ? shared_from_this():sockobj.lock());
			if (proactorObj == NULL) return false;
		}

		return true;
	}
	~Socket_Object()
	{
		disconnect();
	}
	bool nonBlocking(bool nonblock)
	{
		if (sockfd == -1) return false;
		setSocketNoBlock(sockfd, nonblock);

		return true;
	}
	bool disconnect()
	{
		UsingLocker::LockerManager::instance()->delSock(sockptr == NULL ? this : sockptr);
		if (proactorObj != NULL && sockfd != -1)
		{
			proactorObj->proactor()->delSocket(proactorObj, sockptr == NULL ? this : sockptr);
			proactorObj = NULL;
		}
		if (sockfd != -1)
		{
			closesocket(sockfd);
			sockfd = -1;
		}
		return true;
	}
	
	bool bind(const NetAddr& addr, bool reusedAddr)
	{
		if (sockfd == -1 || !addr.isValid()) return false;
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
			netstatus = NetStatus_connected;
		}

		return ret >= 0;
	}
	bool async_connect(const NetAddr& addr, const ConnectedCallback& callback)
	{
		if (sockfd == -1 || proactorObj == NULL)
		{
			return false;
		}
		return proactorObj->proactor()->addConnect(proactorObj,addr, callback);
	}
	bool async_accept(const AcceptedCallback& callback)
	{
		if (sockfd == -1 || callback == NULL || type != NetType_TcpServer || proactorObj == NULL) return false;

		return proactorObj->proactor()->addAccept(proactorObj, callback);
	}
	Public::Base::shared_ptr<Socket> accept()
	{
		if (sockfd == -1) return Public::Base::shared_ptr<Socket>();

		sockaddr_in conaddr;
		int len = sizeof(sockaddr_in);
		memset(&conaddr, 0, len);
		conaddr.sin_family = AF_INET;

		int newsock = ::accept(sockfd, (sockaddr*)&conaddr, (socklen_t*)&len);
		if (newsock <= 0)
		{
			return Public::Base::shared_ptr<Socket>();
		}
		NetAddr otheraaddr = NetAddr(*(SockAddr*)&conaddr);

		return buildSocketBySock(proactor, newsock, otheraaddr);
	}
	bool setDisconnectCallback(const DisconnectedCallback& disconnected)
	{
		if (proactorObj == NULL) return false;
		return proactorObj->proactor()->addDisconnect(proactorObj, disconnected);
	}
	bool setSocketTimeout(uint32_t recvTimeout, uint32_t sendTimeout)
	{ 
		if (sockfd == -1 || proactorObj != NULL)
		{
			return false;
		}
		if (recvTimeout != (uint32_t)-1)
		{
			int ret = setsockopt(getHandle(), SOL_SOCKET, SO_RCVTIMEO, (const char*)&recvTimeout, sizeof(recvTimeout));
			if (ret < 0)
			{
				return false;
			}
		}
		if (sendTimeout != (uint32_t)-1)
		{
			int ret = setsockopt(getHandle(), SOL_SOCKET, SO_SNDTIMEO, (const char*)&sendTimeout, sizeof(sendTimeout));
			if (ret < 0)
			{
				return false;
			}
		}
		
		return true;
	}

	bool getSocketTimeout(uint32_t& recvTimeout, uint32_t& sendTimeout) const 
	{
		if (sockfd == -1 || proactorObj != NULL)
		{
			return false;
		}
		int sendlen = sizeof(uint32_t), recvlen = sizeof(uint32_t);
		int ret = getsockopt(getHandle(), SOL_SOCKET, SO_SNDTIMEO, (char*)&sendTimeout, (socklen_t*)&sendlen);
		ret |= getsockopt(getHandle(), SOL_SOCKET, SO_RCVTIMEO, (char*)&recvTimeout, (socklen_t*)&recvlen);
		return false; 
	}

	bool getSocketBuffer(uint32_t& recvSize, uint32_t& sendSize) const
	{
		if (sockfd == -1) return false;
		int optlen = sizeof(sendSize);
		int err = getsockopt(sockfd, SOL_SOCKET, SO_SNDBUF, (char*)&sendSize, (socklen_t*)&optlen);
		if (err<0) {
			return false;
		}
		optlen = sizeof(recvSize);
		err = getsockopt(sockfd, SOL_SOCKET, SO_RCVBUF, (char*)&recvSize, (socklen_t*)&optlen);
		if (err<0) {
			return false;
		}

		return true;
	}
	bool setSocketBuffer(uint32_t recvSize, uint32_t sendSize)
	{
		if (sockfd == -1 || recvSize == (uint32_t)-1 || sendSize == (uint32_t)-1) return false;
		if (recvSize != (uint32_t)-1)
		{
			int optlen = sizeof(recvSize);
			int err = setsockopt(sockfd, SOL_SOCKET, SO_RCVBUF, (char *)&recvSize, optlen);
			if (err < 0) {
				return false;
			}
		}
		if (sendSize != (uint32_t)-1)
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
		if (sockfd == -1 || buf == NULL || len == 0 || type == NetType_Udp || proactorObj == NULL) return false;

		return proactorObj->proactor()->addRecv(proactorObj, buf, len, received);
	}
	bool async_send(const char * buf, uint32_t len, const SendedCallback& sended)
	{
		if (sockfd == -1 || buf == NULL || len == 0 || type == NetType_Udp || proactorObj == NULL) return false;

		return proactorObj->proactor()->addSend(proactorObj, buf, len, sended);
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
		if (sockfd == -1 || buf == NULL || len == 0)
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
	NetStatus getStatus() const
	{
		return netstatus;
	}
public:
	static Public::Base::shared_ptr<Socket> buildSocketBySock(const Public::Base::shared_ptr<Proactor>& _proactor, int _sockfd, const NetAddr& _otheraddr)
	{
		Public::Base::shared_ptr<Socket_Object> obj;
		obj = new Socket_Object(_proactor, _sockfd, _otheraddr);
		obj->initSocket();

		return obj;
	}
	static void updateSocketStatus(const Public::Base::shared_ptr<Socket>& obj, bool status)
	{
		Socket_Object* objptr = (Socket_Object*)obj.get();
		if (objptr != NULL)
		{
			objptr->netstatus = status ? NetStatus_connected : NetStatus_error;
		}
	}
private:
	Public::Base::weak_ptr<Socket> sockobj;
	Socket*					sockptr;
	NetType					type;
	int						sockfd;	
	NetAddr					myaddr;
	NetAddr					otheraddr;
	NetStatus				netstatus;

	Public::Base::shared_ptr<Proactor>		proactor;
	Public::Base::shared_ptr<Proactor_Object> proactorObj;
};



#endif
