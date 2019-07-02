#pragma once

#include "Base/Base.h"
#include "Network/Network.h"
#include "connection.h"
using namespace Public::Base;
using namespace Public::Network;


typedef Function1<void, void*> ConnectionDisconnectCallback;

class Communication
{
public:
	Communication(const shared_ptr<IOWorker>& worker,const RecvDataCallback& callback,const ConnectionDisconnectCallback& discallback,uint32_t port) 
		:recvdatacallback(callback),disconnectcallback(discallback)
	{
		tcpserver = TCPServer::create(worker, port);
		tcpserver->async_accept(Socket::AcceptedCallback(&Communication::acceptCallback, this));

		pooltimer = make_shared<Timer>("Communication");
		pooltimer->start(Timer::Proc(&Communication::onPoolTimerproc, this), 0, 5*1000);
	}
	~Communication()
	{
		pooltimer = NULL;
		tcpserver = NULL;
	}

	shared_ptr<Connection> getConnection(void* user)
	{
		Guard locker(mutex);
		std::map<void*, shared_ptr<Connection> >::iterator iter = connectlist.find(user);
		if (iter == connectlist.end()) return shared_ptr<Connection>();

		return iter->second;
	}
private:
	void acceptCallback(const weak_ptr<Socket>& oldsock, const shared_ptr<Socket>& newsock)
	{
		newsock->setDisconnectCallback(Socket::DisconnectedCallback(&Communication::socketDisconnectCallback, this));


		shared_ptr<Connection> connection = make_shared<Connection>(newsock, recvdatacallback);
		{
			Guard locker(mutex);
			connectlist[newsock.get()] = connection;
		}
		
		shared_ptr<Socket> tmp = tcpserver;
		if (tmp != NULL)
		{
			tmp->async_accept(Socket::AcceptedCallback(&Communication::acceptCallback, this));
		}
	}
	void socketDisconnectCallback(const weak_ptr<Socket>& user, const std::string& errmsg)
	{
		shared_ptr < Socket> tmp = user.lock();
		if (tmp == NULL) return;

		{
			Guard locker(mutex);
			std::map<void*, shared_ptr<Connection> >::iterator iter = connectlist.find(tmp.get());
			if (iter != connectlist.end())
			{
				errlist.push_back(iter->second);
				connectlist.erase(iter);
			}
		}
		disconnectcallback(tmp.get());
	}
	void onPoolTimerproc(unsigned long)
	{
		std::list<shared_ptr<Connection> > errlisttmp;
		{
			Guard locker(mutex);

			for (std::map<void*, shared_ptr<Connection> >::iterator iter = connectlist.begin(); iter != connectlist.end();)
			{
				if (iter->second->isTimeoutOrError())
				{
					errlist.push_back(iter->second);
					connectlist.erase(iter++);
				}
				else
				{
					iter++;
				}
			}

			errlisttmp = errlist;
			errlist.clear();
		}
	}
private:
	RecvDataCallback							recvdatacallback;
	ConnectionDisconnectCallback				disconnectcallback;

	shared_ptr<Timer>							pooltimer;
	Mutex										mutex;
	std::map<void*, shared_ptr<Connection> >	connectlist;
	std::list<shared_ptr<Connection> >			errlist;
	shared_ptr<Socket>							tcpserver;
};