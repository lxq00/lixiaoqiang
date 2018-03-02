#ifndef __PROACTOR_REACTOR_H__
#define __PROACTOR_REACTOR_H__
#include "proactor.h"
#include "reactor_epoll.h"
#include "reactor_poll.h"
#include "reactor_select.h"

class Proactor_Reactor;
class Proactor_Object_Reactor :public Reactor_Object,public Proactor_Object
{
public:
	Proactor_Object_Reactor(const Proactor::UpdateSocketStatusCallback& _statusCallback, const Proactor::BuildSocketCallback& _buildCallback, const Public::Base::shared_ptr<Socket>& _sock, const Public::Base::shared_ptr<Proactor>& _proactor)
		:Reactor_Object(_sock),Proactor_Object(_proactor),statusCallback(_statusCallback), buildcallback(_buildCallback){}
	~Proactor_Object_Reactor() {}

	Public::Base::shared_ptr<Socket> buildSocketBySock(int newsock, const NetAddr& otheraaddr)
	{
		return buildcallback(newsock, otheraaddr);
	}
	void updateSocketStatus(const Public::Base::shared_ptr<Socket>& sock, bool status)
	{
		return statusCallback(sock, status);
	}
private:
	Proactor::UpdateSocketStatusCallback statusCallback;
	Proactor::BuildSocketCallback		buildcallback;
};

class Proactor_Reactor :public Proactor
{
public:
	class Proactor_ReactorObject :public Proactor
	{
	public:
		Proactor_ReactorObject():Proactor(NULL,NULL,0)
		{
#ifdef SIMPLEIO_SUPPORT_EPOLL
			reactor = new Reactor_Epoll();
#elif defined SIMPLEIO_SUPPORT_POLL
			reactor = new Reactor_Poll();
#elif defined SIMPLEIO_SUPPORT_SELECT
			reactor = new Reactor_Select();
#endif
		}
		virtual ~Proactor_ReactorObject()
		{
			reactor = NULL;
		}

		virtual bool delSocket(const Public::Base::shared_ptr<Proactor_Object>& obj, Socket* sockptr)
		{
			Proactor_Object_Reactor* object = (Proactor_Object_Reactor*)obj.get();
			if (reactor == NULL || object == NULL) return false;
			object->clean();

			return true;
		}
		virtual bool addConnect(const Public::Base::shared_ptr<Proactor_Object>& obj, const NetAddr& othreaddr, const Socket::ConnectedCallback& callback)
		{
			Proactor_Object_Reactor* object = (Proactor_Object_Reactor*)obj.get();
			if (reactor == NULL || object == NULL) return false;

			return object->addConnect(othreaddr, callback);
		}
		virtual bool addAccept(const Public::Base::shared_ptr<Proactor_Object>& obj, const Socket::AcceptedCallback& callback)
		{
			Proactor_Object_Reactor* object = (Proactor_Object_Reactor*)obj.get();
			if (reactor == NULL || object == NULL) return false;

			return object->addAccept(callback);
		}
		virtual bool addRecvfrom(const Public::Base::shared_ptr<Proactor_Object>& obj, char* buffer, int bufferlen, const Socket::RecvFromCallback& callback)
		{
			Proactor_Object_Reactor* object = (Proactor_Object_Reactor*)obj.get();
			if (reactor == NULL || object == NULL) return false;

			return object->addRecvfrom(buffer,bufferlen,callback);
		}
		virtual bool addRecv(const Public::Base::shared_ptr<Proactor_Object>& obj, char* buffer, int bufferlen, const Socket::ReceivedCallback& callback)
		{
			Proactor_Object_Reactor* object = (Proactor_Object_Reactor*)obj.get();
			if (reactor == NULL || object == NULL) return false;

			return object->addRecv(buffer, bufferlen, callback);
		}
		virtual bool addSendto(const Public::Base::shared_ptr<Proactor_Object>& obj, const char* buffer, int bufferlen, const NetAddr& otheraddr, const Socket::SendedCallback& callback)
		{
			Proactor_Object_Reactor* object = (Proactor_Object_Reactor*)obj.get();
			if (reactor == NULL || object == NULL) return false;

			return object->addSendto(buffer, bufferlen, otheraddr,callback);
		}
		virtual bool addSend(const Public::Base::shared_ptr<Proactor_Object>& obj, const char* buffer, int bufferlen, const Socket::SendedCallback& callback)
		{
			Proactor_Object_Reactor* object = (Proactor_Object_Reactor*)obj.get();
			if (reactor == NULL || object == NULL) return false;

			return object->addSend(buffer, bufferlen, callback);
		}
		virtual bool addDisconnect(const Public::Base::shared_ptr<Proactor_Object>& obj, const Socket::DisconnectedCallback& callback)
		{
			Proactor_Object_Reactor* object = (Proactor_Object_Reactor*)obj.get();
			if (reactor == NULL || object == NULL) return false;

			return object->addDisconnect(callback);
		}
		long long userRef() { return reactor == NULL ? 0xfffffff: reactor->userRef(); }
		Public::Base::shared_ptr<Reactor> reactorObj(){ return reactor; }
	private:
		virtual int createSocket(NetType type) { return -1; }
		Public::Base::shared_ptr<Proactor_Object> addSocket(const Public::Base::shared_ptr<Socket>& sock) { return Public::Base::shared_ptr<Proactor_Object>(); }
	private:
		Public::Base::shared_ptr<Reactor>		reactor;
	};
public:
	//threadnum 线程数
	Proactor_Reactor(const UpdateSocketStatusCallback& _statusCallback, const BuildSocketCallback& buildCallback, uint32_t threadnum)
		:Proactor(_statusCallback,buildCallback,threadnum)
	{
		threadnum = buildThreadNum(threadnum);

		for (uint32_t i = 0; i < threadnum; i++)
		{
			Public::Base::shared_ptr<Proactor_ReactorObject> obj(new Proactor_ReactorObject());
			reactorList.push_back(obj);
		}
	}
	virtual ~Proactor_Reactor()
	{
		reactorList.clear();
	}
	virtual int createSocket(NetType _type)
	{
		int domain = (_type != NetType_Udp) ? SOCK_STREAM : SOCK_DGRAM;
		int sockfd = socket(AF_INET, domain, 0);
		if (sockfd <= 0)
		{
			perror("socket");
			return -1;
		}
	
		{
			int sendtimeouttmp = 100;
			setsockopt(sockfd, SOL_SOCKET, SO_SNDTIMEO, (const char*)&sendtimeouttmp, sizeof(sendtimeouttmp));
			setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, (const char*)&sendtimeouttmp, sizeof(sendtimeouttmp));
		}
		return sockfd;
	}
	//添加socket
	virtual Public::Base::shared_ptr<Proactor_Object> addSocket(const Public::Base::shared_ptr<Socket>& sock)
	{
		Public::Base::shared_ptr<Proactor_ReactorObject> obj;
		int nowuserref = 0;
		for (uint32_t i = 0; i < reactorList.size(); i++)
		{
			if (obj == NULL || nowuserref > reactorList[i]->userRef())
			{
				obj = reactorList[i];
				nowuserref = (int)obj->userRef();
			}
		}


		Public::Base::shared_ptr<Proactor_Object_Reactor> reactor(new Proactor_Object_Reactor(statusCallback,buildcallback, sock, obj));
		reactor->addReactor(obj->reactorObj());

		return reactor;
	}	
private:
	//删除socket
	virtual bool delSocket(const Public::Base::shared_ptr<Proactor_Object>& obj, Socket* sockptr) { return false; }
	virtual bool addConnect(const Public::Base::shared_ptr<Proactor_Object>& obj, const NetAddr& othreaddr, const Socket::ConnectedCallback& callback) { return false; }
	virtual bool addAccept(const Public::Base::shared_ptr<Proactor_Object>& obj, const Socket::AcceptedCallback& callback) { return false; }
	virtual bool addRecvfrom(const Public::Base::shared_ptr<Proactor_Object>& obj, char* buffer, int bufferlen, const Socket::RecvFromCallback& callback) { return false; }
	virtual bool addRecv(const Public::Base::shared_ptr<Proactor_Object>& obj, char* buffer, int bufferlen, const Socket::ReceivedCallback& callback) { return false; }
	virtual bool addSendto(const Public::Base::shared_ptr<Proactor_Object>& obj, const char* buffer, int bufferlen, const NetAddr& otheraddr, const Socket::SendedCallback& callback) { return false; }
	virtual bool addSend(const Public::Base::shared_ptr<Proactor_Object>& obj, const char* buffer, int bufferlen, const Socket::SendedCallback& callback) { return false; }
	virtual bool addDisconnect(const Public::Base::shared_ptr<Proactor_Object>& obj, const Socket::DisconnectedCallback& callback) { return false; }
private:
	std::vector<Public::Base::shared_ptr<Proactor_ReactorObject> > reactorList;
};

#endif //__PROACTOR_REACTOR_H__
