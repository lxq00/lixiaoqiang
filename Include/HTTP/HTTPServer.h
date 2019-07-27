#ifndef __HTTPSERVER_H__
#define __HTTPSERVER_H__
#include "HTTPPublic.h"
#include "Base/Base.h"
using namespace Public::Base;
namespace Public {
namespace HTTP {

class HTTP_API HTTPSession
{
public:
	HTTPSession();
	virtual ~HTTPSession();

	shared_ptr<HTTPRequest>		request;
	shared_ptr<HTTPResponse>	response;
private:
	struct HTTPSessionInternal;
	HTTPSessionInternal* internal;
};

//web socket session
class HTTP_API WebSocketSession
{
public:
	typedef Function1<void, WebSocketSession*> DisconnectCallback;
	typedef Function3<void, WebSocketSession*, const std::string&, WebSocketDataType> RecvDataCallback;
public:
	WebSocketSession();
	virtual ~WebSocketSession();

	void start(const RecvDataCallback& datacallback, const DisconnectCallback& disconnectcallback);
	void stop();

	bool send(const std::string& data, WebSocketDataType type);

	const std::map<std::string, Value>& headers() const;
	Value header(const std::string& key) const;
	const URL& url() const;
	NetAddr remoteAddr() const;

	uint32_t sendListSize();
private:
	struct WebSocketSessionInternal;
	WebSocketSessionInternal* internal;
};

class HTTP_API HTTPServer
{
public:
	typedef enum {
		CacheType_Mem = 0,
		CacheType_File,
	}CacheType;
	typedef Function1<void, const shared_ptr<HTTPSession>&> HTTPCallback;
	typedef Function1<void, const shared_ptr<WebSocketSession>&> WebsocketCallback;
public:
	HTTPServer(const shared_ptr<IOWorker>& worker, const std::string& useragent);
	~HTTPServer();	
	
	// path 为 请求的url,*为所有  ,callback监听消息的回掉,处理线程数据，先于run启用
	// Add resources using path and method-string, and an anonymous function
	bool listen(const std::string& path,const std::string& method,const HTTPCallback& callback, CacheType type = CacheType_Mem);

	// path 为 请求的url,*为所有  ,callback监听消息的回掉,处理线程数据，先于run启用
	// Add resources using path and method-string, and an anonymous function
	bool listen(const std::string& path, const WebsocketCallback& callback);

	// path 为 请求的url,*为所有  ,callback监听消息的回掉,处理线程数据，先于run启用
	// Add resources using path and method-string, and an anonymous function
	bool defaultListen(const WebsocketCallback& callback);

	// path 为 请求的url,*为所有  ,callback监听消息的回掉,处理线程数据，先于run启用
	// Add resources using path and method-string, and an anonymous function
	bool defaultListen(const std::string& method, const HTTPCallback& callback, CacheType type = CacheType_Mem);

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
