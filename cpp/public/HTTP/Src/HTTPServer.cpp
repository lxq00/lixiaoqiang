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
typedef SimpleWeb::ServerBase<SimpleWeb::HTTP> SIMPLEHTTPServerBase;
#ifdef HAVE_OPENSSL
typedef SimpleWeb::Server<SimpleWeb::HTTPS> SIMPLEHTTPSServer;
typedef SimpleWeb::ServerBase<SimpleWeb::HTTPS> SIMPLEHTTPSServerBase;
#endif

template<typename LISTENHTTPResponse>
class ServerResponse:public HTTPResponse
{
public:
	ServerResponse(const std::shared_ptr<LISTENHTTPResponse>& _response):response(_response)
	{
	}
	~ServerResponse()
	{
		SimpleWeb::CaseInsensitiveMultimap ackhreaders;
		for (std::map<std::string, URI::Value>::const_iterator iter = headers.begin(); iter != headers.end(); iter++)
		{
			ackhreaders.insert(make_pair(iter->first, iter->second.readString()));
		}

		if (statusCode != 200)
		{
			response->write((SimpleWeb::StatusCode)statusCode, errorMessage, ackhreaders);
		}
		else
		{
			response->write(SimpleWeb::StatusCode::success_ok, buf.read(),ackhreaders);
		}
	}
private:
	std::shared_ptr<LISTENHTTPResponse> response;
};

struct HTTPListenInfo
{
public:
	HTTPListenInfo(const HTTPServer::HttpListenCallback& _callback):callback(_callback) {}
	~HTTPListenInfo() {}
	void httpListenCallback(const std::shared_ptr<SIMPLEHTTPServerBase::Response>& response, const std::shared_ptr<SIMPLEHTTPServerBase::Request>& request)
	{
		doCallback<SIMPLEHTTPServerBase::Response, SIMPLEHTTPServerBase::Request, false>(response, request);
	}
#ifdef HAVE_OPENSSL
	void httpsListenCallback(const std::shared_ptr<SIMPLEHTTPSServerBase::Response>& response, const std::shared_ptr<SIMPLEHTTPSServerBase::Request>& request)
	{
		doCallback<SIMPLEHTTPSServerBase::Response, SIMPLEHTTPSServerBase::Request, false>(response, request);
	}
#endif
private:
	template<typename LISTENHTTPResponse, typename LISTENHTTPRequest, bool HTTPS>
	void doCallback(const std::shared_ptr<LISTENHTTPResponse>& response, const std::shared_ptr<LISTENHTTPRequest>& request)
	{
		shared_ptr<HTTPRequest> ack_request(new HTTPRequest());
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
		shared_ptr<HTTPResponse> ack_response(new ServerResponse<LISTENHTTPResponse>(response));
		callback(ack_request, ack_response);
	}
private:
	HTTPServer::HttpListenCallback callback;
};


struct WEBHTTPServer:public Thread
{
public:
	WEBHTTPServer():Thread("HTTPServer")
	{
	}
	virtual ~WEBHTTPServer()
	{
		destroyThread();
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
		std::string flag = String::toupper(path + "--!!--" + method);

		std::map<std::string, shared_ptr<HTTPListenInfo> >::iterator iter = callbacklist.find(flag);
		if (iter != callbacklist.end()) return false;

		shared_ptr<HTTPListenInfo> info(new HTTPListenInfo(callback));

		doResource(path, method, info);

		callbacklist[flag] = info;

		return true;
	}

	virtual bool start(int port, int threadnum)
	{
		return createThread();
	}
private:
	virtual bool doResource(const std::string& path, const std::string& method, shared_ptr<HTTPListenInfo>& info) = 0;
private:
	virtual void threadProc() = 0;

protected:
	std::map<std::string,shared_ptr<HTTPListenInfo> > callbacklist;
	
};

struct HTTPServerResource :public WEBHTTPServer
{
public:
	HTTPServerResource()
	{
		server = make_shared<SIMPLEHTTPServer>();
	}
	virtual bool doResource(const std::string& path, const std::string& method, shared_ptr<HTTPListenInfo>& info)
	{
		std::function<void(std::shared_ptr<SIMPLEHTTPServerBase::Response>, std::shared_ptr<SIMPLEHTTPServerBase::Request>)> listencallback
			= std::bind(&HTTPListenInfo::httpListenCallback, info, std::placeholders::_1, std::placeholders::_2);

		if (path == "*" || path == "") (*server.get()).default_resource[String::toupper(method)] = listencallback;
		else (*server.get()).resource[path][String::toupper(method)] = listencallback;
		return true;
	}
private:
	virtual void threadProc()
	{
		if (server != NULL) server->start();
	}
	virtual bool start(int port, int threadnum)
	{
		server->config.port = port;
		server->config.thread_pool_size = threadnum;

		return WEBHTTPServer::start(port,threadnum);
	}
private:
	shared_ptr<SIMPLEHTTPServer> server;
};

#ifdef HAVE_OPENSSL
struct HTTPSServerResource :public WEBHTTPServer
{
public:
	HTTPSServerResource(const std::string &cert_file, const std::string &private_key_file, const std::string &verify_file)
	{
		server = make_shared<SIMPLEHTTPSServer>(cert_file, private_key_file, verify_file);
	}
	virtual bool doResource(const std::string& path, const std::string& method, shared_ptr<HTTPListenInfo>& info)
	{
		std::function<void(std::shared_ptr<SIMPLEHTTPSServerBase::Response>, std::shared_ptr<SIMPLEHTTPSServerBase::Request>)> listencallback
			= std::bind(&HTTPListenInfo::httpsListenCallback, info, std::placeholders::_1, std::placeholders::_2);

		if (path == "*" || path == "") (*server.get()).default_resource[String::toupper(method)] = listencallback;
		else (*server.get()).resource[path][String::toupper(method)] = listencallback;

		return true;
	}
private:
	virtual void threadProc()
	{
		if (server != NULL) server->start();
	}
	virtual bool start(int port, int threadnum)
	{
		server->config.port = port;
		server->config.thread_pool_size = threadnum;

		return WEBHTTPServer::start(port, threadnum);
	}
private:
	shared_ptr<SIMPLEHTTPSServer> server;
};
#endif

struct HTTPServer::HTTPServrInternal
{
	shared_ptr<WEBHTTPServer> web;
};
#ifdef HAVE_OPENSSL
HTTPServer::HTTPServer(const std::string &cert_file, const std::string &private_key_file, const std::string &verify_file)
{
	internal = new HTTPServrInternal();
	internal->web = make_shared<HTTPSServerResource>(cert_file, private_key_file, verify_file);
}
#endif

HTTPServer::HTTPServer()
{
	internal = new HTTPServrInternal();
	internal->web = make_shared<HTTPServerResource>();
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
#include "boost/asio.hpp"

//异步监听
bool HTTPServer::run(uint32_t httpport, uint32_t threadNum)
{


	internal->web->dosource(resource);

	return internal->web->start(httpport,threadNum);
}

}
}
#endif

