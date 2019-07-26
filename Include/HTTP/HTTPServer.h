#ifndef __HTTPSERVER_H__
#define __HTTPSERVER_H__
#include "HTTPPublic.h"
#include "Base/Base.h"
#include "HTTP/WebSocket.h"
using namespace Public::Base;
namespace Public {
namespace HTTP {

class HTTP_API HTTPServer
{
public:
	typedef enum {
		CacheType_Mem = 0,
		CacheType_File,
	}CacheType;
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
