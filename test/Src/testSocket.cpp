#include "Network/Network.h"
using namespace Public::Network;
#if 0

class NetworkServerInfo
{
public:
	Mutex				mutex;
	std::list<String>	sendlist;
	shared_ptr<Socket>	sock;

	void inputData(const String& data)
	{
		Guard locker(mutex);
		sendlist.push_back(data);

		if (sendlist.size() == 1)
		{
			const char* buffer = sendlist.front().c_str();
			uint32_t bufferlen = sendlist.front().length();

			shared_ptr<Socket> tmp = sock;
			if (tmp) tmp->async_send(buffer, bufferlen, Socket::SendedCallback(&NetworkServerInfo::_sendcallback, this));
		}
	}

	void _sendcallback(const weak_ptr<Socket>&, const char* buffer, int len)
	{
		Guard locker(mutex);
		if (sendlist.size() <= 0)return;

		sendlist.pop_front();

		if (sendlist.size() > 0)
		{
			const char* buffer = sendlist.front().c_str();
			uint32_t bufferlen = sendlist.front().length();

			shared_ptr<Socket> tmp = sock;
			if (tmp) tmp->async_send(buffer, bufferlen, Socket::SendedCallback(&NetworkServerInfo::_sendcallback, this));
		}
	}
};

Mutex servermutex;
std::map<Socket*, shared_ptr<NetworkServerInfo> > serverlist;
shared_ptr<Thread>	serverthreadex;
shared_ptr<Socket>	tcpserver;
std::list< shared_ptr<NetworkServerInfo>> freelist;

void _socketRecv(const weak_ptr<Socket>& sock, const char* buffer, int len)
{
	shared_ptr<Socket> socktmp = sock.lock();
	if (socktmp) socktmp->async_recv(_socketRecv);
}
void _socketDisconnect(const weak_ptr<Socket>& sock, const std::string& err)
{
	shared_ptr<Socket> socktmp = sock.lock();
	if (socktmp == NULL) return;

	{
		Guard locker(servermutex);
		std::map<Socket*, shared_ptr<NetworkServerInfo> >::iterator iter = serverlist.find(socktmp.get());
		if (iter != serverlist.end())
		{
			freelist.push_back(iter->second);
			serverlist.erase(iter);
		}
	}

}

void _socketAccpet(const weak_ptr<Socket>&sock, const shared_ptr<Socket>& newsock)
{
	{
		shared_ptr<NetworkServerInfo> info = make_shared<NetworkServerInfo>();
		info->sock = newsock;
		info->sock->async_recv(_socketRecv);
		info->sock->setDisconnectCallback(_socketDisconnect);

		Guard locker(servermutex);
		serverlist[newsock.get()] = info;
	}
	
	shared_ptr<Socket> socktmp = sock.lock();
	if (socktmp == NULL) return;
	socktmp->async_accept(_socketAccpet);
}

void runServerThread(Thread* ex, void* param)
{
	String data;
	data.alloc(10240);
	data.resize(10240);

	while (ex->looping())
	{
		Thread::sleep(5);


		std::list< shared_ptr<NetworkServerInfo>> freelisttmp;
		std::map<Socket*, shared_ptr<NetworkServerInfo> > serverlisttmp;
		{
			Guard lock(servermutex);
			serverlisttmp = serverlist;

			freelisttmp = freelist;
			freelist.clear();
		}

		for (std::map<Socket*, shared_ptr<NetworkServerInfo> >::iterator iter = serverlisttmp.begin(); iter != serverlisttmp.end(); iter++)
		{
			shared_ptr<NetworkServerInfo> serverinfo = iter->second;
			if (serverinfo == NULL) continue;;

			serverinfo->inputData(data);
		}

	}
}

void runServer(const shared_ptr<IOWorker>& worker)
{
	tcpserver = TCPServer::create(worker, 4444);
	tcpserver->async_accept(_socketAccpet);

	serverthreadex = ThreadEx::creatThreadEx("", runServerThread, NULL);
	serverthreadex->createThread();
}


std::map<Socket*, shared_ptr<Socket> > clientlist;

void _socketRecvcallback(const weak_ptr<Socket>& sock, const char* buf, int len)
{
	shared_ptr<Socket> socktmp = sock.lock();
	if (socktmp == NULL) return;

	socktmp->async_recv(_socketRecvcallback);
}

void _socketConnectCallback(const weak_ptr<Socket>& sock,bool,const std::string&)
{
	shared_ptr<Socket> socktmp = sock.lock();
	if (socktmp == NULL) return;

	socktmp->async_recv(_socketRecvcallback);
}

void runClient(const shared_ptr<IOWorker>& worker,uint32_t size)
{
	for (uint32_t i = 0; i < size; i++)
	{
		shared_ptr<Socket> sock = TCPClient::create(worker);
		clientlist[sock.get()] = sock;

		sock->async_connect(NetAddr("192.168.2.46", 4444), _socketConnectCallback);

		Thread::sleep(100);
	}
}


int main(int args,const char* argv[])
{
	shared_ptr<IOWorker>	worker = make_shared<IOWorker>(4);
	
	if (args == 1)
	{
		runServer(worker);
	}
	else
	{
		runClient(worker, 100);
	}


	getchar();

	return 0;
}

#endif