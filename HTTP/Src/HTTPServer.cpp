#include "HTTP/HTTPServer.h"
#include "HTTPCommunication.h"
#include "boost/regex.hpp" 
namespace Public {
namespace HTTP {


#define COMMUFINISHTIMEOUT		5*1000
#define NOSESSIONTIMEOUT		2*60*1000

struct ListenInfo
{
	HTTPServer::HTTPCallback	callback;
	HTTPCacheType				type;
};

struct HTTPSessionInfo
{
	shared_ptr<HTTPCommunication>		commu;
	weak_ptr<WebSocketServerSession>	websocketsession;
	shared_ptr<HTTPServerSession>		httpsession;
	weak_ptr<HTTPServerSession>			httpsessionback;
	ListenInfo							listeninfo;
	uint64_t							finishtime;
	uint64_t							nosessiontime;

	HTTPSessionInfo():finishtime(0), nosessiontime(0){}
};

struct HTTPServrManager:public HTTPCommunicationHandler,public enable_shared_from_this<HTTPServrManager>
{
	shared_ptr<IOWorker>				worker;
	shared_ptr<Socket>					tcpServer;
	uint32_t							httpport;
	std::string							useragent;
	

	Mutex														  mutex;
	std::map<std::string, std::map<std::string, ListenInfo> >     httplistencallbackmap;
	std::map<std::string, HTTPServer::WebsocketCallback>		  websocketlistencallbackmap;
	std::map<std::string, ListenInfo>							  httpdefaultlistencallback;
private:
	std::map< HTTPCommunication*, shared_ptr<HTTPSessionInfo> >   sessionlist;

	std::list< shared_ptr< HTTPSessionInfo> >					  freesessionlist;

	shared_ptr<Timer>											pooltimer;
public:
	HTTPServrManager() :httpport(0)
	{
		pooltimer = make_shared<Timer>("HTTPServrManager");
		pooltimer->start(Timer::Proc(&HTTPServrManager::onPoolTimerProc, this), 0, 1000);
	}
	~HTTPServrManager()
	{
		pooltimer = NULL;
		if (tcpServer) tcpServer->disconnect();
		tcpServer = NULL;
		worker = NULL;
		
		sessionlist.clear();
		freesessionlist.clear();
	}

	void onAcceptCallback(const weak_ptr<Socket>& oldsock, const shared_ptr<Socket>& newsock)
	{
		shared_ptr<HTTPSessionInfo> info = make_shared<HTTPSessionInfo>();
		info->commu = make_shared<HTTPCommunication>(true,newsock,shared_from_this(),useragent);

		{
			Guard locker(mutex);
			sessionlist[info->commu.get()]  = info;
		}
	}
private:
	bool onRecvHeaderOk(HTTPCommunication* commu)
	{
		shared_ptr<HTTPSessionInfo> info;
		{
			Guard locker(mutex);
			std::map<HTTPCommunication*, shared_ptr<HTTPSessionInfo> >::iterator iter = sessionlist.find(commu);
			if (iter == sessionlist.end()) return;

			info = iter->second;
		}

		Value connectionval = info->commu->recvHeader->header(CONNECTION);
		if (!connectionval.empty() && strcasecmp(connectionval.readString().c_str(), CONNECTION_Upgrade) == 0)
		{
			HTTPServer::WebsocketCallback callback = findWebsocketCallback(info->commu);

			if (!callback)
			{
				buildErrorResponse(info->commu, 405, "Method Not Allowed");

				return false;
			}

			if (!WebSocketServerSession::checkWebsocketHeader(info->commu->recvHeader))
			{
				buildErrorResponse(info->commu, 405, "Bad Request");

				return false;
			}

			shared_ptr<WebSocketServerSession> websocketsession = make_shared<WebSocketServerSession>(info->commu);
			info->websocketsession = websocketsession;
			
			callback(websocketsession);

			return false;
		}
		else
		{
			info->listeninfo = findHttpCallback(info->commu);
			if(!info->listeninfo.callback)
			{
				buildErrorResponse(info->commu, 405, "Method Not Allowed");

				return false;
			}
			info->httpsessionback = info->httpsession = make_shared<HTTPServerSession>(info->commu, info->listeninfo.type);

			return true;
		}
	}
	virtual void onRecvContentOk(HTTPCommunication* commu)
	{
		shared_ptr<HTTPSessionInfo> info;
		{
			Guard locker(mutex);
			std::map<HTTPCommunication*, shared_ptr<HTTPSessionInfo> >::iterator iter = sessionlist.find(commu);
			if (iter == sessionlist.end()) return;

			info = iter->second;
		}

		if (!info->httpsession) return;

		shared_ptr<HTTPServerSession> httpsession = info->httpsession;
		info->httpsession = NULL;

		info->listeninfo.callback(httpsession);
	}
	virtual void onDisconnect(HTTPCommunication* commu)
	{
		shared_ptr<HTTPSessionInfo> info;
		{
			Guard locker(mutex);
			std::map<HTTPCommunication*, shared_ptr<HTTPSessionInfo> >::iterator iter = sessionlist.find(commu);
			if (iter == sessionlist.end()) return;

			info = iter->second;
			freesessionlist.push_back(info);
			sessionlist.erase(iter);
		}

		shared_ptr<WebSocketServerSession> websocketsession = info->websocketsession.lock();
		if (websocketsession) websocketsession->disconnected();

		shared_ptr<HTTPServerSession> httpsession = info->httpsessionback.lock();
		if (httpsession) httpsession->disconnected();
	}

	HTTPServer::WebsocketCallback findWebsocketCallback(const shared_ptr<HTTPCommunication>& commu)
	{
		Guard locker(mutex);
		std::string requestPathname = String::tolower(commu->recvHeader->url.pathname);
		
		for (std::map<std::string, HTTPServer::WebsocketCallback>::iterator iter = websocketlistencallbackmap.begin(); iter != websocketlistencallbackmap.end(); iter++)
		{
			boost::regex oRegex(iter->first);
			if (!boost::regex_match(requestPathname, oRegex))
			{
				continue;
			}

			return iter->second;
		}

		return HTTPServer::WebsocketCallback();
	}

	ListenInfo findHttpCallback(const shared_ptr<HTTPCommunication>& commu)
	{
		ListenInfo liteninfo;
		{
			std::string requestPathname = String::tolower(commu->recvHeader->url.pathname);
			std::string method = String::tolower(commu->recvHeader->method);

			Guard locker(mutex);

			for (std::map<std::string, std::map<std::string, ListenInfo> >::iterator citer = httplistencallbackmap.begin(); citer != httplistencallbackmap.end() && liteninfo.callback == NULL; citer++)
			{
				boost::regex oRegex(citer->first);
				if(!boost::regex_match(requestPathname,oRegex))
				{
					continue;
				}
				for (std::map<std::string, ListenInfo>::iterator miter = citer->second.begin() ; miter != citer->second.end() && liteninfo.callback == NULL; miter++)
				{
					if (strcasecmp(method.c_str(), miter->first.c_str()) == 0)
					{
						liteninfo = miter->second;
						break;
					}
				}
			}
			for (std::map<std::string, ListenInfo>::iterator miter = httpdefaultlistencallback.begin() ; miter != httpdefaultlistencallback.end() && liteninfo.callback == NULL; miter++)
			{
				if (strcasecmp(method.c_str(), miter->first.c_str()) == 0)
				{
					liteninfo = miter->second;
					break;
				}
			}
		}
		
		return liteninfo;
	}

	void onPoolTimerProc(unsigned long)
	{
		std::map< HTTPCommunication*, shared_ptr<HTTPSessionInfo> > sessionlisttmp;
		std::list< shared_ptr< HTTPSessionInfo> >	freelisttmp;
		{
			Guard locker(mutex);
			sessionlisttmp = sessionlist;
			freelisttmp = freesessionlist;
			freesessionlist.clear();
		}
		uint64_t nowtime = Time::getCurrentMilliSecond();
		for (std::map< HTTPCommunication*, shared_ptr<HTTPSessionInfo> >::iterator iter = sessionlisttmp.begin(); iter != sessionlisttmp.end(); iter++)
		{
			iter->second->commu->onPoolTimerProc();
			if (iter->second->commu->isTimeout())
			{
				{
					Guard locker(mutex);
					sessionlist.erase(iter->first);
				}
				freelisttmp.push_back(iter->second);

				continue;
			}
			if (iter->second->finishtime == 0 && iter->second->commu->sessionIsFinish())
			{
				iter->second->finishtime = nowtime;
			}
			else if (iter->second->finishtime != 0 && nowtime - iter->second->finishtime >= COMMUFINISHTIMEOUT)
			{
				{
					Guard locker(mutex);
					sessionlist.erase(iter->first);
				}
				freelisttmp.push_back(iter->second);

				continue;
			}

			shared_ptr<WebSocketServerSession> websocketsession = iter->second->websocketsession.lock();
			shared_ptr<HTTPServerSession> httpsession = iter->second->httpsessionback.lock();
			if (iter->second->nosessiontime == 0 && websocketsession == NULL && httpsession == NULL)
			{
				iter->second->nosessiontime = nowtime;
			}
			else if (iter->second->nosessiontime != 0 && nowtime - iter->second->nosessiontime >= NOSESSIONTIMEOUT)
			{
				{
					Guard locker(mutex);
					sessionlist.erase(iter->first);
				}
				freelisttmp.push_back(iter->second);

				continue;
			}
		}
	}
};

struct HTTPServer::HTTPServrInternal
{
	shared_ptr<HTTPServrManager> manager;
};

HTTPServer::HTTPServer(const shared_ptr<IOWorker>& worker, const std::string& useragent)
{
	internal = new HTTPServrInternal();
	internal->manager = make_shared<HTTPServrManager>();
	internal->manager->worker = worker;
	internal->manager->useragent = useragent;

	if (internal->manager->worker == NULL) internal->manager->worker = IOWorker::defaultWorker();
}
HTTPServer::~HTTPServer()
{
	internal->manager = NULL;
	SAFE_DELETE(internal);
}

bool HTTPServer::listen(const std::string& path, const std::string& method, const HTTPCallback& callback, CacheType type)
{
	std::string flag1 = String::tolower(path);
	std::string flag2 = String::tolower(method);

	Guard locker(internal->manager->mutex);

	std::map<std::string, std::map<std::string, ListenInfo> >::iterator citer = internal->manager->httplistencallbackmap.find(String::tolower(flag1));
	if (citer == internal->manager->httplistencallbackmap.end())
	{
		internal->manager->httplistencallbackmap[flag1] = std::map<std::string, ListenInfo>();
		citer = internal->manager->httplistencallbackmap.find(String::tolower(flag1));
	}
	ListenInfo info;
	info.callback = callback;
	info.type = type;

	citer->second[flag2] = info;

	return true;
}
bool HTTPServer::listen(const std::string& path, const HTTPServer::WebsocketCallback& callback)
{
	std::string flag1 = String::tolower(path);
	
	Guard locker(internal->manager->mutex);

	internal->manager->websocketlistencallbackmap[flag1] = callback;

	return true;
}
bool HTTPServer::defaultListen(const std::string& method, const HTTPCallback& callback,CacheType type)
{
	std::string flag2 = String::tolower(method);
	
	Guard locker(internal->manager->mutex);

	ListenInfo info;
	info.callback = callback;
	info.type = type;

	internal->manager->httpdefaultlistencallback[flag2] = info;

	return true;
}

bool HTTPServer::run(uint32_t httpport)
{
	Guard locker(internal->manager->mutex);
	internal->manager->httpport = httpport;
	internal->manager->tcpServer = TCPServer::create(internal->manager->worker,httpport);
    if (internal->manager->tcpServer == shared_ptr<Socket>())
    {
        return false;
    }
	internal->manager->tcpServer->async_accept(Socket::AcceptedCallback(&HTTPServrManager::onAcceptCallback, internal->manager));

	return true;
}

uint32_t HTTPServer::listenPort() const
{
	return internal->manager->httpport;
}


}
}

