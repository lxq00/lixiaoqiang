#ifndef __IASYNCOBJECTCAN_H__
#define __IASYNCOBJECTCAN_H__
#include "AsyncObject.h"
#ifdef WIN32
typedef  int socklen_t;
#endif
struct ConnectCanEvent:public ConnectEvent
{
	bool doCanEvent(const Public::Base::shared_ptr<Socket>& sock, int flag)
	{
		int sockfd = sock->getHandle();

		int ret = connect(sockfd, otheraddr.getAddr(), otheraddr.getAddrLen());
		if (ret < 0)
		{
			return false;
		}

#ifdef WIN32
		{
			int ionbio = 1;
			ioctlsocket(sockfd, FIONBIO, (u_long*)&ionbio);
		}
#else
		{
			int flags = fcntl(sockfd, F_GETFL, 0);
			fcntl(sockfd, F_SETFL, flags | O_NONBLOCK);
		}
#endif

		return doResultEvent(sock);
	}
};

struct AcceptCanEvent :public AcceptEvent
{
	bool doCanEvent(const Public::Base::shared_ptr<Socket>& sock, int flag)
	{
		int sockfd = sock->getHandle();

		sockaddr_in conaddr;
		int len = sizeof(sockaddr_in);
		memset(&conaddr, 0, len);
		conaddr.sin_family = AF_INET;
		
		int newsock = accept(sockfd, (sockaddr*)&conaddr, (socklen_t*)&len);
		if (newsock <= 0)
		{
			return false;
		}
		otheraaddr = NetAddr(*(SockAddr*)&conaddr);

		return doResultEvent(sock,newsock);
	}
};

struct SendCanEvent :public SendEvent
{
	const char* sendtmp;
	int   sendtmplen;

	SendCanEvent():sendtmp(NULL),sendtmplen(0){}

	bool doCanEvent(const Public::Base::shared_ptr<Socket>& sock, int flag)
	{
		int sockfd = sock->getHandle();

		if (sendtmp == NULL)
		{
			sendtmp = buffer;
			sendtmplen = bufferlen;
		}

		int sendlen = 0;
		if (otheraddr == NULL)
		{
			sendlen = send(sockfd, sendtmp, sendtmplen, 0);
		}
		else
		{
			sendlen = sendto(sockfd, sendtmp, sendtmplen, 0, otheraddr->getAddr(), otheraddr->getAddrLen());
		}
		if (sendlen > 0 && sendlen <= sendtmplen)
		{
			sendtmp += sendlen;
			sendtmplen -= sendlen;
		}

		if (sendlen > 0 && sendtmplen > 0)
		{
			return false;
		}

		return doResultEvent(sock, bufferlen - sendtmplen);
	}
};

struct RecvCanEvent :public RecvEvent
{
	bool doCanEvent(const Public::Base::shared_ptr<Socket>& sock, int flag)
	{
		int sockfd = sock->getHandle();

		int recvlen = 0;

		if (recvCallback == NULL)
		{
			sockaddr_in conaddr;
			int len = sizeof(sockaddr_in);
			memset(&conaddr, 0, len);
			conaddr.sin_family = AF_INET;

			recvlen = recvfrom(sockfd, buffer, bufferlen, 0, (sockaddr*)&conaddr, (socklen_t*)&len);
			if (recvlen > 0)
			{
				otheraddr = new NetAddr(*(SockAddr*)&conaddr);
			}
		}
		else
		{
			recvlen = recv(sockfd, buffer, bufferlen, 0);
		}

		if (recvlen <= 0)
		{
			return false;
		}

		return doResultEvent(sock, recvlen);
	}
};


class AsyncObjectCan:public AsyncObject
{
public:
	AsyncObjectCan(const Public::Base::shared_ptr<AsyncManager>& _manager, AsyncSuportType _type):AsyncObject(_manager,_type){}
	virtual ~AsyncObjectCan(){}

	virtual void _addSocketEvent(const Public::Base::shared_ptr<Socket>& sock, EventType type, const Public::Base::shared_ptr<DoingAsyncInfo>& doasyncinfo, const Public::Base::shared_ptr<AsyncEvent>& event) {}

	virtual bool addSocket(const Public::Base::shared_ptr<Socket>& sock)
	{
		return AsyncObject::addSocket(sock);
	}
	virtual bool deleteSocket(Socket* sockptr, int sockfd)
	{
		{
			Guard locker(connectmutex);
			connectList.erase(sockptr);
		}
		return AsyncObject::deleteSocket(sockptr, sockfd);
	}

	virtual bool addConnect(const Public::Base::shared_ptr<Socket>& sock, const NetAddr& othreaddr, const Socket::ConnectedCallback& callback)
	{
		Public::Base::shared_ptr<ConnectCanEvent> event(new ConnectCanEvent);
		event->callback = callback;
		event->otheraddr = othreaddr;
		event->manager = manager;
		event->asyncobj = shared_from_this();
#if 0
		if (!AsyncObject::addConnectEvent(sock, event))
		{
			return false;
		}
		AsyncObject::buildDoingEvent(sock);
#else
		ConnectInfo connectInfo;
		connectInfo.sock = sock;
		connectInfo.event = event;

		Guard locker(connectmutex);
		connectList[sock.get()] = connectInfo;
#endif
		return true;
	}
	virtual bool addAccept(const Public::Base::shared_ptr<Socket>& sock, const Socket::AcceptedCallback& callback)
	{
		Public::Base::shared_ptr<AcceptCanEvent> event(new AcceptCanEvent);
		event->callback = callback;
		event->manager = manager;

		if (!AsyncObject::addAcceptEvent(sock, event))
		{
			return false;
		}
		AsyncObject::buildDoingEvent(sock);
		return true;
	}
	virtual bool addRecvfrom(const Public::Base::shared_ptr<Socket>& sock, char* buffer, int bufferlen, const Socket::RecvFromCallback& callback)
	{
		Public::Base::shared_ptr<RecvCanEvent> event(new RecvCanEvent);
		event->buffer = buffer;
		event->bufferlen = bufferlen;
		event->recvFromCallback = callback;
		event->manager = manager;

		if (!AsyncObject::addRecvEvent(sock, event))
		{
			return false;
		}
		AsyncObject::buildDoingEvent(sock);
		return true;
	}
	virtual bool addRecv(const Public::Base::shared_ptr<Socket>& sock, char* buffer, int bufferlen, const Socket::ReceivedCallback& callback)
	{
		Public::Base::shared_ptr<RecvCanEvent> event(new RecvCanEvent);
		event->buffer = buffer;
		event->bufferlen = bufferlen;
		event->recvCallback = callback;
		event->manager = manager;

		if (!AsyncObject::addRecvEvent(sock, event))
		{
			return false;
		}
		AsyncObject::buildDoingEvent(sock);
		return true;
	}
	virtual bool addSendto(const Public::Base::shared_ptr<Socket>& sock, const char* buffer, int bufferlen, const NetAddr& otheraddr, const Socket::SendedCallback& callback)
	{
		Public::Base::shared_ptr<SendCanEvent> event(new SendCanEvent);
		event->buffer = buffer;
		event->bufferlen = bufferlen;
		event->otheraddr = new NetAddr(otheraddr);
		event->callback = callback;
		event->manager = manager;

		if (!AsyncObject::addSendEvent(sock, event))
		{
			return false;
		}
		AsyncObject::buildDoingEvent(sock);
		return true;
	}
	virtual bool addSend(const Public::Base::shared_ptr<Socket>& sock, const char* buffer, int bufferlen, const Socket::SendedCallback& callback)
	{
		Public::Base::shared_ptr<SendCanEvent> event(new SendCanEvent);
		event->buffer = buffer;
		event->bufferlen = bufferlen;
		event->callback = callback;
		event->manager = manager;

		if (!AsyncObject::addSendEvent(sock, event))
		{
			return false;
		}
		AsyncObject::buildDoingEvent(sock);
		return true;
	}
	virtual bool addDisconnect(const Public::Base::shared_ptr<Socket>& sock, const Socket::DisconnectedCallback& callback)
	{
		Public::Base::shared_ptr<DisconnectEvent> event(new DisconnectEvent);
		event->callback = callback;

		if (!AsyncObject::addDisconnectEvent(sock, event))
		{
			return false;
		}
		AsyncObject::buildDoingEvent(sock);
		return true;
	}
protected:
#define DEFAULTCANDOCONNECTTIMEOUT		10
	void doThreadConnectProc()
	{
		std::map<Socket*, ConnectInfo> connectListtmp;
		{
			Guard locker(mutex);
			connectListtmp = connectList;
		}

		std::list<Socket*> successList;
		for (std::map<Socket*, ConnectInfo>::iterator iter = connectListtmp.begin(); iter != connectListtmp.end(); iter++)
		{
			Public::Base::shared_ptr<Socket> sock = iter->second.sock.lock();
			if (sock == NULL) continue;

			int sendTimeout = 0;
			int sockfd = sock->getHandle();
			{
				int sendlen = sizeof(uint32_t);
				getsockopt(sockfd, SOL_SOCKET, SO_SNDTIMEO, (char*)&sendTimeout, (socklen_t*)&sendlen);

				int sendtimeouttmp = DEFAULTCANDOCONNECTTIMEOUT;
				setsockopt(sockfd, SOL_SOCKET, SO_SNDTIMEO, (const char*)&sendtimeouttmp, sizeof(sendtimeouttmp));
			}

			if (iter->second.event == NULL ||  iter->second.event->doCanEvent(sock, 0))
			{
				successList.push_back(iter->first);
			}
			else
			{
				setsockopt(sockfd, SOL_SOCKET, SO_SNDTIMEO, (const char*)&sendTimeout, sizeof(sendTimeout));
			}
		}

		{
			Guard locker(mutex);
			for (std::list<Socket*>::iterator iter = successList.begin(); iter != successList.end(); iter++)
			{
				connectList.erase(*iter);
			}
		}
	}
private:
	struct ConnectInfo
	{
		Public::Base::weak_ptr<Socket>			  sock;
		Public::Base::shared_ptr<ConnectCanEvent> event;
	};
	
	Public::Base::Mutex				connectmutex;
	std::map<Socket*, ConnectInfo>	connectList;
};


#endif //__IASYNCOBJECT_H__
