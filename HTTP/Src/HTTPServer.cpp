#include "HTTP/HTTPServer.h"
#include "HTTPSession.h"
#include "boost/regex.hpp" 
namespace Public {
namespace HTTP {

#define MAXCONNECTIONTIMEOUT	3*60*1000

struct ListenInfo
{
	HTTPCallback			callback;
	HTTPServer::CacheType	type;
};

struct HTTPServerCacheFile
{
public:
	HTTPServerCacheFile()
	{
		File::removeDirectory(cachepath);
		buildpath();
	}
	~HTTPServerCacheFile()
	{
		File::removeDirectory(cachepath);
	}
	std::string path()
	{
		if (!File::access(cachepath, File::accessExist)) File::makeDirectory(cachepath);

		return cachepath;
	}
private:
	void buildpath()
	{
#ifdef WIN32
		char systempath[513];
		GetSystemDirectory(systempath, 512);
		const char* syspathtmp = strcasestr(systempath, "Windows\\system32");
		if (syspathtmp == NULL)
		{
			syspathtmp = strcasestr(systempath, "Windows/system32");
		}
		if (syspathtmp != NULL)
		{
			*(char*)syspathtmp = 0;
			std::string systemdatapath = std::string(systempath) + "ProgramData\\ZVan";
			cachepath = systemdatapath + "\\IVS-III\\HttpCache\\" + File::getExcutableFileName();
		}
#else
		cachepath = std::string("/home/.ZVan/IVS-III/HttpCache/") + File::getExcutableFileName();
#endif
	}
private:
	std::string cachepath;
}cache;

struct HTTPServerObject
{
public:
	typedef Function1<ListenInfo, const shared_ptr<HTTPRequest>& > QueryCallback;
	typedef Function1<void, HTTPServerObject*>	DisconnectCallback;
private:
	shared_ptr<Socket>				sock;
	shared_ptr<HTTPSession_Service> session;
	QueryCallback					queryCallback;
	DisconnectCallback				disconnectCallback;
	std::string						useragent;
	char							buffer[MAXRECVBUFFERLEN];
	int								bufferlen;
public:
	HTTPServerObject(const shared_ptr<Socket>& _sock, const std::string& _useragent,const QueryCallback& _querycallback,const DisconnectCallback& _discallback)
	{
		bufferlen = 0;
		queryCallback = _querycallback;
		disconnectCallback = _discallback;

		sock = _sock;
		useragent = _useragent;
		session = make_shared<HTTPSession_Service>(sock, _useragent,
			HTTPParser<HTTPRequest>::HeaderParseSuccessCallback(&HTTPServerObject::headerParseCallback,this));

		sock->setDisconnectCallback(Socket::DisconnectedCallback(&HTTPServerObject::socketDisconnectCallback, this));
		sock->async_recv(buffer, MAXRECVBUFFERLEN,Socket::ReceivedCallback(&HTTPServerObject::recvCallback, this));
	}
	~HTTPServerObject()
	{
		sock = NULL;
		session = NULL;
	}
	void onPoolTimerProc()
	{
		shared_ptr<HTTPSession> tmp = session;
		if(tmp != NULL) tmp->onPoolTimerProc();
	}
	bool isTimeout()
	{
		shared_ptr<HTTPSession> tmp = session;
		if (tmp == NULL) return true;

		uint64_t nowtime = Time::getCurrentMilliSecond();
		uint64_t sessiontime = tmp->prevAliveTime();

		bool timeoutflag = nowtime > sessiontime && nowtime - sessiontime >= MAXCONNECTIONTIMEOUT;

		if (timeoutflag)
		{
			shared_ptr<HTTPRequest> request = tmp->request;
			if (request != NULL) request->discallback()(request.get(), "timeout");
		}

		return timeoutflag;
	}
private:
	void headerParseCallback(const shared_ptr<HTTPRequest>& req)
	{
		ListenInfo callback = queryCallback(req);

		if (callback.callback == NULL || callback.type == HTTPServer::CacheType_File)
		{
			char buffer[256] = { 0 };
			snprintf_x(buffer, 255, "%llu_%x.cache", Time::getCurrentMilliSecond(), buffer);

			req->content()->setReadToFile(cache.path() + PATH_SEPARATOR + buffer, true);
		}
	}
	void recvCallback(const weak_ptr<Socket>& ,const char* ,int len)
	{
		shared_ptr<HTTPSession_Service> tmp = session;
		if (tmp == NULL) return;

		if (len <= 0)
		{
			shared_ptr<HTTPRequest> request = tmp->request;
			if (request != NULL) request->discallback()(request.get(), "Socket Recv Error");

			shared_ptr<HTTPResponse> response = tmp->response;
			if (response != NULL)
			{
				response->statusCode() = 500;
				response->errorMessage() = "Socket Recv Error";
			}

			disconnectCallback(this);

			return;
		}

		if (tmp->isFinish())
		{
			tmp = session = make_shared<HTTPSession_Service>(sock, useragent,
				HTTPParser<HTTPRequest>::HeaderParseSuccessCallback(&HTTPServerObject::headerParseCallback, this));
		}

		bufferlen += len;

		const char* usedendbuf = tmp->inputData(buffer, bufferlen);
		bufferlen = buffer + bufferlen - usedendbuf;
		if (bufferlen > 0 && usedendbuf != buffer)
		{
			memmove(buffer, usedendbuf, bufferlen);
		}

		if (tmp->requestFinish() && tmp->response == NULL)
		{
			shared_ptr<HTTPResponse> response = make_shared<HTTPResponse>
				(HTTPContent::CheckConnectionIsOk(&HTTPSession_Service::checkSocketIsOk,tmp));

			tmp->setResponse(response);
			ListenInfo callback = queryCallback(tmp->request);
			if (callback.callback == NULL)
			{
				response->statusCode() = 405;
				response->errorMessage() = "Method Not Allowed";

				disconnectCallback(this);

				return;
			}
			else
			{
				callback.callback(tmp->request, response);
				session->onPoolTimerProc();
			}
		}

		int recvlen = MAXRECVBUFFERLEN - bufferlen;
		if (recvlen <= 0)
		{
			shared_ptr<HTTPRequest> request = tmp->request;
			if (request != NULL) request->discallback()(request.get(), "Socket Recv Buffer Full");

			shared_ptr<HTTPResponse> response = tmp->response;
			if (response != NULL)
			{
				response->statusCode() = 500;
				response->errorMessage() = "Socket Recv Buffer Full";
			}			

			disconnectCallback(this);

			return;
		}

		shared_ptr<Socket> socktmp = sock;
		if(socktmp != NULL) socktmp->async_recv(buffer + bufferlen,recvlen,Socket::ReceivedCallback(&HTTPServerObject::recvCallback, this));
	}
	void socketDisconnectCallback(const weak_ptr<Socket>& sock,const std::string&)
	{
		shared_ptr<HTTPSession> tmp = session;
		if (tmp != NULL)
		{
			shared_ptr<HTTPRequest> request = tmp->request;
			if (request != NULL) request->discallback()(request.get(), "socket disconnect");
		}
		if (tmp != NULL)
		{
			shared_ptr<HTTPResponse> response = tmp->response;
			if (response != NULL) response->discallback()(response.get(), "socket disconnect");
		}
		disconnectCallback(this);
	}
};
struct HTTPServer::HTTPServrInternal:public Thread
{
	shared_ptr<IOWorker>	worker;
	shared_ptr<Socket>		tcpServer;
	uint32_t				httpport;
	std::string				useragent;
	
	Mutex														  mutex;
	std::map<std::string, std::map<std::string, ListenInfo> >     listencallbackmap;
	std::map<std::string, ListenInfo>							  defaultcallback;

	std::map<HTTPServerObject*,shared_ptr<HTTPServerObject> >	  objectlist;
	std::set<HTTPServerObject*>									  freelist;

	HTTPServrInternal() :Thread("HTTPServrInternal"),httpport(0)
	{
		createThread();
	}
	~HTTPServrInternal()
	{
		destroyThread();
	}

	void acceptCallback(const weak_ptr<Socket>& oldsock, const shared_ptr<Socket>& newsock)
	{
		shared_ptr<HTTPServerObject> obj = make_shared<HTTPServerObject>(newsock,useragent,
			HTTPServerObject::QueryCallback(&HTTPServrInternal::getHttpRequestCallback,this),
			HTTPServerObject::DisconnectCallback(&HTTPServrInternal::disconnectCallback,this)
			);

		{
			Guard locker(mutex);
			objectlist[obj.get()]  = obj;
		}
	}

	void disconnectCallback(HTTPServerObject* object)
	{
		Guard locker(mutex);
		freelist.insert(object);
	}

	void threadProc()
	{
		while (looping())
		{
			Thread::sleep(100);
		
			std::list<shared_ptr<HTTPServerObject> > closelist;
			std::list<shared_ptr<HTTPServerObject> > timeoutlist;
			std::map<HTTPServerObject*, shared_ptr<HTTPServerObject> > poollist;
			{
				Guard locker(mutex);
		
				for (std::set<HTTPServerObject*>::iterator iter = freelist.begin(); iter != freelist.end(); iter++)
				{
					std::map<HTTPServerObject*, shared_ptr<HTTPServerObject> >::iterator oiter = objectlist.find(*iter);
					if (oiter != objectlist.end())
					{
						closelist.push_back(oiter->second);
						objectlist.erase(oiter);
					}
				}
				freelist.clear();

				poollist = objectlist;
			}

			for (std::map<HTTPServerObject*, shared_ptr<HTTPServerObject> >::iterator iter = poollist.begin(); iter != poollist.end(); iter++)
			{
				iter->second->onPoolTimerProc();

				if (iter->second->isTimeout())
				{
					timeoutlist.push_back(iter->second);
				}
			}

			{
				Guard locker(mutex);
				for (std::list<shared_ptr<HTTPServerObject> >::iterator iter = timeoutlist.begin(); iter != timeoutlist.end(); iter++)
				{
					objectlist.erase((*iter).get());
				}
			}
		}
	}
	
	ListenInfo getHttpRequestCallback(const shared_ptr<HTTPRequest>& request)
	{
		ListenInfo callback;
		{
			std::string requestPathname = String::tolower(request->url().pathname);
			std::string method = String::tolower(request->method());

			Guard locker(mutex);
			for (std::map<std::string, std::map<std::string, ListenInfo> >::iterator citer = listencallbackmap.begin(); citer != listencallbackmap.end() && callback.callback == NULL; citer++)
			{
				boost::regex oRegex(citer->first);
				if(!boost::regex_match(requestPathname,oRegex))
				{
					continue;
				}
				for (std::map<std::string, ListenInfo>::iterator miter = citer->second.begin() ; miter != citer->second.end() && callback.callback == NULL; miter++)
				{
					if (strcasecmp(method.c_str(), miter->first.c_str()) == 0)
					{
						callback = miter->second;
						break;
					}
				}
			}
			for (std::map<std::string, ListenInfo>::iterator miter = defaultcallback.begin() ; miter != defaultcallback.end() && callback.callback == NULL; miter++)
			{
				if (strcasecmp(method.c_str(), miter->first.c_str()) == 0)
				{
					callback = miter->second;
					break;
				}
			}
		}
		
		return callback;
	}
};


HTTPServer::HTTPServer(const shared_ptr<IOWorker>& worker, const std::string& useragent)
{
	internal = new HTTPServrInternal();
	internal->worker = worker;
	internal->useragent = useragent;
}
HTTPServer::~HTTPServer()
{
	SAFE_DELETE(internal);
}

bool HTTPServer::listen(const std::string& path, const std::string& method, const HTTPCallback& callback, CacheType type)
{
	std::string flag1 = String::tolower(path);
	std::string flag2 = String::tolower(method);

	Guard locker(internal->mutex);

	std::map<std::string, std::map<std::string, ListenInfo> >::iterator citer = internal->listencallbackmap.find(String::tolower(flag1));
	if (citer == internal->listencallbackmap.end())
	{
		internal->listencallbackmap[flag1] = std::map<std::string, ListenInfo>();
		citer = internal->listencallbackmap.find(String::tolower(flag1));
	}
	ListenInfo info;
	info.callback = callback;
	info.type = type;

	citer->second[flag2] = info;

	return true;
}

bool HTTPServer::defaultListen(const std::string& method, const HTTPCallback& callback,CacheType type)
{
	std::string flag2 = String::tolower(method);
	
	Guard locker(internal->mutex);

	ListenInfo info;
	info.callback = callback;
	info.type = type;

	internal->defaultcallback[flag2] = info;

	return true;
}

bool HTTPServer::run(uint32_t httpport)
{
	Guard locker(internal->mutex);
	internal->httpport = httpport;
	internal->tcpServer = TCPServer::create(internal->worker,httpport);
	internal->tcpServer->async_accept(Socket::AcceptedCallback(&HTTPServrInternal::acceptCallback, internal));

	return true;
}

uint32_t HTTPServer::listenPort() const
{
	return internal->httpport;
}


}
}

