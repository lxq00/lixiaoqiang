#ifndef __IASYNCOBJECTCAN_H__
#define __IASYNCOBJECTCAN_H__
#include "AsyncObject.h"

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
		
		int newsock = accept(sockfd, (sockaddr*)&conaddr, &len);
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

			recvlen = recvfrom(sockfd, buffer, bufferlen, 0, (sockaddr*)&conaddr, &len);
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
	AsyncObjectCan(const Public::Base::shared_ptr<AsyncManager>& _manager):AsyncObject(_manager) {}
	virtual ~AsyncObjectCan(){}

	virtual bool addConnectEvent(const Public::Base::shared_ptr<Socket>& sock, const NetAddr& othreaddr, const Socket::ConnectedCallback& callback)
	{
		Public::Base::shared_ptr<ConnectCanEvent> event(new ConnectCanEvent);
		event->callback = callback;
		event->otheraddr = othreaddr;
		event->manager = manager;

		if (!AsyncObject::addConnectEvent(sock, event))
		{
			return false;
		}
		AsyncObject::buildDoingEvent(sock);
		return true;
	}
	virtual bool addAcceptEvent(const Public::Base::shared_ptr<Socket>& sock, const Socket::AcceptedCallback& callback)
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
	virtual bool addRecvFromEvent(const Public::Base::shared_ptr<Socket>& sock, char* buffer, int bufferlen, const Socket::RecvFromCallback& callback)
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
	virtual bool addRecvEvent(const Public::Base::shared_ptr<Socket>& sock, char* buffer, int bufferlen, const Socket::ReceivedCallback& callback)
	{
		Public::Base::shared_ptr<RecvCanEvent> event(new RecvCanEvent);
		event->buffer = buffer;
		event->bufferlen = bufferlen;
		event->recvCallback = callback;
		event->manager = manager;

		return AsyncObject::addRecvEvent(sock, event);
	}
	virtual bool addSendToEvent(const Public::Base::shared_ptr<Socket>& sock, const char* buffer, int bufferlen, const NetAddr& otheraddr, const Socket::SendedCallback& callback)
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
	virtual bool addSendEvent(const Public::Base::shared_ptr<Socket>& sock, const char* buffer, int bufferlen, const Socket::SendedCallback& callback)
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
	virtual bool addDisconnectEvent(const Public::Base::shared_ptr<Socket>& sock, const Socket::DisconnectedCallback& callback)
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

};


#endif //__IASYNCOBJECT_H__
