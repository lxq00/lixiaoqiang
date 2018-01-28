#ifndef __ASYNCOBJECTIOCP_H__
#define __ASYNCOBJECTIOCP_H__
#include "AsyncObject.h"

#include <winsock2.h>
#include <MSWSock.h>

struct IOCPConnectCanEvent :public ConnectEvent
{
	OVERLAPPED				overlped;
	LPFN_CONNECTEX			connectExFunc;

	bool doPrevEvent(const shared_ptr<Socket>& sock)
	{
		memset(&overlped, 0, sizeof(OVERLAPPED));

		GUID connetEx = WSAID_CONNECTEX;
		DWORD bytes = 0;
		int ret = WSAIoctl(sock->getHandle(), SIO_GET_EXTENSION_FUNCTION_POINTER, &connetEx, sizeof(connetEx), &connectExFunc, sizeof(connectExFunc), &bytes, NULL, NULL);

		if (ret == SOCKET_ERROR)
		{
			connectExFunc = NULL;
			return false;
		}

		if(!connectExFunc(sock->getHandle(), (const sockaddr*)otheraddr.getAddr(), otheraddr.getAddrLen(), NULL, 0, NULL, &overlped)
			&& GetLastError() != WSA_IO_PENDING)
		{
			return false;
		}

		return true;
	}
	bool doCanEvent(const shared_ptr<Socket>& sock, int flag)
	{
		return doResultEvent(sock);
	}
};

#define SWAPBUFFERLEN			64
struct IOCPAcceptCanEvent :public AcceptEvent
{
	OVERLAPPED		overlped;
	char			swapBuffer[SWAPBUFFERLEN];
	int				newsock;

	LPFN_ACCEPTEX					acceptExFunc;
	LPFN_GETACCEPTEXSOCKADDRS		getAcceptAddExFunc;

	IOCPAcceptCanEvent() :newsock(INVALID_SOCKET) {}
	~IOCPAcceptCanEvent()
	{
		if (newsock != INVALID_SOCKET)
		{
			closesocket(newsock);
			newsock = INVALID_SOCKET;
		}
	}
	bool doPrevEvent(const shared_ptr<Socket>& sock)
	{
		memset(&overlped, 0, sizeof(OVERLAPPED));

		GUID acceptEX = WSAID_ACCEPTEX;
		GUID getAcceptAddrEx = WSAID_GETACCEPTEXSOCKADDRS;

		DWORD bytes = 0;

		int ret = WSAIoctl(sock->getHandle(), SIO_GET_EXTENSION_FUNCTION_POINTER, &acceptEX, sizeof(acceptEX), &acceptExFunc, sizeof(acceptExFunc), &bytes, NULL, NULL);

		bytes = 0;
		ret |= WSAIoctl(sock->getHandle(), SIO_GET_EXTENSION_FUNCTION_POINTER, &getAcceptAddrEx, sizeof(getAcceptAddrEx), &getAcceptAddExFunc, sizeof(getAcceptAddExFunc), &bytes, NULL, NULL);

		if (ret == SOCKET_ERROR)
		{
			acceptExFunc = NULL;
			getAcceptAddExFunc = NULL;

			return false;
		}

		newsock = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, NULL, 0, WSA_FLAG_OVERLAPPED);
		if (newsock == INVALID_SOCKET)
		{
			return false;
		}

		if (!acceptExFunc(sock->getHandle(), newsock, swapBuffer, 0, sizeof(SOCKADDR_IN) + 16, sizeof(SOCKADDR_IN) + 16, NULL, (LPOVERLAPPED)&overlped)
			&& GetLastError() != WSA_IO_PENDING)
		{
			return false;
		}

		return true;
	}	

	bool doCanEvent(const shared_ptr<Socket>& sock, int flag)
	{
		int sockfd = sock->getHandle();

		::setsockopt(newsock, SOL_SOCKET, SO_UPDATE_ACCEPT_CONTEXT, (const char*)&sockfd, sizeof(SOCKET));

		SOCKADDR_IN* clientAddr = NULL, *localAddr = NULL;
		int clientlen = sizeof(SOCKADDR_IN), locallen = sizeof(SOCKADDR_IN);

		getAcceptAddExFunc(swapBuffer, 0, sizeof(SOCKADDR_IN) + 16, sizeof(SOCKADDR_IN) + 16, (LPSOCKADDR*)&localAddr, &locallen, (LPSOCKADDR*)&clientAddr, &clientlen);

		otheraaddr = NetAddr(*(SockAddr*)clientAddr);
		
		int newsocktmp = newsock;
		newsock = INVALID_SOCKET;

		return doResultEvent(sock, newsocktmp);
	}
};

struct IOCPSendCanEvent :public SendEvent
{
	OVERLAPPED		overlped;
	WSABUF			wbuf;
	DWORD			dwBytes;
	
	bool doPrevEvent(const shared_ptr<Socket>& sock)
	{
		dwBytes = 0;
		memset(&overlped, 0, sizeof(OVERLAPPED));
		wbuf.buf = (CHAR*)buffer;
		wbuf.len = bufferlen;

		int sockfd = sock->getHandle();
		if (otheraddr == NULL)
		{
			int ret = ::WSASend(sockfd, &wbuf, 1, &dwBytes, 0, &overlped, NULL);
			if (ret == SOCKET_ERROR && GetLastError() != WSA_IO_PENDING)
			{
				return false;
			}
		}
		else
		{
			int ret = ::WSASendTo(sockfd, &wbuf, 1, &dwBytes, 0, (sockaddr*)otheraddr->getAddr(), otheraddr->getAddrLen(), &overlped, NULL);
			if (ret == SOCKET_ERROR && GetLastError() != WSA_IO_PENDING)
			{
				return false;
			}
		}

		return true;
	}


	bool doCanEvent(const shared_ptr<Socket>& sock, int flag)
	{
		return doResultEvent(sock, flag);
	}
};

struct IOCPRecvCanEvent :public RecvEvent
{
	OVERLAPPED		overlped;
	WSABUF			wbuf;
	DWORD			dwBytes;
	DWORD			dwFlags;

	SOCKADDR		addr;
	int 			addrlen;

	bool doPrevEvent(const shared_ptr<Socket>& sock)
	{
		dwBytes = dwFlags = 0;
		memset(&overlped, 0, sizeof(OVERLAPPED));
		wbuf.buf = (CHAR*)buffer;
		wbuf.len = bufferlen;

		int sockfd = sock->getHandle();

		if (recvCallback != NULL)
		{
			int ret = WSARecv(sockfd, &wbuf, 1, &dwBytes, &dwFlags,&overlped, NULL);
			if (ret == SOCKET_ERROR && GetLastError() != WSA_IO_PENDING)
			{
				return false;
			}
		}
		else
		{
			memset(&addr, 0, sizeof(addr));
			addr.sa_family = AF_INET;
			addrlen = sizeof(SOCKADDR);

			int ret = WSARecvFrom(sockfd, &wbuf, 1, &dwBytes, &dwFlags,(sockaddr*)&addr, &addrlen, &overlped, NULL);
			if (ret == SOCKET_ERROR && GetLastError() != WSA_IO_PENDING)
			{
				return false;
			}
		}
	}
	bool doCanEvent(const shared_ptr<Socket>& sock, int flag)
	{
		int sockfd = sock->getHandle();

		if (recvCallback == NULL)
		{
			otheraddr = new NetAddr(*(SockAddr*)&addr);
		}
		
		return doResultEvent(sock, flag);
	}
};

class AsyncObjectIOCP :public AsyncObject
{
public:
	AsyncObjectIOCP(int threadnum)
	{
		iocpHandle = ::CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, 0);
		if (iocpHandle == NULL)
		{
			return;
		}
		for (uint32_t i = 0; i < threadnum; i++)
		{
			shared_ptr<Thread> thread = ThreadEx::creatThreadEx("AsyncObjectIOCP", ThreadEx::Proc(&AsyncObjectIOCP::doWorkThreadProc, this), NULL);
			thread->createThread();
			threadlist.push_back(thread);
		}
	}
	~AsyncObjectIOCP()
	{
		threadlist.clear();
		if (iocpHandle != NULL)
		{
			CloseHandle(iocpHandle);
			iocpHandle = NULL;
		}
	}
private:
	virtual shared_ptr<AsyncInfo> addConnectEvent(const shared_ptr<Socket>& sock, const NetAddr& othreaddr, const Socket::ConnectedCallback& callback)
	{
		shared_ptr<IOCPConnectCanEvent> event(new IOCPConnectCanEvent);
		event->callback = callback;
		event->otheraddr = othreaddr;

		return AsyncObject::addConnectEvent(sock, event);
	}
	virtual shared_ptr<AsyncInfo> addAcceptEvent(const shared_ptr<Socket>& sock, const Socket::AcceptedCallback& callback)
	{
		shared_ptr<IOCPAcceptCanEvent> event(new IOCPAcceptCanEvent());
		event->callback = callback;

		return AsyncObject::addAcceptEvent(sock, event);
	}
	virtual shared_ptr<AsyncInfo> addRecvFromEvent(const shared_ptr<Socket>& sock, char* buffer, int bufferlen, const Socket::RecvFromCallback& callback)
	{
		shared_ptr<IOCPRecvCanEvent> event(new IOCPRecvCanEvent);
		event->buffer = buffer;
		event->bufferlen = bufferlen;
		event->recvFromCallback = callback;

		return AsyncObject::addRecvEvent(sock, event);
	}
	virtual shared_ptr<AsyncInfo> addRecvEvent(const shared_ptr<Socket>& sock, char* buffer, int bufferlen, const Socket::ReceivedCallback& callback)
	{
		shared_ptr<IOCPRecvCanEvent> event(new IOCPRecvCanEvent);
		event->buffer = buffer;
		event->bufferlen = bufferlen;
		event->recvCallback = callback;

		return AsyncObject::addRecvEvent(sock, event);
	}
	virtual shared_ptr<AsyncInfo> addSendToEvent(const shared_ptr<Socket>& sock, const char* buffer, int bufferlen, const NetAddr& otheraddr, const Socket::SendedCallback& callback)
	{
		shared_ptr<IOCPSendCanEvent> event(new IOCPSendCanEvent);
		event->buffer = buffer;
		event->bufferlen = bufferlen;
		event->otheraddr = new NetAddr(otheraddr);
		event->callback = callback;

		return AsyncObject::addSendEvent(sock, event);
	}
	virtual shared_ptr<AsyncInfo> addSendEvent(const shared_ptr<Socket>& sock, const char* buffer, int bufferlen, const Socket::SendedCallback& callback)
	{
		shared_ptr<IOCPSendCanEvent> event(new IOCPSendCanEvent);
		event->buffer = buffer;
		event->bufferlen = bufferlen;
		event->callback = callback;

		return AsyncObject::addSendEvent(sock, event);
	}
	virtual shared_ptr<AsyncInfo> addDisconnectEvent(const shared_ptr<Socket>& sock, const Socket::DisconnectedCallback& callback)
	{
		shared_ptr<DisconnectEvent> event(new DisconnectEvent);
		event->callback = callback;

		return AsyncObject::addDisconnectEvent(sock, event);
	}
private:
	virtual bool deleteSocket(const Socket* sock)
	{
		AsyncObject::deleteSocket(sock);

		{
			Guard locker(iocpMutex);
			for (std::map<void*, IOCPInfo>::iterator iter = iocpList.begin(); iter != iocpList.end();)
			{
				shared_ptr<AsyncInfo> asyncinfo = iter->second.doasyncinfo->asyncInfo.lock();
				if (asyncinfo == NULL)
				{
					iocpList.erase(iter++);
					continue;
				}
				shared_ptr<Socket> sockobj = asyncinfo->sock.lock();
				if (sockobj == NULL || sockobj.get() == sock)
				{
					iocpList.erase(iter++);
					continue;
				}
				iter++;
			}
		}
	}
	virtual void addSocketEvent(const shared_ptr<Socket>& sock, EventType type,const shared_ptr<DoingAsyncInfo>& doasyncinfo, const shared_ptr<ConnectEvent>& event)
	{
		if (iocpHandle == NULL)
		{
			return;
		}
		{
			IOCPInfo info;
			info.doasyncinfo = doasyncinfo;
			info.event = event;
			Guard locker(iocpMutex);
			iocpList[event.get()] = info;
		}
		event->doPrevEvent(sock);
		HANDLE iosock = ::CreateIoCompletionPort((HANDLE)sock->getHandle(), iocpHandle, (DWORD)event.get(), 0);
		if (iosock == NULL)
		{
			return;
		}
	}
	void doWorkThreadProc(Thread* thread, void* param)
	{
		while (thread->looping())
		{
			DWORD bytes = 0, key = 0;
			OVERLAPPED* poverlaped = NULL;

			if (!GetQueuedCompletionStatus(iocpHandle, &bytes, &key, &poverlaped, 100))
			{
				continue;
			}

			IOCPInfo info;
			{
				Guard locker(iocpMutex);
				std::map<void*, IOCPInfo>::iterator iter = iocpList.find((void*)key);
				if (iter == iocpList.end())
				{
					continue;
				}
			}
			shared_ptr<AsyncInfo> asyncinfo = info.doasyncinfo->asyncInfo.lock();
			if (asyncinfo == NULL)
			{
				continue;
			}
			shared_ptr<Socket> sock = asyncinfo->sock.lock();
			if (sock == NULL)
			{
				continue;
			}

			info.event->doCanEvent(sock);
			AsyncObject::buildDoingEvent(sock.get());
		}
	}
private:
	HANDLE						   iocpHandle;
	std::list<shared_ptr<Thread> > threadlist;

	struct IOCPInfo
	{
		shared_ptr<DoingAsyncInfo> doasyncinfo;
		shared_ptr<ConnectEvent>  event;
	};

	Mutex							iocpMutex;
	std::map<void*, IOCPInfo>		iocpList;
};

#endif //__ASYNCOBJECTIOCP_H__
