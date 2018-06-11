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

typedef SimpleWeb::Server<SimpleWeb::HTTP> SIMPLEHTTPServer;
#ifdef HAVE_OPENSSL
typedef SimpleWeb::Server<SimpleWeb::HTTPS> SIMPLEHTTPSServer;
#endif


template<typename LISTENHTTPResponse,typename LISTENHTTPRequest, bool HTTPS>
struct HTTPListenInfo
{
public:
	HTTPListenInfo() {}
	~HTTPListenInfo() {}
	void httpListenCallback(const std::shared_ptr<LISTENHTTPResponse>& response, const std::shared_ptr<LISTENHTTPRequest>& request)
	{
		shared_ptr<HTTPRequest> ack_request(new HTTPRequest);
		{
			std::string url = HTTPS ? "https://*" : "http://*";
			url += request->path;
			if (request->query_string != "")
			{
				url += std::string("?") + request->query_string;
			}
			ack_request->method = request->method;
			ack_request->url.href(url);
			ack_request->buf.write(request->content.string());

			for (SimpleWeb::CaseInsensitiveMultimap::const_iterator iter = request->header.begin(); iter != request->header.end(); iter++)
			{
				ack_request->headers[iter->first] = iter->second;
			}

		}
		shared_ptr<HTTPResponse> ack_response(new HTTPResponse());


		callback(ack_request, ack_response);


		SimpleWeb::CaseInsensitiveMultimap ackhreaders;
		for (std::map<std::string, URI::Value>::const_iterator iter = ack_response->headers.begin(); iter != ack_response->headers.end(); iter++)
		{
			ackhreaders.insert(make_pair(iter->first, iter->second.readString()));
		}

		if (ack_response->statusCode != 200)
		{
			response->write((SimpleWeb::StatusCode)ack_response->statusCode, ack_response->errorMessage, ackhreaders);
		}
		else
		{
			response->write(ack_response->buf.buf.str(), ackhreaders);
		}
	}
	HTTPServer::HttpListenCallback callback;
};



struct WebInterface
{
	WebInterface(){}
	virtual ~WebInterface() {}

	virtual bool resource(const std::string& path, const std::string& method, const HTTPServer::HttpListenCallback& callback) = 0;

	virtual bool start(int port, int threadnum) = 0;

	virtual void dosource(const std::map<std::string, std::map<std::string, HTTPServer::HttpListenCallback> >&resourceList) = 0;
};

template<typename SIMHTTPServer,bool HTTPS>
struct WEBHTTPServer:public Thread,public WebInterface
{
	typedef typename SIMHTTPServer::Response LISTENHTTPResponse;
	typedef typename SIMHTTPServer::Request  LISTENHTTPRequest;
	typedef typename  HTTPListenInfo<LISTENHTTPResponse, LISTENHTTPRequest, HTTPS> WEBHTTPListen;
public:
	WEBHTTPServer(const std::string &cert_file, const std::string &private_key_file, const std::string &verify_file) :Thread("HTTPServer"), WebInterface()
	{
		server = make_shared<SIMHTTPServer>(cert_file,private_key_file,verify_file);
	}

	WEBHTTPServer():Thread("HTTPServer"),WebInterface()
	{
		server = make_shared<SIMHTTPServer>();
	}
	~WEBHTTPServer()
	{
		destroyThread();
		server = NULL;
		callbacklist.clear();
	}

	virtual void dosource(const std::map<std::string, std::map<std::string, HTTPServer::HttpListenCallback> >&resourceList)
	{
		for (std::map<std::string, std::map<std::string, HTTPServer::HttpListenCallback> >::const_iterator iter = resourceList.begin(); iter != resourceList.end(); iter++)
		{
			for (std::map<std::string, HTTPServer::HttpListenCallback> ::const_iterator miter = iter->second.begin(); miter != iter->second.end(); miter++)
			{
				resource(iter->first, miter->first, miter->second);
			}
		}
	}

	virtual bool resource(const std::string& path, const std::string& method, const HTTPServer::HttpListenCallback& callback)
	{
		std::string flag = path + "--!!--" + method;

		std::map<std::string, shared_ptr<WEBHTTPListen> >::iterator iter = callbacklist.find(flag);
		if (iter != callbacklist.end()) return false;

		shared_ptr<WEBHTTPListen> info(new WEBHTTPListen());
		info->callback = callback;

		std::function<void(std::shared_ptr<SIMHTTPServer::Response>, std::shared_ptr<SIMHTTPServer::Request>)> listencallback
			= std::bind(&WEBHTTPListen::httpListenCallback,info,std::placeholders::_1, std::placeholders::_2);
		
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
	std::map<std::string, shared_ptr<WEBHTTPListen> > callbacklist;
	
};

struct HTTPServer::HTTPServrInternal
{
	shared_ptr<WebInterface> web;
};
#ifdef HAVE_OPENSSL
HTTPServer::HTTPServer(const std::string &cert_file, const std::string &private_key_file, const std::string &verify_file)
{
	internal = new HTTPServrInternal();
	internal->web = make_shared<WEBHTTPServer<SIMPLEHTTPSServer,true> >(cert_file, private_key_file, verify_file);
}
#endif

HTTPServer::HTTPServer()
{
	internal = new HTTPServrInternal();
	internal->web = make_shared<WEBHTTPServer<SIMPLEHTTPServer,false> >();
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
	internal->web->dosource(resource);

	return internal->web->start(httpport,threadNum);
}

}
}
#endif

