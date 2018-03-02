#ifndef __SIMPLEIO_REACTOR_OBJECT_H__
#define __SIMPLEIO_REACTOR_OBJECT_H__

#include "reactor.h"

class Reactor_Object:public enable_shared_from_this<Reactor_Object>
{
	struct RecvInfo
	{
		char* buffer;
		int bufferlen;
		Socket::RecvFromCallback recvfromCallback;
		Socket::ReceivedCallback recvCallback;
	};
	struct SendInfo
	{
		const char* buffer;
		int bufferlen;
		NetAddr otheraddr;
		Socket::SendedCallback callback;
	};
public:
	Reactor_Object(const Public::Base::shared_ptr<Socket>& _sock) :sockobj(_sock), reactorHandle(NULL){}
	~Reactor_Object()
	{
		clean();
	}
	virtual bool addReactor(const Public::Base::shared_ptr<Reactor>& _reactor)
	{
		reactor = _reactor;
		reactorHandle = reactor->addSocket(shared_from_this());

		return true;
	}
	virtual void clean()
	{
		if (reactorHandle != NULL)
			reactor->delSocket(this, reactorHandle);
		reactor->delConnect(this);

		reactorHandle = NULL;
		sockobj = weak_ptr<Socket>();
	}
	virtual bool addConnect(const NetAddr& othreaddr, const Socket::ConnectedCallback& callback)
	{
		Socket::ConnectedCallback tmp = connectCalblack;

		connectCalblack = callback;
		if (othreaddr.isValid()) connectOthreadAddr = new NetAddr(othreaddr);
		else connectOthreadAddr = NULL;

		if (tmp == NULL && connectCalblack != NULL) reactor->addConnect(shared_from_this());
		else if (tmp != NULL && connectCalblack == NULL) reactor->delConnect(this);

		return true;
	}
	virtual bool addAccept(const Socket::AcceptedCallback& callback)
	{
		Socket::AcceptedCallback tmp = acceptCallback;
		acceptCallback = callback;

		if (tmp == NULL && acceptCallback != NULL) reactor->setInEvent(shared_from_this(), reactorHandle);
		else if (tmp != NULL && acceptCallback == NULL) reactor->clearInEvent(shared_from_this(), reactorHandle);

		return true;
	}
	virtual bool addRecvfrom(char* buffer, int bufferlen, const Socket::RecvFromCallback& callback)
	{
		RecvInfo info;
		info.buffer = buffer;
		info.bufferlen = bufferlen;
		info.recvfromCallback = callback;

		uint32_t recvsize = recvList.size();
		recvList.push_back(info);

		if(recvsize == 0) reactor->setInEvent(shared_from_this(), reactorHandle);

		return true;
	}
	virtual bool addRecv(char* buffer, int bufferlen, const Socket::ReceivedCallback& callback)
	{
		RecvInfo info;
		info.buffer = buffer;
		info.bufferlen = bufferlen;
		info.recvCallback = callback;

		uint32_t recvsize = recvList.size();
		recvList.push_back(info);

		if (recvsize == 0) reactor->setInEvent(shared_from_this(), reactorHandle);

		return true;
	}
	virtual bool addSendto(const char* buffer, int bufferlen, const NetAddr& otheraddr, const Socket::SendedCallback& callback)
	{
		SendInfo info;
		info.otheraddr = otheraddr;
		info.buffer = buffer;
		info.bufferlen = bufferlen;
		info.callback = callback;

		uint32_t sendsize = sendList.size();
		sendList.push_back(info);

		if (sendsize == 0) reactor->setOutEvent(shared_from_this(), reactorHandle);

		return true;
	}
	virtual bool addSend(const char* buffer, int bufferlen, const Socket::SendedCallback& callback)
	{
		SendInfo info;
		info.buffer = buffer;
		info.bufferlen = bufferlen;
		info.callback = callback;

		uint32_t sendsize = sendList.size();
		sendList.push_back(info);

		if (sendsize == 0) reactor->setOutEvent(shared_from_this(), reactorHandle);

		return true;
	}
	virtual bool addDisconnect(const Socket::DisconnectedCallback& callback)
	{
		Socket::DisconnectedCallback tmp = disconnectCallback;
		disconnectCallback = callback;

		if (tmp == NULL && disconnectCallback != NULL) reactor->setErrEvent(shared_from_this(), reactorHandle);
		else if(tmp != NULL && disconnectCallback == NULL) reactor->clearErrEvent(shared_from_this(), reactorHandle);

		return true;
	}
	Public::Base::shared_ptr<Socket> sock() { return sockobj.lock(); }

	virtual void connectEvent()
	{
		Public::Base::shared_ptr<Socket> s = sockobj.lock();
		if (s == NULL) return;
		int fd = s->getHandle();
		
		Public::Base::shared_ptr<NetAddr> addrtmp = connectOthreadAddr;
		if (fd == -1 || addrtmp == NULL) return;

		int ret = connect(fd, addrtmp->getAddr(), addrtmp->getAddrLen());
		if (ret < 0)
		{
			return;
		}

		reactor->delConnect(this);
		connectOthreadAddr = NULL;

		setSocketNoBlock(fd);

		Socket::ConnectedCallback callbacktmp(connectCalblack);

		updateSocketStatus(s, true);

		UsingLocker locker(s);
		if(locker.lock()) callbacktmp(s);
	}
	virtual void inEvent()
	{
		Public::Base::shared_ptr<Socket> s = sockobj.lock();
		if (s == NULL)
		{
			return clean();
		}
		int sockfd = s->getHandle();
		if (sockfd == -1)
		{
			return clean();
		}

		if (recvList.size() > 0)
		{
			RecvInfo info = recvList.front();
			recvList.pop_front();

			int recvlen = 0;

			UsingLocker locker(s);

			if (s->getNetType() == NetType_Udp)
			{
				sockaddr_in othearaddr;
				int len = sizeof(sockaddr_in);
				memset(&othearaddr, 0, len);
				othearaddr.sin_family = AF_INET;

				recvlen = recvfrom(sockfd, info.buffer, info.bufferlen, 0, (sockaddr*)&othearaddr, (socklen_t*)&len);
				if (recvlen > 0 && locker.lock())
					info.recvfromCallback(s, info.buffer, recvlen, NetAddr(*(SockAddr*)&othearaddr));
			}
			else
			{
				recvlen = recv(sockfd, info.buffer, info.bufferlen, 0);
				if (recvlen > 0 && locker.lock())
					info.recvCallback(s, info.buffer, recvlen);
			}

			if (recvlen <= 0)
			{
				return errEvent();
			}			
			if (recvList.size() <= 0) reactor->clearInEvent(shared_from_this(),reactorHandle);
		}
		else if (acceptCallback != NULL)
		{
			UsingLocker locker(s);

			sockaddr_in othearaddr;
			int len = sizeof(sockaddr_in);
			memset(&othearaddr, 0, len);
			othearaddr.sin_family = AF_INET;

			int newsock = accept(sockfd, (sockaddr*)&othearaddr, (socklen_t*)&len);
			if (newsock > 0)
			{
				setSocketNoBlock(newsock);

				Public::Base::shared_ptr<Socket> ns = buildSocketBySock(newsock, NetAddr(*(SockAddr*)&othearaddr));
				if (ns != NULL && locker.lock() && acceptCallback != NULL)
					acceptCallback(s,ns);
			}
		}
		else
		{
			reactor->clearInEvent(shared_from_this(),reactorHandle);
		}
	}
	virtual void outEvent()
	{
		Public::Base::shared_ptr<Socket> s = sockobj.lock();
		if (s == NULL)
		{
			return clean();
		}
		int sockfd = s->getHandle();
		if (sockfd == -1)
		{
			return clean();
		}
		if (sendList.size() > 0)
		{
			UsingLocker locker(s);

			SendInfo info = sendList.front();
			sendList.pop_front();

			int sendlen = 0;
			if (s->getNetType() == NetType_Udp)
			{
				sendlen = send(sockfd, info.buffer, info.bufferlen, 0);
			}
			else
			{
				sendlen = sendto(sockfd, info.buffer, info.bufferlen, 0, info.otheraddr.getAddr(), info.otheraddr.getAddrLen());
			}
			if(locker.lock())
				info.callback(s, info.buffer, info.bufferlen);

			if (sendList.size() < 0) reactor->clearOutEvent(shared_from_this(),reactorHandle);
		}
		else
		{
			reactor->clearOutEvent(shared_from_this(), reactorHandle);
		}
	}
	virtual void errEvent()
	{
		Public::Base::shared_ptr<Socket> s = sockobj.lock();
		if (s == NULL)
		{
			return clean();
		}
		int sockfd = s->getHandle();
		if (sockfd == -1)
		{
			return clean();
		}
		updateSocketStatus(s, false);
		UsingLocker locker(s);
		if(locker.lock())
			disconnectCallback(s, "disconnect");

		clean();
	}
private:
	virtual Public::Base::shared_ptr<Socket> buildSocketBySock(int newsock, const NetAddr& otheraaddr) = 0;
	virtual void updateSocketStatus(const Public::Base::shared_ptr<Socket>& sock, bool status) = 0;
protected:
	Public::Base::weak_ptr<Socket>	sockobj;
	Public::Base::shared_ptr<Reactor>	reactor;
	Reactor::Handle		reactorHandle;

	Public::Base::shared_ptr<NetAddr>				connectOthreadAddr;
	Socket::ConnectedCallback		connectCalblack;

	Socket::AcceptedCallback		acceptCallback;
	Socket::DisconnectedCallback	disconnectCallback;
	std::list<RecvInfo>				recvList;
	std::list<SendInfo>				sendList;
};

#endif //__SIMPLEIO_REACTOR_H__
