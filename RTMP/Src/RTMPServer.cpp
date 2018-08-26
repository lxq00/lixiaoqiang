#include "RTMP/RTMPServer.h"
#include "libRTMPServer.h"

namespace Public {
namespace RTMP {

struct RTMPServer::Publish::PublishInternal
{
	RTMPServer::Publish*					ptr;
	RTMPServer::Publish::DisconnectCallback discallback;
	RTMPServer::Publish::ReadCallback		readcallback;
	shared_ptr<LibRTMPServer>				server;

	void rtmpDiscallback()
	{
		discallback(ptr);
	}

	void rtmpReadCallback(RTMPType type, const void* data, size_t bytes, uint32_t timestamp)
	{
		PacketInfo info;
		info.data = (char*)data;
		info.size = bytes;
		info.type = type;
		info.timestamp = timestamp;

		readcallback(ptr, info);
	}
};

RTMPServer::Publish::Publish(void* param)
{
	internal = new PublishInternal;
	internal->server = *(shared_ptr<LibRTMPServer>*)param;
	internal->ptr = this;
}
RTMPServer::Publish::~Publish()
{
	internal->server = NULL;
	SAFE_DELETE(internal);
}
void RTMPServer::Publish::setCallback(const ReadCallback& readcallback, const DisconnectCallback& discallback)
{
	internal->discallback = discallback;
	internal->readcallback = readcallback;

	internal->server->setDisconnect(LibRTMPServer::DisconnectCallback(&PublishInternal::rtmpDiscallback, internal));
	internal->server->setPublishCallback(LibRTMPServer::ReadCallback(&PublishInternal::rtmpReadCallback, internal));
}

struct RTMPServer::Server::ServerInternal
{
	RTMPServer::Server*					ptr;
	RTMPServer::Server::DisconnectCallback discallback;
	RTMPServer::Server::PauseCallback		pausecallback;
	RTMPServer::Server::SeekCallback		seekcallback;
	shared_ptr<LibRTMPServer>				server;

	void rtmpDiscallback()
	{
		discallback(ptr);
	}

	void rtmppauseCallback(bool pause)
	{
		pausecallback(ptr,pause);
	}

	void rtmpseekCallback(uint32_t ms)
	{
		seekcallback(ptr, ms);
	}
};

RTMPServer::Server::Server(void* param)
{
	internal = new ServerInternal;
	internal->server = *(shared_ptr<LibRTMPServer>*)param;
	internal->ptr = this;
}

RTMPServer::Server::~Server()
{
	internal->server = NULL;
	SAFE_DELETE(internal);
}

void RTMPServer::Server::setCallback(const DisconnectCallback& discallback, const PauseCallback& pause, const SeekCallback& seek)
{
	internal->discallback = discallback;
	internal->pausecallback = pause;
	internal->seekcallback = seek;

	internal->server->setDisconnect(LibRTMPServer::DisconnectCallback(&ServerInternal::rtmpDiscallback, internal));
	internal->server->setServerCalblack(LibRTMPServer::PauseCallback(&ServerInternal::rtmppauseCallback, internal), LibRTMPServer::SeekCallback(&ServerInternal::rtmpseekCallback, internal));
}
bool RTMPServer::Server::writePacket(RTMPType type, uint32_t timestmap /*ms*/, const char* buffer, int maxlen)
{
	return internal->server->writePacket(type, timestmap, buffer, maxlen);
}

struct RTMPServer::RTMPServerInternal
{
	shared_ptr<Timer>			pooltimer;
	shared_ptr<Socket>			listensock;


	struct ServerInfo
	{
		shared_ptr<LibRTMPServer>	server;
		uint64_t					initTime;
	};

	Mutex						mutex;


	std::map<LibRTMPServer*, ServerInfo> servermap;

	PublishCallback				publishCallback;
	ServerCallback				serverCallback;


	void rtmpPlayRequestCallback(LibRTMPServer* serverptr, bool playreq,const LibRTMPServer::ReqInfo& info)
	{
		shared_ptr<LibRTMPServer> server;
		{
			Guard locker(mutex);
			std::map<LibRTMPServer*, ServerInfo>::iterator iter = servermap.find(serverptr);
			if (iter == servermap.end())
			{
				return;
			}
			server = iter->second.server;
			servermap.erase(iter);
		}
		if (playreq)
		{
			shared_ptr<Server> serverobj = shared_ptr<Server>(new Server(&server));
			serverCallback(serverobj,info.app,info.stream,info.start,info.duration,info.reset);
		}
		else
		{
			shared_ptr<Publish> publishobj = shared_ptr<Publish>(new Publish(&server));
			publishCallback(publishobj, info.app, info.stream, info.type);
		}
	}

	void socketAcceptCallback(const shared_ptr<Socket>& oldsock, const shared_ptr<Socket>& newsock)
	{
		Guard locker(mutex);

		ServerInfo info;
		info.server = make_shared<LibRTMPServer>(newsock, LibRTMPServer::PlayCallback(&RTMPServerInternal::rtmpPlayRequestCallback, this));
		info.initTime = Time::getCurrentMilliSecond();
		servermap[info.server.get()] = info;
	}

	void onPoolTimerProc(unsigned long)
	{
#define CONNECTTIMEOUT	60*1000

		Guard locker(mutex);
		uint64_t nowtime = Time::getCurrentMilliSecond();
		for (std::map<LibRTMPServer*, ServerInfo>::iterator iter = servermap.begin(); iter != servermap.end();)
		{
			if (nowtime < iter->second.initTime || nowtime > iter->second.initTime + CONNECTTIMEOUT)
			{
				servermap.erase(iter++);
			}
			else
			{
				iter++;
			}
		}

	}
};

RTMPServer::RTMPServer(uint32_t port, const shared_ptr<IOWorker>& worker)
{
	internal = new RTMPServerInternal;
	internal->listensock = TCPServer::create(worker, port);

	internal->pooltimer = make_shared<Timer>("RTMPServer");
	internal->pooltimer->start(Timer::Proc(&RTMPServerInternal::onPoolTimerProc, internal), 0, 10000);
}
RTMPServer::~RTMPServer()
{
	internal->pooltimer = NULL;
	internal->listensock = NULL;

	SAFE_DELETE(internal);
}

bool RTMPServer::start(const PublishCallback& pcallback, const ServerCallback& scallback)
{
	internal->publishCallback = pcallback;
	internal->serverCallback = scallback;

	internal->listensock->async_accept(Socket::AcceptedCallback(&RTMPServerInternal::socketAcceptCallback, internal));

	return true;
}

bool RTMPServer::stop()
{
	internal->listensock = NULL;

	return true;
}


}
}
