#ifndef __SIMPEIO_PROACTOR_IOCP_H__
#define __SIMPEIO_PROACTOR_IOCP_H__

#include "simpleio_def.h"
#ifdef SIMPLEIO_SUPPORT_IOCP
#include <MSWSock.h>
#include "proactor.h"

struct ISocketEvent
{
	enum SocketEventType
	{
		SocketEventType_Accept,
		SocketEventType_Connect,
		SocketEventType_Recv,
		SocketEventType_Send,
	}type;
	OVERLAPPED					overlped;

	ISocketEvent(SocketEventType _type) :type(_type)
	{
		memset(&overlped, 0, sizeof(OVERLAPPED));
	}

	virtual void postEvent(const Public::Base::shared_ptr<Socket>& sock) = 0;
	virtual void doResultEvent(const Public::Base::shared_ptr<Socket>& sock, int bytes) = 0;
};
struct IOCPConnectCanEvent:public ISocketEvent
{
	LPFN_CONNECTEX				connectExFunc;
	NetAddr						otheraddr;
	Socket::ConnectedCallback	callback;

	Proactor::UpdateSocketStatusCallback	statusCallback;

	IOCPConnectCanEvent(const NetAddr& addr, const Socket::ConnectedCallback& _callback):ISocketEvent(SocketEventType_Connect), otheraddr(addr), callback(_callback){}

	virtual void postEvent(const Public::Base::shared_ptr<Socket>& sock)
	{
		int sockfd = sock->getHandle();

		GUID connetEx = WSAID_CONNECTEX;
		DWORD bytes = 0;
		int ret = WSAIoctl(sockfd, SIO_GET_EXTENSION_FUNCTION_POINTER, &connetEx, sizeof(connetEx), &connectExFunc, sizeof(connectExFunc), &bytes, NULL, NULL);

		if (ret == SOCKET_ERROR)
		{
			connectExFunc = NULL;
			return;
		}
		if (false == connectExFunc(sockfd, (const sockaddr*)otheraddr.getAddr(), otheraddr.getAddrLen(), NULL, 0, NULL, &overlped))
		{
			int errorno = GetLastError();
			if (errorno != WSA_IO_PENDING)
			{
				return;
			}
		}
	}
	void doResultEvent(const Public::Base::shared_ptr<Socket>& sock, int bytes)
	{
		statusCallback(sock, true);

		UsingLocker locker(sock);
		if (locker.lock()) callback(sock);
	}
};

struct IOCPAcceptCanEvent :public ISocketEvent
{
#define SWAPBUFFERLEN			64
	char							swapBuffer[SWAPBUFFERLEN];
	int								newsock;

	LPFN_ACCEPTEX					acceptExFunc;
	LPFN_GETACCEPTEXSOCKADDRS		getAcceptAddExFunc;
	
	Socket::AcceptedCallback		acceptCalblack;
	Proactor::BuildSocketCallback	buildSocketCallback;

	IOCPAcceptCanEvent(const Socket::AcceptedCallback& _callback):ISocketEvent(SocketEventType_Accept), newsock(INVALID_SOCKET), acceptCalblack(_callback) {}

	virtual void postEvent(const Public::Base::shared_ptr<Socket>& sock)
	{
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

			return;
		}

		newsock = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, NULL, 0, WSA_FLAG_OVERLAPPED);
		if (newsock == INVALID_SOCKET)
		{
			return;
		}

		if (!acceptExFunc(sock->getHandle(), newsock, swapBuffer, 0, sizeof(SOCKADDR_IN) + 16, sizeof(SOCKADDR_IN) + 16, NULL, (LPOVERLAPPED)&overlped)
			&& GetLastError() != WSA_IO_PENDING)
		{
			return;
		}
	}

	void doResultEvent(const Public::Base::shared_ptr<Socket>& sock, int bytes)
	{
		int sockfd = sock->getHandle();

		::setsockopt(newsock, SOL_SOCKET, SO_UPDATE_ACCEPT_CONTEXT, (const char*)&sockfd, sizeof(SOCKET));

		SOCKADDR_IN* clientAddr = NULL, *localAddr = NULL;
		int clientlen = sizeof(SOCKADDR_IN), locallen = sizeof(SOCKADDR_IN);

		getAcceptAddExFunc(swapBuffer, 0, sizeof(SOCKADDR_IN) + 16, sizeof(SOCKADDR_IN) + 16, (LPSOCKADDR*)&localAddr, &locallen, (LPSOCKADDR*)&clientAddr, &clientlen);

		NetAddr otheraaddr = NetAddr(*(SockAddr*)clientAddr);
		
		Public::Base::shared_ptr<Socket> newsockobj = buildSocketCallback(newsock, otheraaddr);
		if (newsockobj == NULL) return;

		newsock = INVALID_SOCKET;

		UsingLocker locker(sock);
		if (acceptCalblack != NULL && locker.lock()) acceptCalblack(sock, newsockobj);
	}
};

struct IOCPSendCanEvent :public ISocketEvent
{
	WSABUF			wbuf;
	Socket::SendedCallback sendcallback;
	NetAddr			otheraddr;
	DWORD			dwBytes;

	IOCPSendCanEvent(const char* buffer, int bufferlen, const Socket::SendedCallback& callback, const NetAddr& addr):ISocketEvent(SocketEventType_Send)
	{
		wbuf.buf = (CHAR*)buffer;
		wbuf.len = bufferlen;
		sendcallback = callback;
		otheraddr = addr;
		dwBytes = 0;
	}
	virtual void postEvent(const Public::Base::shared_ptr<Socket>& sock)
	{
		int sockfd = sock->getHandle();
		if (sock->getNetType() != NetType_Udp)
		{
			int ret = ::WSASend(sockfd, &wbuf, 1, &dwBytes, 0, &overlped, NULL);
			if (ret == SOCKET_ERROR && GetLastError() != WSA_IO_PENDING)
			{
				return;
			}
		}
		else
		{
			int ret = ::WSASendTo(sockfd, &wbuf, 1, &dwBytes, 0, (sockaddr*)otheraddr.getAddr(), otheraddr.getAddrLen(), &overlped, NULL);
			if (ret == SOCKET_ERROR && GetLastError() != WSA_IO_PENDING)
			{
				return;
			}
		}
	}

	void doResultEvent(const Public::Base::shared_ptr<Socket>& sock, int bytes)
	{
		UsingLocker locker(sock);
		if (locker.lock()) sendcallback(sock, wbuf.buf, bytes);
	}
};

struct IOCPRecvCanEvent :public ISocketEvent
{
	WSABUF			wbuf;
	DWORD			dwBytes;
	DWORD			dwFlags;

	SOCKADDR		addr;
	int 			addrlen;

	Socket::ReceivedCallback	rcallback;
	Socket::RecvFromCallback	rfcallback;

	IOCPRecvCanEvent(const char* buffer, int bufferlen,
		const Socket::ReceivedCallback& _rcallback, const Socket::RecvFromCallback& _rfcalblack) :ISocketEvent(SocketEventType_Recv)
	{
		dwBytes = dwFlags = 0;
		wbuf.buf = (CHAR*)buffer;
		wbuf.len = bufferlen;
		rcallback = _rcallback;
		rfcallback = _rfcalblack;
	}

	virtual void postEvent(const Public::Base::shared_ptr<Socket>& sock)
	{
		int sockfd = sock->getHandle();

		if (sock->getNetType() != NetType_Udp)
		{
			int ret = WSARecv(sockfd, &wbuf, 1, &dwBytes, &dwFlags, &overlped, NULL);
			if (ret == SOCKET_ERROR && GetLastError() != WSA_IO_PENDING)
			{
				return ;
			}
		}
		else
		{
			memset(&addr, 0, sizeof(addr));
			addr.sa_family = AF_INET;
			addrlen = sizeof(SOCKADDR);

			int ret = WSARecvFrom(sockfd, &wbuf, 1, &dwBytes, &dwFlags, (sockaddr*)&addr, &addrlen, &overlped, NULL);
			if (ret == SOCKET_ERROR && GetLastError() != WSA_IO_PENDING)
			{
				return ;
			}
		}
	}
	void doResultEvent(const Public::Base::shared_ptr<Socket>& sock, int bytes)
	{
		UsingLocker locker(sock);
		if (locker.lock())
		{
			if (sock->getNetType() == NetType_Udp) rfcallback(sock, wbuf.buf, bytes,NetAddr(*(SockAddr*)&addr));
			else rcallback(sock, wbuf.buf, bytes);
		}
	}
};

class IOCPSocketObj:public Proactor_Object
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
	IOCPSocketObj(const Public::Base::shared_ptr<Socket>& _sock, const Public::Base::shared_ptr<Proactor>& _proactor,const Proactor::UpdateSocketStatusCallback& ccallback,const Proactor::BuildSocketCallback& acallback)
		:Proactor_Object(_proactor),sockobj(_sock), statusCallback(ccallback),buildSocketCallback(acallback){}
	virtual ~IOCPSocketObj() 
	{
		clean();
	}
	virtual bool clean()
	{
		{
			Guard locker(mutex);
			sockobj = Public::Base::weak_ptr<Socket>();

			connectEvent = NULL;
			acceptEvent = NULL;

			recvEvent = NULL;
			sendEvent = NULL;

			recvList.clear();
			sendList.clear();
		}

		disconnectCallback = NULL;
		statusCallback = NULL;
		buildSocketCallback = NULL;

		return true;
	}
	virtual bool result(OVERLAPPED* poverlaped, DWORD bytes) 
	{
		Public::Base::shared_ptr<Socket> sock = sockobj.lock();
		if (sock == NULL) return false;

		{
			Public::Base::shared_ptr<IOCPConnectCanEvent> connectmp;
			{
				Guard locker(mutex);
				if (connectEvent != NULL && &connectEvent->overlped == poverlaped)
				{
					connectmp = connectEvent;
					connectEvent = NULL;
					connectCallback = NULL;
				}
			}
			if (connectmp != NULL)
			{
				connectmp->doResultEvent(sock, bytes);
				
				return postEvent(ISocketEvent::SocketEventType_Connect);
			}
		}
		
		{
			shared_ptr<IOCPAcceptCanEvent> accepttmp;
			{
				Guard locker(mutex);
				if (acceptEvent != NULL && &acceptEvent->overlped == poverlaped)
				{
					accepttmp = acceptEvent;
					acceptEvent = NULL;
				}
			}
			if (accepttmp != NULL)
			{
				accepttmp->doResultEvent(sock, bytes);

				return postEvent(ISocketEvent::SocketEventType_Accept);
			}
		}
		
		{
			Public::Base::shared_ptr<IOCPSendCanEvent> sendtmp;
			{
				Guard locker(mutex);
				if (sendEvent != NULL && &sendEvent->overlped == poverlaped)
				{
					sendtmp = sendEvent;
					sendEvent = NULL;
				}
				
			}
			if (sendtmp != NULL)
			{
				sendtmp->doResultEvent(sock, bytes);
				
				return postEvent(ISocketEvent::SocketEventType_Send);
			}
		}
		
		{
			Public::Base::shared_ptr<IOCPRecvCanEvent> recvtmp;
			{
				Guard locker(mutex); 
				if (recvEvent != NULL && &recvEvent->overlped == poverlaped)
				{
					recvtmp = recvEvent;
					recvEvent = NULL;
				}
				
			}
			if (recvtmp != NULL)
			{
				if (bytes <= 0)
				{
					Proactor::UpdateSocketStatusCallback satustmp;
					Socket::DisconnectedCallback disconnectmp;
					{
						Guard locker(mutex);
						satustmp = statusCallback;
						disconnectmp = disconnectCallback;
					}
					satustmp(sock, false);

					UsingLocker locker(sock);
					if (locker.lock())	disconnectmp(sock, "disconnect");

					return true;
				}
				recvtmp->doResultEvent(sock, bytes);
				
				return postEvent(ISocketEvent::SocketEventType_Recv);
			}
		}	

		return true;
	}
	virtual bool addConnect(const NetAddr& _othreaddr, const Socket::ConnectedCallback& callback)
	{
		Public::Base::shared_ptr<Socket> sock = sockobj.lock();
		if (sock == NULL) return false;

		//iocp connect 必须bind端口
		if (!sock->getMyAddr().isValid())
		{
			NetAddr addr(50000 + Time::getCurrentMilliSecond() % 2000);
			sock->bind(addr);
		}

		{
			Guard locker(mutex);

			connectEvent = NULL;
			connectCallback = callback;
			othearAddr = _othreaddr;
		}
		
		return postEvent(ISocketEvent::SocketEventType_Connect);
	}
	virtual bool addAccept(const Socket::AcceptedCallback& callback)
	{
		{
			Guard locker(mutex);

			acceptEvent = NULL;
			acceptCallback = callback;
		}
		
		return postEvent(ISocketEvent::SocketEventType_Accept);
	}
	virtual bool addRecvfrom(char* buffer, int bufferlen, const Socket::RecvFromCallback& callback)
	{
		{
			Guard locker(mutex);

			RecvInfo info;
			info.buffer = buffer;
			info.bufferlen = bufferlen;
			info.recvfromCallback = callback;

			recvList.push_back(info);
		}

		return postEvent(ISocketEvent::SocketEventType_Recv);
	}
	virtual bool addRecv(char* buffer, int bufferlen, const Socket::ReceivedCallback& callback)
	{
		{
			Guard locker(mutex);

			RecvInfo info;
			info.buffer = buffer;
			info.bufferlen = bufferlen;
			info.recvCallback = callback;

			recvList.push_back(info);
		}

		return postEvent(ISocketEvent::SocketEventType_Recv);
	}
	virtual bool addSendto(const char* buffer, int bufferlen, const NetAddr& otheraddr, const Socket::SendedCallback& callback)
	{
		{
			Guard locker(mutex);

			SendInfo info;
			info.otheraddr = otheraddr;
			info.buffer = buffer;
			info.bufferlen = bufferlen;
			info.callback = callback;

			sendList.push_back(info);
		}

		return postEvent(ISocketEvent::SocketEventType_Send);
	}
	virtual bool addSend(const char* buffer, int bufferlen, const Socket::SendedCallback& callback)
	{
		{
			Guard locker(mutex);

			SendInfo info;
			info.buffer = buffer;
			info.bufferlen = bufferlen;
			info.callback = callback;
			
			sendList.push_back(info);
		}

		return postEvent(ISocketEvent::SocketEventType_Send);
	}
	virtual bool addDisconnect(const Socket::DisconnectedCallback& callback)
	{
		Guard locker(mutex);
		disconnectCallback = callback;

		return true;
	}
private:
	bool postEvent(ISocketEvent::SocketEventType type)
	{
		Guard locker(mutex);
		Public::Base::shared_ptr<Socket> sock = sockobj.lock();
		if (sock == NULL) return false;

		if(type == ISocketEvent::SocketEventType_Connect)
		{
			if(connectCallback != NULL) connectEvent = new IOCPConnectCanEvent(othearAddr, connectCallback);
			if(connectEvent != NULL)  connectEvent->postEvent(sock);
		}
		else if (type == ISocketEvent::SocketEventType_Accept)
		{
			if(acceptCallback != NULL)	acceptEvent = new IOCPAcceptCanEvent(acceptCallback);
			if(acceptEvent != NULL) acceptEvent->postEvent(sock);
		}
		else if (type == ISocketEvent::SocketEventType_Recv)
		{
			if (recvList.size() > 0 && recvEvent == NULL)
			{
				RecvInfo info = recvList.front();
				recvList.pop_front();
				recvEvent = new IOCPRecvCanEvent(info.buffer, info.bufferlen, info.recvCallback, info.recvfromCallback);
				recvEvent->postEvent(sock);
			}
		}
		else if (type == ISocketEvent::SocketEventType_Send)
		{
			if (sendList.size() > 0 && sendEvent == NULL)
			{
				SendInfo info = sendList.front();
				sendList.pop_front();
				sendEvent = new IOCPSendCanEvent(info.buffer, info.bufferlen, info.callback, info.otheraddr);
				sendEvent->postEvent(sock);
			}
		}
		else
		{
			return false;
		}

		return true;
	}
private:
	Mutex							mutex;
	Public::Base::weak_ptr<Socket>				sockobj;

	Socket::ConnectedCallback		connectCallback;
	NetAddr							othearAddr;

	Socket::AcceptedCallback		acceptCallback;

	Public::Base::shared_ptr<IOCPConnectCanEvent>	connectEvent;
	Public::Base::shared_ptr<IOCPAcceptCanEvent>	acceptEvent;

	Socket::DisconnectedCallback	disconnectCallback;
	std::list<RecvInfo>				recvList;
	Public::Base::shared_ptr<IOCPRecvCanEvent>	recvEvent;

	std::list<SendInfo>				sendList;
	Public::Base::shared_ptr<IOCPSendCanEvent>	sendEvent;

	Proactor::UpdateSocketStatusCallback	statusCallback;
	Proactor::BuildSocketCallback	buildSocketCallback;
};

class IOCPObject
{
	struct IOCPObjectInfo
	{
		Public::Base::shared_ptr<IOCPSocketObj>	obj;
	};
public:
	IOCPObject(int threadnum):iocpQuit(false)
	{
		iocpHandle = ::CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, 0);
		if (iocpHandle == NULL)
		{
			return;
		}
		threadnum = Proactor::buildThreadNum(threadnum);
		for (int i = 0; i < threadnum; i++)
		{
			Public::Base::shared_ptr<Thread> thread;
			thread = ThreadEx::creatThreadEx("IOCPObject", ThreadEx::Proc(&IOCPObject::threadProc,this), 0);
			thread->createThread();
			threadlist.push_back(thread);
		}
	}
	virtual ~IOCPObject() 
	{
		iocpQuit = true;
		for (uint32_t i = 0; iocpHandle != NULL && i < threadlist.size(); i++)
		{
			::PostQueuedCompletionStatus(iocpHandle, 0, NULL, NULL);
		}

		threadlist.clear();
		objectList.clear();
		if (iocpHandle != NULL)
		{
			CloseHandle(iocpHandle);
			iocpHandle = NULL;
		}
	}
	virtual bool addSocket(const Public::Base::shared_ptr<Socket>& sock,const Public::Base::shared_ptr<IOCPSocketObj>& obj)
	{
		int fd = sock->getHandle();

		IOCPObjectInfo info;
		info.obj = obj;
		{
			GuardWriteMutex locker(mutex);
			objectList[sock.get()] = info;
		}
		
		::CreateIoCompletionPort((HANDLE)fd, iocpHandle, (DWORD)sock.get(), 0);

		return true;
	}
	virtual void delSocket(Socket* _sockptr)
	{
		GuardWriteMutex locker(mutex);
		objectList.erase(_sockptr);
	}
private:
	void threadProc(Thread* param,void*)
	{
		while (!iocpQuit)
		{
			DWORD bytes = 0, key = 0;
			OVERLAPPED* poverlaped = NULL;

			if (!GetQueuedCompletionStatus(iocpHandle, &bytes, &key, &poverlaped, INFINITE))
			{
				continue;
			}
			if (key == 0) break;

			IOCPObjectInfo info;
			{
				GuardReadMutex locker(mutex);
				std::map<Socket*, IOCPObjectInfo>::iterator iter = objectList.find((Socket*)key);
				if (iter != objectList.end())
				{
					info = iter->second;
				}
			}
			if(info.obj == NULL) continue;
			
			info.obj->result(poverlaped, bytes);
		}
	}
private:
	HANDLE							iocpHandle;
	std::list<Public::Base::shared_ptr<Thread> >	threadlist;
	bool							iocpQuit;

	ReadWriteMutex					mutex;
	std::map<Socket*,IOCPObjectInfo>objectList;
};


class Proactor_IOCP :public Proactor
{
public:
	//threadnum 线程数
	Proactor_IOCP(const UpdateSocketStatusCallback& _statusCallback, const BuildSocketCallback& buildCallback, uint32_t threadnum) 
		:Proactor(_statusCallback,buildCallback,threadnum) 
	{
		iocp = new IOCPObject(threadnum);
	}
	virtual ~Proactor_IOCP() 
	{
		iocp = NULL;
	}
	virtual int createSocket(NetType _type)
	{
		int domain = (_type != NetType_Udp) ? SOCK_STREAM : SOCK_DGRAM;
		int protocol = (_type != NetType_Udp) ? IPPROTO_TCP : IPPROTO_UDP;
		int sockfd = WSASocket(AF_INET, domain, protocol, NULL, 0, WSA_FLAG_OVERLAPPED);
		if (sockfd <= 0)
		{
			perror("WSASocket");			
		}

		return sockfd;
	}
	//添加socket
	virtual Public::Base::shared_ptr<Proactor_Object> addSocket(const Public::Base::shared_ptr<Socket>& sock)
	{
		Public::Base::shared_ptr<Proactor_Object> obj;
		obj = new IOCPSocketObj(sock, shared_from_this(), statusCallback, buildcallback);

		iocp->addSocket(sock, obj);

		return obj;
	}
	//删除socket
	virtual bool delSocket(const Public::Base::shared_ptr<Proactor_Object>& obj, Socket* sockptr)
	{
		iocp->delSocket(sockptr);

		IOCPSocketObj* iocpobj = (IOCPSocketObj*)obj.get();
		iocpobj->clean();

		return true;
	}

	virtual bool addConnect(const Public::Base::shared_ptr<Proactor_Object>& obj, const NetAddr& othreaddr, const Socket::ConnectedCallback& callback)
	{
		IOCPSocketObj* iocpobj = (IOCPSocketObj*)obj.get();
		return iocpobj->addConnect(othreaddr, callback);
	}
	virtual bool addAccept(const Public::Base::shared_ptr<Proactor_Object>& obj, const Socket::AcceptedCallback& callback)
	{
		IOCPSocketObj* iocpobj = (IOCPSocketObj*)obj.get();
		return iocpobj->addAccept(callback);
	}
	virtual bool addRecvfrom(const Public::Base::shared_ptr<Proactor_Object>& obj, char* buffer, int bufferlen, const Socket::RecvFromCallback& callback)
	{
		IOCPSocketObj* iocpobj = (IOCPSocketObj*)obj.get();
		return iocpobj->addRecvfrom(buffer,bufferlen, callback);
	}
	virtual bool addRecv(const Public::Base::shared_ptr<Proactor_Object>& obj, char* buffer, int bufferlen, const Socket::ReceivedCallback& callback)
	{
		IOCPSocketObj* iocpobj = (IOCPSocketObj*)obj.get();
		return iocpobj->addRecv(buffer, bufferlen, callback);
	}
	virtual bool addSendto(const Public::Base::shared_ptr<Proactor_Object>& obj, const char* buffer, int bufferlen, const NetAddr& otheraddr, const Socket::SendedCallback& callback)
	{
		IOCPSocketObj* iocpobj = (IOCPSocketObj*)obj.get();
		return iocpobj->addSendto(buffer, bufferlen, otheraddr,callback);
	}
	virtual bool addSend(const Public::Base::shared_ptr<Proactor_Object>& obj, const char* buffer, int bufferlen, const Socket::SendedCallback& callback)
	{
		IOCPSocketObj* iocpobj = (IOCPSocketObj*)obj.get();
		return iocpobj->addSend(buffer, bufferlen, callback);
	}
	virtual bool addDisconnect(const Public::Base::shared_ptr<Proactor_Object>& obj, const Socket::DisconnectedCallback& callback)
	{
		IOCPSocketObj* iocpobj = (IOCPSocketObj*)obj.get();
		return iocpobj->addDisconnect(callback);
	}
private:
	Public::Base::shared_ptr<IOCPObject> iocp;
};
#endif
#endif //__SIMPEIO_PROACTOR_IOCP_H__
