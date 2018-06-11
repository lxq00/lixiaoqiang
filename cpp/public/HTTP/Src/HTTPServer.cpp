#include "Base/Defs.h"
#include "HTTP/Defs.h"
#ifdef GCCSUPORTC11

#include "Simple-Web-Server-master/server_http.hpp"
#ifdef HAVE_OPENSSL
#include "Simple-Web-Server-master/server_https.hpp"
#endif
#include "HTTP/HTTPServer.h"

namespace Public {
namespace HTTP {


struct WebRequest
{
	virtual ~WebRequest() {}
	virtual std::map<std::string, URI::Value> headers() const = 0;
	virtual std::string header(const std::string& key) const = 0;
	virtual std::string method() const = 0;
	virtual shared_ptr<URL> url() const = 0;
	virtual shared_ptr<HTTPBuffer> buffer() = 0;
};

struct WebResponse
{
	virtual ~WebResponse(){}

	virtual bool writeError(int errocode, const std::string& statusMessage,const std::map<std::string, URI::Value>& header) = 0;
	virtual bool writeOk(const std::string& content, const std::map<std::string, URI::Value>& header) = 0;
};


struct HTTPServer::HTTPRequest::HTTPRequestInternal
{
	shared_ptr<WebRequest> request;
};

HTTPServer::HTTPRequest::HTTPRequest()
{
	internal = new HTTPRequest::HTTPRequestInternal();
}
HTTPServer::HTTPRequest::~HTTPRequest()
{
	SAFE_DELETE(internal);
}
std::map<std::string, URI::Value> HTTPServer::HTTPRequest::headers() const
{
	return internal->request->headers();
}

std::string HTTPServer::HTTPRequest::header(const std::string& key) const
{
	return internal->request->header(key);
}
std::string HTTPServer::HTTPRequest::method() const
{
	return internal->request->method();
}
shared_ptr<URL> HTTPServer::HTTPRequest::url() const
{
	return internal->request->url();
}
shared_ptr<HTTPBuffer> HTTPServer::HTTPRequest::buffer() const
{
	return internal->request->buffer();
}

struct HTTPServer::HTTPResponse::HTTPResponseInternal
{
	shared_ptr<WebResponse> response;

	uint32_t code;
	std::string statusMessage;
	std::map<std::string, URI::Value> header;
	shared_ptr<HTTPBuffer> buf;
};

HTTPServer::HTTPResponse::HTTPResponse()
{
	internal = new HTTPResponseInternal;
	internal->code = 200;
}
HTTPServer::HTTPResponse::~HTTPResponse()
{
	SAFE_DELETE(internal);
}

bool HTTPServer::HTTPResponse::statusCode(uint32_t code, const std::string& statusMessage)
{
	internal->code = code;
	internal->statusMessage = statusMessage;

	return true;
}
bool HTTPServer::HTTPResponse::setHeader(const std::string& key, const URI::Value& val)
{
	internal->header[key] = val;

	return true;
}

shared_ptr<HTTPBuffer> HTTPServer::HTTPResponse::buffer()
{
	return internal->buf;
}

//end后将会发送数据，buffer操作将会无效
bool HTTPServer::HTTPResponse::end()
{
	if (internal->code == 200) internal->response->writeOk(internal->buf.buf.string(), internal->header);
	else internal->response->writeError(internal->code, internal->statusMessage, internal->header);

	return true;
}

template<typename ServerBaseRequest>
struct HTTPWebRequest :public WebRequest
{
	HTTPWebRequest(const std::shared_ptr<ServerBaseRequest>& req, HTTPBufferCacheType type,bool ishttp) :request(req)
	{
		std::string url;
		url += request->path;
		if (request->query_string != "")
		{
			url += std::string("?") + request->query_string;
		}

		uri = make_shared<URL>(url);
		buf = make_shared<HTTPBuffer>(type);
		buf->write(request->content.string());
	}
	virtual ~HTTPWebRequest() {}
	virtual std::map<std::string, URI::Value> headers() const
	{
		std::map<std::string, URI::Value> headerstmp;
		for (SimpleWeb::CaseInsensitiveMultimap::const_iterator iter = request->header.begin(); iter != request->header.end(); iter++)
		{
			headerstmp[iter->first] = iter->second;
		}

		return headerstmp;
	}
	virtual std::string header(const std::string& key) const
	{
		SimpleWeb::CaseInsensitiveMultimap::const_iterator iter = request->header.find(key);
		if (iter == request->header.end()) return "";

		return iter->second;
	}
	virtual std::string method() const
	{
		return request->method;
	}
	virtual shared_ptr<URL> url() const
	{
		return uri;
	}
	virtual shared_ptr<HTTPBuffer> buffer()
	{
		return buf;
	}

	std::shared_ptr<ServerBaseRequest> request;

	shared_ptr<URL>				 uri;
	shared_ptr<HTTPBuffer>		 buf;
};

template<typename ServerBaseResponse>
struct HTTPWebResponse :public WebResponse
{
	HTTPWebResponse(const std::shared_ptr<ServerBaseResponse>& res) :response(res) {}

	virtual bool writeError(int errocode, const std::string& statusMessage,const std::map<std::string, URI::Value>& header)
	{
		SimpleWeb::CaseInsensitiveMultimap ackhreaders;
		for (std::map<std::string, URI::Value>::const_iterator iter = header.begin(); iter != header.end(); iter++)
		{
			ackhreaders.insert(make_pair(iter->first, iter->second.readString()));
		}

		response->write((SimpleWeb::StatusCode)errocode, statusMessage, ackhreaders);

		return true;
	}
	virtual bool writeOk(const std::string& content, const std::map<std::string, URI::Value>& header)
	{
		SimpleWeb::CaseInsensitiveMultimap ackhreaders;
		for (std::map<std::string, URI::Value>::const_iterator iter = header.begin(); iter != header.end(); iter++)
		{
			ackhreaders.insert(make_pair(iter->first, iter->second.readString()));
		}

		response->write(content, ackhreaders);

		return true;
	}

	std::shared_ptr<ServerBaseResponse> response;
};

struct WebInterface
{
	WebInterface(bool _ishttp):ishttp(_ishttp) {}
	virtual ~WebInterface() {}

	virtual bool resource(const std::string& path, const std::string& method, const HTTPServer::HttpListenCallback& callback) = 0;

	virtual bool start(int port, int threadnum) = 0;

	template<typename ServerBaseRequest,typename ServerBaseResponse>
	void doCallback(const HTTPServer::HttpListenCallback& callback,const std::shared_ptr<ServerBaseResponse>& response,const std::shared_ptr<ServerBaseRequest>& request)
	{
		shared_ptr<HTTPServer::HTTPResponse> ack_response(new HTTPServer::HTTPResponse());
		ack_response->internal->response = make_shared<HTTPWebResponse<ServerBaseResponse> >(response);
		ack_response->internal->buf = make_shared<HTTPBuffer>(type);

		shared_ptr<HTTPServer::HTTPRequest> ack_request(new HTTPServer::HTTPRequest);
		ack_request->internal->request = make_shared<HTTPWebRequest<ServerBaseRequest> >(request,type, ishttp);

		HTTPServer::HttpListenCallback callbacktmp = callback;

		callbacktmp(ack_request, ack_response);

		ack_response->end();
	}

	std::string			cachepath;
	bool				ishttp;
};



template<typename ServerBaseRequest, typename ServerBaseResponse>
struct HTTPListenInfoIterface
{
	void httpListenCallback(const std::shared_ptr<ServerBaseResponse>& response, const std::shared_ptr<ServerBaseRequest>& request)
	{
		inter->doCallback<ServerBaseRequest, ServerBaseResponse>(callback, response, request);
	}

	HTTPServer::HttpListenCallback callback;
	WebInterface* inter;
};



struct HTTPWeb:public Thread,public WebInterface
{
	typedef SimpleWeb::Server<SimpleWeb::HTTP> SIMPLEHTTPServer;
	typedef SimpleWeb::ServerBase<SimpleWeb::HTTP> SIMPLEHTTPBase;	
	typedef SimpleWeb::ServerBase<SimpleWeb::HTTP>::Request SIMPLEHTTPBaseRequest;
	typedef SimpleWeb::ServerBase<SimpleWeb::HTTP>::Response SIMPLEHTTPBaseResponse;
	typedef HTTPListenInfoIterface<SIMPLEHTTPBaseRequest, SIMPLEHTTPBaseResponse> HTTPListenInfo;
public:
	HTTPWeb():Thread("HTTPWeb"),WebInterface(true)
	{
		server = make_shared<SIMPLEHTTPServer>();
	}
	~HTTPWeb() 
	{
		destroyThread();
		server = NULL;
		callbacklist.clear();
	}

	virtual bool resource(const std::string& path, const std::string& method, const HTTPServer::HttpListenCallback& callback)
	{
		std::string flag = path + "--!!--" + method;

		std::map<std::string, shared_ptr<HTTPListenInfo> >::iterator iter = callbacklist.find(flag);
		if (iter != callbacklist.end()) return false;

		shared_ptr<HTTPListenInfo> info(new HTTPListenInfo);
		info->callback = callback;
		info->inter = this;

		std::function<void(std::shared_ptr<SIMPLEHTTPBaseResponse>, std::shared_ptr<SIMPLEHTTPBaseRequest>)> listencallback
			= std::bind(&HTTPListenInfo::httpListenCallback,info,std::placeholders::_1, std::placeholders::_2);
		
		if (path == "*" || path == "") (*server.get()).default_resource[method] = listencallback;
		else (*server.get()).resource[path][method] = listencallback;

		callbacklist[flag] = info;

		return true;
	}

	virtual bool start(int port, int threadnum)
	{
		server->config.port = port;
		server->config.thread_pool_size = threadnum;

		return createThread();
	}
private:
	void threadProc()
	{
		if (server != NULL) server->start();
	}

private:
	shared_ptr<SIMPLEHTTPServer> server;
	std::map<std::string, shared_ptr<HTTPListenInfo> > callbacklist;
	
};
#ifdef HAVE_OPENSSL
struct HTTPSWeb :public Thread, public WebInterface
{
	typedef SimpleWeb::Server<SimpleWeb::HTTPS> SIMPLEHTTPSServer;
	typedef SimpleWeb::ServerBase<SimpleWeb::HTTPS> SIMPLEHTTPSBase;
	typedef SimpleWeb::ServerBase<SimpleWeb::HTTPS>::Request SIMPLEHTTPSBaseRequest;
	typedef SimpleWeb::ServerBase<SimpleWeb::HTTPS>::Response SIMPLEHTTPSBaseResponse;
	typedef HTTPListenInfoIterface<SIMPLEHTTPSBaseRequest, SIMPLEHTTPSBaseResponse> HTTPSListenInfo;
public:
	HTTPSWeb(const std::string &cert_file, const std::string &private_key_file, const std::string &verify_file) :Thread("HTTPSWeb"), WebInterface(false)
	{
		server = make_shared<SIMPLEHTTPSServer>(cert_file,private_key_file,verify_file);
	}
	~HTTPSWeb()
	{
		destroyThread();
		server = NULL;
		callbacklist.clear();
	}

	virtual bool resource(const std::string& path, const std::string& method, const HTTPServer::HttpListenCallback& callback)
	{
		std::string flag = path + "    " + method;

		std::map<std::string, shared_ptr<HTTPSListenInfo> >::iterator iter = callbacklist.find(flag);
		if (iter != callbacklist.end()) return false;

		shared_ptr<HTTPSListenInfo> info(new HTTPSListenInfo);
		info->callback = callback;
		info->inter = this;

		std::function<void(std::shared_ptr<SIMPLEHTTPSBaseResponse>, std::shared_ptr<SIMPLEHTTPSBaseRequest>)> listencallback
			= std::bind(&HTTPSListenInfo::httpListenCallback, info, std::placeholders::_1, std::placeholders::_2);

		if (path == "*" || path == "") (*server.get()).default_resource[method] = listencallback;
		else (*server.get()).resource[path][method] = listencallback;

		callbacklist[flag] = info;

		return true;
	}

	virtual bool start(int port, int threadnum)
	{
		server->config.port = port;
		server->config.thread_pool_size = threadnum;

		return createThread();
	}
private:
	void threadProc()
	{
		if (server != NULL) server->start();
	}

private:
	shared_ptr<SIMPLEHTTPSServer> server;
	std::map<std::string, shared_ptr<HTTPSListenInfo> > callbacklist;
};
#endif
struct HTTPServer::HTTPServrInternal
{
	shared_ptr<WebInterface> web;
	HTTPBufferCacheType type;
};
HTTPServer::HTTPServer(const std::string &cert_file, const std::string &private_key_file, const std::string &verify_file, HTTPBufferCacheType type)
{
#ifdef HAVE_OPENSSL
	internal = new HTTPServrInternal();
	internal->web = make_shared<HTTPSWeb>(cert_file, private_key_file, verify_file);

	std::string cachepath = File::getExcutableFileFullPath() + "/._httpcache";

	if (!File::access(cachepath.c_str(), File::accessExist))
	{
		File::makeDirectory(cachepath.c_str());
	}

	internal->web->type = type;
	internal->web->cachepath = cachepath;
#endif
}
HTTPServer::HTTPServer(HTTPBufferCacheType type)
{
	internal = new HTTPServrInternal();
	internal->web = make_shared<HTTPWeb>();

	std::string cachepath = File::getExcutableFileFullPath() + "/._httpcache";

	if (!File::access(cachepath.c_str(), File::accessExist))
	{
		File::makeDirectory(cachepath.c_str());
	}

	internal->web->type = type;
	internal->web->cachepath = cachepath;
}

HTTPServer::~HTTPServer()
{
	SAFE_DELETE(internal);
}

// path 为 请求的url,*为所有  ,callback监听消息的回掉,处理线程数据
bool HTTPServer::listen(const std::string& path, const std::string& method, const HttpListenCallback& callback)
{
	return internal->web->resource(path, method, callback);
}

//异步监听
bool HTTPServer::run(uint32_t httpport, uint32_t threadNum)
{
	return internal->web->start(httpport,threadNum);
}

}
}
#endif

