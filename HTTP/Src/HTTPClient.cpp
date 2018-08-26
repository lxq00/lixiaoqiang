#include "HTTP/HTTPClient.h"
#include "HTTPParser.h"
#include "HTTPBuilder.h"

#ifdef WIN32
#define socklen_t int
#endif


namespace Public {
namespace HTTP {

struct HTTPRequestObject :public HTTPParser<HTTPResponse>, public HTTPBuilder<HTTPRequest>
{
public:
	typedef Function1<void, HTTPRequest*> CloseCallbak;
private:
	shared_ptr<HTTPRequest>	req;
	HTTPCallback			httpcallback;
	shared_ptr<Socket>		sock;
	HTTPBuffer::HTTPBufferType readtype;
	CloseCallbak			 closecallback;
	uint64_t				starttime;
	std::string				outfilename;
public:
	HTTPRequestObject(const shared_ptr<HTTPRequest>& _req,const shared_ptr<Socket>& _sock,const HTTPCallback& _callback,const CloseCallbak& close, const std::string& useragent, HTTPBuffer::HTTPBufferType type, const std::string& outfile)
		:HTTPBuilder(_sock,useragent),req(_req),sock(_sock), httpcallback(_callback), closecallback(close),outfilename(outfile)
	{
		sock->setSocketTimeout(1000, 1000);
		req->headers()[CONNECTION] = CONNECTION_Close;
		readtype = type;
		starttime = Time::getCurrentMilliSecond();
	}
	~HTTPRequestObject(){}
	shared_ptr<HTTPRequest> reqeust() { return req; }
	HTTPRequest* getObject()
	{
		return req.get();
	}
	uint64_t prevtime() { return starttime; }
	bool startConnect()
	{
		sock->async_connect(NetAddr(req->url().getHostname(), req->url().getPort(80)), Socket::ConnectedCallback(&HTTPRequestObject::connectCallback, this));

		return true;
	}
	bool start()
	{
		bool ret = sock->connect(NetAddr(req->url().getHostname(), req->url().getPort(80)));
		if (!ret)
		{
			object->statusCode() = 503;
			object->errorMessage() = "Connect Server Error";

			return false;
		}

		startsenddata();

		return true;
	}
private:
	void startsenddata()
	{
	//	void getMyAddr()
		{
			struct sockaddr_in iface_out;
			int len = sizeof(iface_out);
			if (getsockname(sock->getHandle(), (struct sockaddr *) &iface_out, (socklen_t*)&len) >= 0 && iface_out.sin_addr.s_addr != 0)
			{
				req->myAddr() = NetAddr(iface_out);
			}
		}
		req->headers()["Host"] = req->url().getHost();
		sock->setDisconnectCallback(Socket::DisconnectedCallback(&HTTPRequestObject::disconnectCallback, this));
		sock->async_recv(readfbuffer(), readbufferlen(), Socket::ReceivedCallback(&HTTPRequestObject::recvCallback, this));

		if (req->content()->writetype() != HTTPBuffer::HTTPBufferType_String)
		{
			req->content()->setReadCallback(HTTPBuffer::StreamCallback(&HTTPBuilder::readBufferCallback, (HTTPBuilder*)this));
		}
		else
		{
			HTTPBuilder::startasyncsend(shared_ptr<Socket>(), NULL, 0);
		}
	}
private:
	void connectCallback(const shared_ptr<Socket>& sock)
	{
		startsenddata();
	}
	void disconnectCallback(const shared_ptr<Socket>& sock, const std::string& err)
	{
		req->discallback()(req.get(), err);
		closecallback(req.get());
	}
	void recvCallback(const shared_ptr<Socket>& sock, const char* buffer, int len)
	{
		starttime = Time::getCurrentMilliSecond();

		intpulen(len);
		
		if (!headerIsOk())
		{
			if (parseHeader())
			{
				if (readtype != HTTPBuffer::HTTPBufferType_String)
				{
					if (readtype == HTTPBuffer::HTTPBufferType_File && outfilename != "")
					{
						HTTPParser::object->content()->readToFile(outfilename);
					}
					httpcallback(req,HTTPParser::object);
				}
			}
		}
		if (parseContent())
		{
			if (readtype == HTTPBuffer::HTTPBufferType_String)
			{
				httpcallback(req, HTTPParser::object);
			}
			closecallback(req.get());
		}
		
		sock->async_recv(readfbuffer(), readbufferlen(), Socket::ReceivedCallback(&HTTPRequestObject::recvCallback, this));

	}
	const char* parseFirstLine(const char* buffer, int len)
	{
		int pos = String::indexOf(buffer, len, HTTPHREADERLINEEND);
		if (pos == -1) return NULL;

		std::vector<std::string> tmp = String::split(buffer, pos, " ");
		if (tmp.size() < 2) return NULL;

		HTTPParser::object->statusCode() = Value(tmp[1]).readInt();

		std::string errstr;
		for (uint32_t i = 2; i < tmp.size(); i++)
		{
			errstr += (i == 2 ? "" : " ")+tmp[i];
		}

		HTTPParser::object->errorMessage() = errstr;

		return buffer + pos + strlen(HTTPHREADERLINEEND);
	}
	virtual std::string buildFirstLine()
	{
		std::string path = req->url().getPath();
		if (path == "") path = "/";
		return String::toupper(req->method()) + " " + path + " " + HTTPVERSION + HTTPHREADERLINEEND;
	}
};

struct HTTPClient::HTTPClientInternal
{
	shared_ptr<IOWorker>	worker;
	shared_ptr<HTTPRequestObject> obj;
	bool					worksuccess;
	Base::Semaphore 		sem;
	std::string				useragent;
	
	HTTPClientInternal() :worksuccess(false) {}
	void _successCallback(const shared_ptr<HTTPRequest>&, shared_ptr<HTTPResponse>&)
	{
		worksuccess = true;
		sem.post();
	}
};

HTTPClient::HTTPClient(const shared_ptr<IOWorker>& worker, const std::string& useragent)
{
	internal = new HTTPClientInternal();
	internal->worker = worker;
	internal->useragent = useragent;
}
HTTPClient::~HTTPClient()
{
	SAFE_DELETE(internal);
}

const shared_ptr<HTTPResponse> HTTPClient::request(const shared_ptr<HTTPRequest>& req, HTTPBuffer::HTTPBufferType type, const std::string& outfile)
{
	internal->obj = make_shared<HTTPRequestObject>(req,TCPClient::create(internal->worker), 
		HTTPCallback(&HTTPClientInternal::_successCallback, internal),
		HTTPRequestObject::CloseCallbak(),internal->useragent, type,outfile);
	
	bool ret = internal->obj->start();
	if (!ret)
	{
		return internal->obj->object;
	}

	internal->sem.pend(internal->obj->getObject()->timeout());
	if (!internal->worksuccess)
	{
		internal->obj->object->statusCode() = 408;
		internal->obj->object->errorMessage() = "Request Timeout";
	}

	return internal->obj->object;
}
const shared_ptr<HTTPResponse> HTTPClient::request(const std::string& url, const std::string& method, const shared_ptr<HTTPBuffer> buf, const std::map<std::string, Value>& headers, int timeout, HTTPBuffer::HTTPBufferType type, const std::string& outfile)
{
	shared_ptr<HTTPRequest> req = make_shared<HTTPRequest>();
	req->url() = url;
	if(buf != NULL)
		req->content() = buf;
	req->method() = method;
	req->headers() = headers;
	req->timeout() = timeout;

	return request(req,type,outfile);
}

struct HTTPAsyncClient::HTTPAsyncClientInternal
{
	struct ObjectInfo
	{
		HTTPCallback					callback;
		shared_ptr<HTTPRequestObject>	obj;
	};
	shared_ptr<IOWorker>	worker;
	shared_ptr<Timer>		timer;
	std::string				useragent;

	Mutex					mutex;
	std::map<HTTPRequest*, ObjectInfo> objectlist;


	void poolTimerProc(unsigned long)
	{
		std::list<ObjectInfo> timeoutlist;

		{
			Guard locker(mutex);

			uint64_t nowtime = Time::getCurrentMilliSecond();
			for (std::map<HTTPRequest*, ObjectInfo>::iterator iter = objectlist.begin(); iter != objectlist.end();)
			{
				if (nowtime > iter->second.obj->prevtime() && nowtime - iter->second.obj->prevtime() > iter->second.obj->reqeust()->timeout())
				{
					timeoutlist.push_back(iter->second);
					objectlist.erase(iter++);
				}
				else
				{
					iter++;
				}
			}
		}
		for (std::list<ObjectInfo>::iterator iter = timeoutlist.begin(); iter != timeoutlist.end(); iter++)
		{
			iter->obj->object->statusCode() = 408;
			iter->obj->object->errorMessage() = "Request Timeout";

			iter->callback(iter->obj->reqeust(), iter->obj->object);
		}
	}
	void _closeCallback(HTTPRequest* req)
	{
		ObjectInfo info;
		{
			Guard locker(mutex);
			std::map<HTTPRequest*, ObjectInfo>::iterator iter = objectlist.find(req);
			if (iter == objectlist.end())
			{
				return;
			}
			info = iter->second;
			objectlist.erase(iter);
		}
	}
	void _successCallback(const shared_ptr<HTTPRequest>& req, shared_ptr<HTTPResponse>& resp)
	{
		ObjectInfo obj;

		{
			Guard locker(mutex);
			std::map<HTTPRequest*, ObjectInfo>::iterator iter = objectlist.find(req.get());
			if (iter == objectlist.end())
			{
				return;
			}
			obj = iter->second;
		}
		obj.callback(req, resp);
	}
};


HTTPAsyncClient::HTTPAsyncClient(const shared_ptr<IOWorker>& worker, const std::string& useragent)
{
	internal = new HTTPAsyncClientInternal;
	internal->timer = make_shared<Timer>("HTTPAsyncClientInternal");
	internal->timer->start(Timer::Proc(&HTTPAsyncClientInternal::poolTimerProc, internal), 0, 1000);

}
HTTPAsyncClient::~HTTPAsyncClient()
{
	SAFE_DELETE(internal);
}
bool HTTPAsyncClient::request(const shared_ptr<HTTPRequest>& req, const HTTPCallback& callback, HTTPBuffer::HTTPBufferType type, const std::string& outfile)
{
	HTTPAsyncClientInternal::ObjectInfo obj;
	obj.obj = make_shared<HTTPRequestObject>(req,TCPClient::create(internal->worker), 
		HTTPCallback(&HTTPAsyncClientInternal::_successCallback, internal),
		HTTPRequestObject::CloseCallbak(&HTTPAsyncClientInternal::_closeCallback,internal),
		internal->useragent,type,outfile);
	obj.callback = callback;

	{
		Guard locker(internal->mutex);
		internal->objectlist[req.get()] = obj;
	}
	return obj.obj->startConnect();
}

}
}
