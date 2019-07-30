#include "RTSP/RTSPServer.h"
#include "RTSPServerSession.h"

using namespace Public::RTSP;

#define MAXRTSPCONNECTIONTIMEOUT	60*1000

struct RTSPServer::RTSPServerInternal
{
	std::string							useragent;

	shared_ptr<IOWorker>				ioworker;
	shared_ptr<Socket>					tcpserver;

	uint32_t							port;

	Mutex								mutex;
	std::map<Socket*, shared_ptr<RTSPServerSession> >	serverlist;

	ListenCallback						listencallback;

	shared_ptr<Timer>					pooltimer;

	uint32_t					udpstartport;
	uint32_t					udpstopport;
	uint32_t					nowudpport;

	RTSPServerInternal() :udpstartport(40000), udpstopport(4000), nowudpport(udpstartport) {}

	uint32_t allockRTPPort()
	{
		uint32_t udpport = nowudpport;
		uint32_t allocktimes = 0;

		while (allocktimes < udpstopport - udpstartport)
		{
			if (Host::checkPortIsNotUsed(udpport, Host::SocketType_UDP)
				&& Host::checkPortIsNotUsed(udpport + 1, Host::SocketType_UDP)
				&& Host::checkPortIsNotUsed(udpport + 2, Host::SocketType_UDP)
				&& Host::checkPortIsNotUsed(udpport + 3, Host::SocketType_UDP))
			{
				nowudpport = udpport + 1;

				return nowudpport;
			}

			udpport++;
			allocktimes++;
			if (udpport > udpstopport - 4) udpport = udpstartport;
		}

		return udpstartport;
	}

	void onpooltimerproc(unsigned long)
	{
		std::list< shared_ptr<RTSPServerSession> > freelisttmp;
		{
			Guard locker(mutex); 

			uint64_t nowtime = Time::getCurrentMilliSecond();

			for (std::map<Socket*, shared_ptr<RTSPServerSession> >::iterator iter = serverlist.begin(); iter != serverlist.end();)
			{
				uint64_t prevtime = iter->second->prevAlivetime();
				if (prevtime == 0 || (nowtime > prevtime && nowtime - prevtime > MAXRTSPCONNECTIONTIMEOUT))
				{
					freelisttmp.push_back(iter->second);
					serverlist.erase(iter++);
				}
				else
				{
					iter++;
				}
			}
		}

		for (std::list< shared_ptr<RTSPServerSession> >::iterator iter = freelisttmp.begin(); iter != freelisttmp.end(); iter++)
		{
			shared_ptr<RTSPServerHandler> handler = (*iter)->handler();
			if (handler)
			{
				handler->onClose(*iter);
			}
			(*iter)->disconnect();
		}
	}
	void onsocketaccept(const weak_ptr<Socket>& sock, const shared_ptr<Socket>& newsock)
	{
		{
			shared_ptr<RTSPServerSession> session = shared_ptr<RTSPServerSession>(new  RTSPServerSession(ioworker,newsock, listencallback, AllockUdpPortCallback(&RTSPServerInternal::allockRTPPort,this), useragent));
			session->initRTSPServerSessionPtr(session);


			Guard locker(mutex);
			serverlist[newsock.get()] = session;
		}
		

		shared_ptr<Socket> socktmp = sock.lock();
		if (socktmp != NULL)
		{
			socktmp->async_accept(Socket::AcceptedCallback(&RTSPServerInternal::onsocketaccept, this));
		}
	}
};


RTSPServer::RTSPServer(const shared_ptr<IOWorker>& worker, const std::string& useragent)
{
	internal = new RTSPServerInternal();

	internal->ioworker = worker;
	internal->useragent = useragent;
	internal->pooltimer = make_shared<Timer>("RTSPServer");
	internal->pooltimer->start(Timer::Proc(&RTSPServerInternal::onpooltimerproc, internal),0,1000);

	if (internal->ioworker == NULL) internal->ioworker = IOWorker::defaultWorker();
}
RTSPServer::~RTSPServer()
{
	stop();

	SAFE_DELETE(internal);
}

bool RTSPServer::run(uint32_t port, const ListenCallback& callback)
{
	internal->listencallback = callback;
	internal->port = port;

	if (internal->ioworker == NULL || internal->port == 0) return false;

	internal->tcpserver = TCPServer::create(internal->ioworker, port);
	internal->tcpserver->async_accept(Socket::AcceptedCallback(&RTSPServerInternal::onsocketaccept, internal));


	internal->pooltimer = make_shared<Timer>("RTSPServer");
	internal->pooltimer->start(Timer::Proc(&RTSPServerInternal::onpooltimerproc, internal), 0, 1000);

	return true;
}
bool RTSPServer::stop()
{
	internal->pooltimer = NULL;
	if (internal->tcpserver) internal->tcpserver->disconnect();
	internal->tcpserver = NULL;

	return true;
}
uint32_t RTSPServer::listenPort() const
{
	return internal->port;
}
