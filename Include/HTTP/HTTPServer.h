#ifndef __HTTPSERVER_H__
#define __HTTPSERVER_H__
#include "HTTPPublic.h"
#include "Base/Base.h"
#include "HTTP/HTTPParse.h"
using namespace Public::Base;
namespace Public {
namespace HTTP {

struct HTTPServrManager;

class HTTP_API HTTPServerRequest
{
public:
	typedef Function2<void,const shared_ptr<HTTPServerRequest>&, const std::string&> DisconnectCallback;
public:
	HTTPServerRequest(const shared_ptr<HTTPHeader>& header,const shared_ptr<ReadContent>& content,const shared_ptr<Socket>& sock);
	virtual ~HTTPServerRequest();

	const std::map<std::string, Value>& headers() const;
	Value header(const std::string& key) const;

	const std::string& method() const;

	const URL& url() const;

	const shared_ptr<ReadContent>& content() const;

	NetAddr remoteAddr() const;

	NetAddr myAddr() const;

	DisconnectCallback&	discallback();
private:
	struct  HTTPServerRequestInternal;
	HTTPServerRequestInternal* internal;
};

class HTTP_API HTTPServerResponse
{
public:
	HTTPServerResponse(const shared_ptr<HTTPCommunication>& commu, HTTPCacheType type);
	virtual ~HTTPServerResponse();

	int& statusCode();
	std::string& errorMessage();

	std::map<std::string, Value>& headers();
	Value header(const std::string& key);

	shared_ptr<WriteContent>& content();
private:
	struct HTTPServerResponseInternal;
	HTTPServerResponseInternal* internal;
};

class HTTP_API HTTPServerSession
{
	friend struct HTTPServrManager;
public:
	HTTPServerSession(const shared_ptr<HTTPCommunication>& commuSession, HTTPCacheType type);
	virtual ~HTTPServerSession();

	shared_ptr<HTTPServerRequest>		request;
	shared_ptr<HTTPServerResponse>	response;
private:
	void disconnected();
};

//web socket session
class HTTP_API WebSocketServerSession
{
	friend struct HTTPServrManager;
public:
	typedef Function1<void, WebSocketServerSession*> DisconnectCallback;
	typedef Function3<void, WebSocketServerSession*, const std::string&, WebSocketDataType> RecvDataCallback;
public:
	WebSocketServerSession(const shared_ptr<HTTPCommunication>& commuSession);
	virtual ~WebSocketServerSession();

	void start(const RecvDataCallback& datacallback, const DisconnectCallback& disconnectcallback);
	void stop();

	bool send(const std::string& data, WebSocketDataType type);

	const std::map<std::string, Value>& headers() const;
	Value header(const std::string& key) const;
	const URL& url() const;
	NetAddr remoteAddr() const;

	uint32_t sendListSize();

	static bool checkWebsocketHeader(const shared_ptr<HTTPHeader>& header);
private:
	void disconnected();
private:
	struct WebSocketServerSessionInternal;
	WebSocketServerSessionInternal* internal;
};

class HTTP_API HTTPServer
{
public:
	typedef Function1<void, const shared_ptr<HTTPServerSession>&> HTTPCallback;
	typedef Function1<void, const shared_ptr<WebSocketServerSession>&> WebsocketCallback;
public:
	HTTPServer(const shared_ptr<IOWorker>& worker, const std::string& useragent);
	~HTTPServer();	
	
	// path 为 请求的url,*为所有  ,callback监听消息的回掉,处理线程数据，先于run启用
	// Add resources using path and method-string, and an anonymous function
	bool listen(const std::string& path,const std::string& method,const HTTPCallback& callback, HTTPCacheType type = HTTPCacheType_Mem);

	// path 为 请求的url,*为所有  ,callback监听消息的回掉,处理线程数据，先于run启用
	// Add resources using path and method-string, and an anonymous function
	bool listen(const std::string& path, const WebsocketCallback& callback);

	// path 为 请求的url,*为所有  ,callback监听消息的回掉,处理线程数据，先于run启用
	// Add resources using path and method-string, and an anonymous function
	bool defaultListen(const WebsocketCallback& callback);

	// path 为 请求的url,*为所有  ,callback监听消息的回掉,处理线程数据，先于run启用
	// Add resources using path and method-string, and an anonymous function
	bool defaultListen(const std::string& method, const HTTPCallback& callback, HTTPCacheType type = HTTPCacheType_Mem);

	//异步监听
	bool run(uint32_t httpport);

	uint32_t listenPort() const;
private:
	struct HTTPServrInternal;
	HTTPServrInternal* internal;
};


}
}


#endif //__HTTPSERVER_H__
