#include "HTTP/HTTPServer.h"
#include "HTTPParser.h"
#include "HTTPBuilder.h"
#include "boost/regex.hpp" 
namespace Public {
namespace HTTP {


struct HTTPResponseObject :public HTTPParser<HTTPRequest>,public HTTPBuilder<HTTPResponse>,public HTTPResponse
{
	typedef Function1<void, HTTPResponseObject*> DisconnectCallback;
	typedef Function2<HTTPCallback, HTTPResponseObject*,const shared_ptr<HTTPRequest>& > SuccessCallback;
	typedef Function4<void, const shared_ptr<Socket>&, const char*, int,bool> FreeSocketObject;
private:
	shared_ptr<Socket>		sock;
	bool					sockError;

	FreeSocketObject		freecallback;
	DisconnectCallback		discallback;
	SuccessCallback			succcallback;
	HTTPCallback			httpcallback;
	Value					connection;
public:
	weak_ptr<HTTPResponse>	resp;
public:
	HTTPResponseObject(const shared_ptr<Socket>& _sock,const SuccessCallback& scall,const DisconnectCallback& dcall,const FreeSocketObject& fcall, HTTPBuffer::HTTPBufferType type, const std::string& useragent)
		:HTTPBuilder(_sock,useragent),sock(_sock),sockError(false),freecallback(fcall),discallback(dcall),succcallback(scall)
	{
		connection = CONNECTION;
	}
	~HTTPResponseObject()
	{
		readBufferCallback(NULL, NULL, 0);
		if (!sockError)
		{
			freecallback(sock, readfbuffer(), bufferdatalen(),strcasecmp(connection.readString().c_str(),CONNECTION_KeepAlive) == 0);
		}
	}

	bool input(const char* buffer = NULL, int len = 0)
	{
		if (len > 0)
		{
			memcpy(readfbuffer(), buffer, len);

			recvCallback(shared_ptr<Socket>(), NULL, len);
		}

		return true;
	}
	bool start(bool recvflag = true)
	{
		content()->setReadCallback(HTTPBuffer::StreamCallback(&HTTPBuilder::readBufferCallback, (HTTPBuilder*)this));
		sock->setDisconnectCallback(Socket::DisconnectedCallback(&HTTPResponseObject::disconnectCallback, this));
		if(recvflag)
			sock->async_recv(readfbuffer(), readbufferlen(), Socket::ReceivedCallback(&HTTPResponseObject::recvCallback, this));

		return true;
	}
private:
	HTTPResponse* getObject()
	{
		return this;
	}
	void disconnectCallback(const shared_ptr<Socket>& sock, const std::string& err)
	{
		sockError = true;
		discallback(this);
		HTTPParser::object->discallback()(HTTPParser::object.get(),err);
	}
	void recvCallback(const shared_ptr<Socket>& sock, const char* buffer, int len)
	{
		shared_ptr<HTTPResponse> response = resp.lock();
		shared_ptr<Socket> tmp = sock;

		if (response == NULL) return;

		intpulen(len);

		if (!headerIsOk())
		{
			if (parseHeader())
			{
				if (HTTPParser::object->content()->readtype() != HTTPBuffer::HTTPBufferType_String)
				{
					connection = HTTPParser::object->header(CONNECTION);
					httpcallback = succcallback(this, HTTPParser::object);
					this->headers()[CONNECTION] = connection.empty() ? CONNECTION_Close : connection;

					if (httpcallback == NULL)
					{
						this->statusCode() = 405;
						this->errorMessage() = "Method Not Allowed";
						readBufferCallback(NULL, NULL, 0);
					}
					else
					{
						httpcallback(HTTPParser::object, response);
					}
				}
			}
			else if(tmp != NULL)
			{
				tmp->async_recv(readfbuffer(), readbufferlen(), Socket::ReceivedCallback(&HTTPResponseObject::recvCallback, this));
				return;
			}
		}

		if (parseContent())
		{
			connection = HTTPParser::object->header(CONNECTION);
			this->headers()[CONNECTION] = connection.empty() ? CONNECTION_Close : connection;

			if (HTTPParser::object->content()->writetype() == HTTPBuffer::HTTPBufferType_String)
			{
				httpcallback = succcallback(this, HTTPParser::object);
				if (httpcallback == NULL)
				{
					this->statusCode() = 405;
					this->errorMessage() = "Method Not Allowed";
					readBufferCallback(NULL, NULL, 0);
				}
				else
				{
					httpcallback(HTTPParser::object, response);
				}
			}
		}
		else if(tmp != NULL)
		{
			tmp->async_recv(readfbuffer(), readbufferlen(), Socket::ReceivedCallback(&HTTPResponseObject::recvCallback, this));
		}
	}
	const char* parseFirstLine(const char* buffer, int len)
	{
		int pos = String::indexOf(buffer, len, HTTPHREADERLINEEND);
		if (pos == -1) return NULL;

		std::vector<std::string> tmp = String::split(buffer, pos, " ");
		if (tmp.size() != 3) return NULL;

		HTTPParser::object->method() = tmp[0];
		HTTPParser::object->url() = tmp[1];

		return buffer + pos + strlen(HTTPHREADERLINEEND);
	}
	virtual std::string buildFirstLine()
	{
		return std::string(HTTPVERSION) + " " + Value(statusCode()).readString() + " " + errorMessage() + HTTPHREADERLINEEND;
	}
};

struct HTTPServer::HTTPServrInternal
{
	shared_ptr<IOWorker>	worker;
	shared_ptr<Socket>		tcpServer;
	shared_ptr<Timer>		poolTimer;
	uint32_t			   httpport;
	std::string				useragent;
	

	Mutex														  mutex;
	std::map<std::string, std::map<std::string, HTTPCallback> >   listencallbackmap;
	std::map<std::string, HTTPCallback>							  defaultcallback;

	std::map<HTTPResponseObject*, shared_ptr<HTTPResponseObject> > objlist;
	std::list<shared_ptr<HTTPResponseObject> >					 freelist;
	HTTPBuffer::HTTPBufferType									 type;

	HTTPServrInternal() :httpport(0)
	{
		poolTimer = make_shared<Timer>("HTTPServrInternal");
		poolTimer->start(Timer::Proc(&HTTPServrInternal::poolTimerProc, this), 0, 1000);
	}
	~HTTPServrInternal()
	{
		poolTimer = NULL;
	}

	void acceptCallback(const shared_ptr<Socket>& oldsock, const shared_ptr<Socket>& newsock)
	{
		shared_ptr<Socket> sock = newsock;
		sock->setSocketTimeout(1000, 1000);

		shared_ptr<HTTPResponseObject> obj = make_shared<HTTPResponseObject>(newsock,
			HTTPResponseObject::SuccessCallback(&HTTPServrInternal::objectSuccessCallback, this),
			HTTPResponseObject::DisconnectCallback(&HTTPServrInternal::objectDisconnectCllback, this),
			HTTPResponseObject::FreeSocketObject(&HTTPServrInternal::objectFreeCallback, this),
			type,useragent);
		obj->resp = obj;

		{
			Guard locker(mutex);
			objlist[obj.get()] = obj;
		}
		obj->start();
	}

	void poolTimerProc(unsigned long)
	{
		std::list<shared_ptr<HTTPResponseObject> > freeTmp;
		{
			Guard locker(mutex);
			freeTmp = freelist;
			freelist.clear();
		}
	}
	void objectDisconnectCllback(HTTPResponseObject* obj)
	{
		Guard locker(mutex);
		std::map<HTTPResponseObject*, shared_ptr<HTTPResponseObject> >::iterator iter = objlist.find(obj);
		if (iter != objlist.end())
		{
			freelist.push_back(iter->second);
			objlist.erase(iter);
		}
	}
	HTTPCallback objectSuccessCallback(HTTPResponseObject* obj, const shared_ptr<HTTPRequest>& request)
	{
		shared_ptr<HTTPResponseObject> reqobj;
		HTTPCallback callback;
		{
			Guard locker(mutex);
			{
				std::map<HTTPResponseObject*, shared_ptr<HTTPResponseObject> >::iterator iter = objlist.find(obj);
				if (iter == objlist.end())
				{
					return HTTPCallback();
				}
				reqobj = iter->second;
				freelist.push_back(reqobj);
				objlist.erase(iter);
			}

			for (std::map<std::string, std::map<std::string, HTTPCallback> >::iterator citer = listencallbackmap.begin(); citer != listencallbackmap.end() && callback == NULL; citer++)
			{
				boost::regex oRegex(citer->first);
				if(!boost::regex_match(request->url().pathname,oRegex))
				{
					continue;
				}
				for (std::map<std::string, HTTPCallback>::iterator miter = citer->second.begin() ; miter != citer->second.end() && callback == NULL; miter++)
				{
					if (strcasecmp(request->method().c_str(), miter->first.c_str()) == 0)
					{
						callback = miter->second;
						break;
					}
				}
			}
			for (std::map<std::string, HTTPCallback>::iterator miter = defaultcallback.begin() ; miter != defaultcallback.end() && callback == NULL; miter++)
			{
				if (strcasecmp(request->method().c_str(), miter->first.c_str()) == 0)
				{
					callback = miter->second;
					break;
				}
			}
		}
		
		return callback;
	}
	void objectFreeCallback(const shared_ptr<Socket>& sock, const char* buf, int len,bool keep_alive)
	{
		if (!keep_alive) return;

		shared_ptr<HTTPResponseObject> obj = make_shared<HTTPResponseObject>(sock,
			HTTPResponseObject::SuccessCallback(&HTTPServrInternal::objectSuccessCallback, this),
			HTTPResponseObject::DisconnectCallback(&HTTPServrInternal::objectDisconnectCllback, this),
			HTTPResponseObject::FreeSocketObject(&HTTPServrInternal::objectFreeCallback, this),
			type, useragent);
		obj->resp = obj;

		{
			Guard locker(mutex);
			objlist[obj.get()] = obj;
		}

		obj->input(buf, len);
		obj->start(!obj->contentIsOk());

		if(obj->contentIsOk())
		{
			Guard locker(mutex);
			objlist.erase(obj.get());
		}
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

bool HTTPServer::listen(const std::string& path, const std::string& method, const HTTPCallback& callback)
{
	std::string flag1 = String::tolower(path);
	std::string flag2 = String::tolower(method);

	Guard locker(internal->mutex);

	std::map<std::string, std::map<std::string, HTTPCallback> >::iterator citer = internal->listencallbackmap.find(String::tolower(flag1));
	if (citer == internal->listencallbackmap.end())
	{
		internal->listencallbackmap[flag1] = std::map<std::string, HTTPCallback>();
		citer = internal->listencallbackmap.find(String::tolower(flag1));
	}
	citer->second[flag2] = callback;

	return true;
}

bool HTTPServer::defaultListen(const std::string& method, const HTTPCallback& callback)
{
	std::string flag2 = String::tolower(method);
	
	Guard locker(internal->mutex);

	internal->defaultcallback[flag2] = callback;

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

