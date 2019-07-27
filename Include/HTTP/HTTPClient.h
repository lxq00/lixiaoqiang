#ifndef __XM_HTTPCLIENT_H__
#define __XM_HTTPCLIENT_H__

#include "HTTPPublic.h"

 namespace Public{
 namespace HTTP{

 ///HTTP¿Í»§¶Ë·â×°¿â
 class HTTP_API HTTPClient
 {
 public:
 	HTTPClient(const shared_ptr<IOWorker>& worker,const std::string& useragent);
 	~HTTPClient();

	const shared_ptr<HTTPResponse> request(const shared_ptr<HTTPRequest>& req,const std::string& saveasfile = "");
	const shared_ptr<HTTPResponse> request(const std::string& method, const std::string& url , const shared_ptr<HTTPContent>& buf, const std::map<std::string, Value>& headers = std::map<std::string, Value>(), int timeout = 30000);
private:	 
	 struct HTTPClientInternal;
	 HTTPClientInternal* internal;
 };

 class HTTP_API HTTPAsyncClient
 {
 public:
	 HTTPAsyncClient(const shared_ptr<IOWorker>& worker, const std::string& useragent);
	 ~HTTPAsyncClient();
	 bool request(const shared_ptr<HTTPRequest>& req, const HTTPCallback& callback, const std::string& saveasfile = "");
 private:
	 struct HTTPAsyncClientInternal;
	 HTTPAsyncClientInternal*internal;
 };
 
 //web socket client
 class HTTP_API WebSocketClient
 {
 public:
	 typedef Function1<void, WebSocketClient*> DisconnectCallback;
	 typedef Function3<void, WebSocketClient*, const std::string&, WebSocketDataType> RecvDataCallback;
	 typedef Function1<void, WebSocketClient*> ConnnectCallback;
 public:
	 WebSocketClient(const shared_ptr<IOWorker>& worker, const std::map<std::string, Value>& headers = std::map<std::string, Value>());
	 ~WebSocketClient();

	 bool connect(const URL& url, uint32_t timout_ms, const RecvDataCallback& datacallback, const DisconnectCallback& disconnectcallback);
	 bool startConnect(const URL& url, const ConnnectCallback& connectcallback, const RecvDataCallback& datacallback, const DisconnectCallback& disconnectcallback);
	 bool disconnect();
	 bool send(const std::string& data, WebSocketDataType type);
 private:
	 struct WebSocketClientInternal;
	 WebSocketClientInternal* internal;
 };

 }
 }

#endif //__XM_HTTPCLIENT_H__
