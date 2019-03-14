#include "HTTP/HTTPClient.h"
#include "HTTPSession.h"

#ifdef WIN32
#define socklen_t int
#endif


namespace Public {
namespace HTTP {

struct HTTPClientObject
{
	shared_ptr<IOWorker>	worker;
	shared_ptr<Socket>		sock;
	NetAddr					addr;
	shared_ptr<HTTPSession> session;
	std::string				useragent;
	bool					disconnectflag;
	shared_ptr<HTTPRequest>	request;
	std::string				saveToFile;

	char					buffer[MAXRECVBUFFERLEN];
	int						bufferlen;

	HTTPClientObject(const shared_ptr<IOWorker>& _worker, const std::string& _useragent):disconnectflag(false)
	{
		worker = _worker;
		bufferlen = 0;
		useragent = _useragent;
	}
	~HTTPClientObject()
	{
		sock = NULL;
	}
	shared_ptr<HTTPResponse> getResponse()
	{
		shared_ptr<HTTPSession> sessiontmp = session;
		shared_ptr<HTTPResponse> response;
		if (sessiontmp != NULL)
		{
			response = sessiontmp->response;
		}
		else
		{
			response = make_shared<HTTPResponse>();
		}
		if (sessiontmp != NULL && !sessiontmp->isFinish() && disconnectflag)
		{
			response->statusCode() = 500;
			response->errorMessage() = "Socket Disconnect";
		}
		else if (sessiontmp == NULL || !sessiontmp->isFinish())
		{
			response->statusCode() = 408;
			response->errorMessage() = "Request Timeout";
		}

		return response;
	}

	virtual void start(const shared_ptr<HTTPRequest>& req, const std::string& saveasfile)
	{
		request = req;
		request->headers()[CONNECTION] = CONNECTION_KeepAlive;
		request->headers()["Host"] = req->url().getHost();
		saveToFile = saveasfile;
		NetAddr toaddr = NetAddr(request->url().getHostname(), request->url().getPort(80));

		if (toaddr.getIP() != addr.getIP() || toaddr.getPort() != addr.getPort() || sock == NULL)
		{
			session = NULL;
			sock = TCPClient::create(worker);

			sock->async_connect(NetAddr(request->url().getHostname(), request->url().getPort(80)), Socket::ConnectedCallback(&HTTPClientObject::connectCallback, this));
		}
		else if (sock != NULL && sock->getStatus() == NetStatus_connected)
		{
			connectCallback(sock);
		}
		addr = toaddr;
	}

	virtual void connectCallback(const weak_ptr<Socket>&)
	{
		shared_ptr<Socket> socktmp = sock;
		if(socktmp != NULL)
		{
			struct sockaddr_in iface_out;
			int len = sizeof(iface_out);
			if (getsockname(socktmp->getHandle(), (struct sockaddr *) &iface_out, (socklen_t*)&len) >= 0 && iface_out.sin_addr.s_addr != 0)
			{
				request->myAddr() = NetAddr(iface_out);
			}


			shared_ptr<HTTPSession> sessiontmp  = make_shared<HTTPSession_Client>(request, socktmp, useragent);

			if (saveToFile != "")
				sessiontmp->response->content()->setReadToFile(saveToFile);


			socktmp->setDisconnectCallback(Socket::DisconnectedCallback(&HTTPClientObject::socketDisconnect, this));
			socktmp->async_recv(buffer, MAXRECVBUFFERLEN, Socket::ReceivedCallback(&HTTPClientObject::socketRecv, this));

			session = sessiontmp;

		}
	}
	virtual void socketDisconnect(const weak_ptr<Socket>&, const std::string&)
	{
		disconnectflag = true;
	}
	void socketRecv(const weak_ptr<Socket>& s, const char* , int len)
	{
		shared_ptr<HTTPSession> sessiontmp = session;
		if (len <= 0 || sessiontmp == NULL) return socketDisconnect(s, "socket recv error");

		bufferlen += len;

		const char* usedbuf = sessiontmp->inputData(buffer, bufferlen);

		bufferlen = buffer + bufferlen - usedbuf;
		if (bufferlen > 0 && usedbuf != buffer)
		{
			memmove(buffer, usedbuf, bufferlen);
		}

		int recvlen = MAXRECVBUFFERLEN - bufferlen;
		if(recvlen <= 0)  return socketDisconnect(s, "socket recv buffer full");

		shared_ptr<Socket> socktmp = sock;
		if(socktmp != NULL)
			socktmp->async_recv(buffer + bufferlen,recvlen,Socket::ReceivedCallback(&HTTPClientObject::socketRecv, this));
	}
};

struct HTTPClient::HTTPClientInternal:public HTTPClientObject
{
	uint64_t	startTime;
	bool		finish;
	
	HTTPClientInternal(const shared_ptr<IOWorker>& _worker, const std::string& _useragent):HTTPClientObject(_worker,_useragent)
	{
		startTime = Time::getCurrentMilliSecond();
		finish = false;
	}
	~HTTPClientInternal() 
	{
	}
	virtual void start(const shared_ptr<HTTPRequest>& req, const std::string& saveasfile)
	{
		startTime = Time::getCurrentMilliSecond();
		finish = false;

		HTTPClientObject::start(req, saveasfile);
	}
	void pend(uint32_t timeout)
	{
		while (!finish)
		{
			shared_ptr<HTTPSession> sessiontmp = session;
			uint64_t nowtime = Time::getCurrentMilliSecond();
			if (nowtime - startTime > timeout) break;

			if (sessiontmp != NULL)
			{
				sessiontmp->onPoolTimerProc();
				if (sessiontmp->isFinish())
				{
					finish = true; 
					break;
				}
			}

			Thread::sleep(10);
		}
	}
private:
	virtual void socketDisconnect(const weak_ptr<Socket>& socktmp, const std::string& err)
	{
		HTTPClientObject::socketDisconnect(socktmp, err);

		finish = true;
	}
};

HTTPClient::HTTPClient(const shared_ptr<IOWorker>& worker, const std::string& useragent)
{
	internal = new HTTPClientInternal(worker, useragent);
}
HTTPClient::~HTTPClient()
{
	SAFE_DELETE(internal);
}

const shared_ptr<HTTPResponse> HTTPClient::request(const shared_ptr<HTTPRequest>& req, const std::string& saveasfile)
{
	internal->start(req,saveasfile);

	internal->pend(req->timeout());
	
	return internal->getResponse();
}
const shared_ptr<HTTPResponse> HTTPClient::request(const std::string& url, const std::string& method, const shared_ptr<HTTPContent>& buf, const std::map<std::string, Value>& headers, int timeout)
{
	shared_ptr<HTTPRequest> req = make_shared<HTTPRequest>();
	req->url() = url;
	if(buf != NULL)
		req->content() = buf;
	req->method() = method;
	req->headers() = headers;
	req->timeout() = timeout;

	return request(req);
}

struct HTTPAsyncObject :public HTTPClientObject
{
	HTTPCallback					callback;
	bool							connectSuccess;
	uint64_t						prevusedtime;

	HTTPAsyncObject(const HTTPCallback& _callback,const shared_ptr<IOWorker>& _worker, const std::string& _useragent)
		:HTTPClientObject(_worker, _useragent),callback(_callback), connectSuccess(false), prevusedtime(Time::getCurrentMilliSecond()){}
	~HTTPAsyncObject() {}

	virtual void connectCallback(const weak_ptr<Socket>& socktmp)
	{
		connectSuccess = true;
		HTTPClientObject::connectCallback(socktmp);

		prevusedtime = Time::getCurrentMilliSecond();
	}

	uint64_t prevAliveTime()
	{
		if (session == NULL) return prevusedtime;

		return max(prevusedtime, session->prevAliveTime());
	}
};

struct HTTPAsyncClient::HTTPAsyncClientInternal:public Thread
{
	shared_ptr<IOWorker>	worker;
	std::string				useragent;

	Mutex					mutex;
	std::map<HTTPRequest*, shared_ptr<HTTPAsyncObject> > objectlist;

	HTTPAsyncClientInternal() :Thread("HTTPAsyncClientInternal") 
	{
		createThread();
	}
	~HTTPAsyncClientInternal() 
	{
		destroyThread();
	}

	void threadProc()
	{
		while (looping())
		{
			Thread::sleep(100);

			{
				std::map<HTTPRequest*, shared_ptr<HTTPAsyncObject> > poolList;
				{
					Guard locker(mutex);
					poolList = objectlist;
				}
				for (std::map<HTTPRequest*, shared_ptr<HTTPAsyncObject> >::iterator iter = poolList.begin(); iter != poolList.end(); iter++)
				{
					if (iter->second->connectSuccess && iter->second->session != NULL)
						iter->second->session->onPoolTimerProc();
				}
			}
			std::list<shared_ptr<HTTPAsyncObject> > finishlist;
			std::list<shared_ptr<HTTPAsyncObject> > timeoutlist;
			std::list<shared_ptr<HTTPAsyncObject> > disconnectlist;
			{
				Guard locker(mutex);

				uint64_t nowtime = Time::getCurrentMilliSecond();
				for (std::map<HTTPRequest*, shared_ptr<HTTPAsyncObject> >::iterator iter = objectlist.begin(); iter != objectlist.end();)
				{
					uint64_t prevtime = iter->second->prevAliveTime();
					if (nowtime > prevtime && nowtime - prevtime > iter->second->request->timeout())
					{
						timeoutlist.push_back(iter->second);
						objectlist.erase(iter++);
					}
					else if (iter->second->session != NULL && iter->second->session->isFinish())
					{
						finishlist.push_back(iter->second);
						objectlist.erase(iter++);
					}
					else if (iter->second->disconnectflag)
					{
						disconnectlist.push_back(iter->second);
						objectlist.erase(iter++);
					}
					else
					{
						iter++;
					}
				}
			}
			for (std::list<shared_ptr<HTTPAsyncObject> >::iterator iter = timeoutlist.begin(); iter != timeoutlist.end(); iter++)
			{
				shared_ptr<HTTPResponse> response = (*iter)->getResponse();
				(*iter)->callback((*iter)->request, response);
			}
			for (std::list<shared_ptr<HTTPAsyncObject> >::iterator iter = finishlist.begin(); iter != finishlist.end(); iter++)
			{
				shared_ptr<HTTPResponse> response = (*iter)->getResponse();
				(*iter)->callback((*iter)->request, response);
			}
			for (std::list<shared_ptr<HTTPAsyncObject> >::iterator iter = disconnectlist.begin(); iter != disconnectlist.end(); iter++)
			{
				shared_ptr<HTTPRequest> request = (*iter)->request;
				request->discallback()(request.get(), "socket disconnect");
			}
		}
	}
};


HTTPAsyncClient::HTTPAsyncClient(const shared_ptr<IOWorker>& worker, const std::string& useragent)
{
	internal = new HTTPAsyncClientInternal();
	internal->worker = worker;
	internal->useragent = useragent;
}
HTTPAsyncClient::~HTTPAsyncClient()
{
	SAFE_DELETE(internal);
}
bool HTTPAsyncClient::request(const shared_ptr<HTTPRequest>& req, const HTTPCallback& callback, const std::string& saveasfile)
{
	shared_ptr<HTTPAsyncObject> object = make_shared<HTTPAsyncObject>(callback,internal->worker, internal->useragent);
	object->start(req,saveasfile);

	Guard locker(internal->mutex);
	internal->objectlist[req.get()] = object;

	return true;
}

}
}
