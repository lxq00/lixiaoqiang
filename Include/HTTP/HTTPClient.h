#ifndef __XM_HTTPCLIENT_H__
#define __XM_HTTPCLIENT_H__

#include "HTTPPublic.h"
#include "HTTPParse.h"

 namespace Public{
 namespace HTTP{

struct HTTPClientManager;
class HTTP_API HTTPClientRequest
{
	friend struct HTTPClientManager;
public:
	typedef Function2<void, const shared_ptr<HTTPClientRequest>&, const std::string&> DisconnectCallback;
public:
	HTTPClientRequest(const std::string& method,const std::string& url,HTTPCacheType type);
	virtual ~HTTPClientRequest();

	std::string& url();

	std::string& method();

	std::map<std::string, Value>& headers();
	Value header(const std::string& key);

	shared_ptr<WriteContent>& content();
	
	uint32_t& timeout();

	DisconnectCallback&	discallback();
private:
	shared_ptr<HTTPHeader> header();
private:
	struct  HTTPClientRequestInternal;
	HTTPClientRequestInternal* internal;
};

class HTTP_API HTTPClientResponse
{
	friend struct HTTPClientManager;
public:
	HTTPClientResponse(const shared_ptr<HTTPCommunication>& commu, HTTPCacheType type, const std::string& filename = "");
	virtual ~HTTPClientResponse();

	int statusCode() const;
	const std::string& errorMessage() const;

	const std::map<std::string, Value>& headers() const;
	Value header(const std::string& key) const;

	const shared_ptr<ReadContent>& content() const;
private:
	shared_ptr<HTTPHeader> header();
private:
	struct HTTPClientResponseInternal;
	HTTPClientResponseInternal* internal;
};


 ///HTTP¿Í»§¶Ë·â×°¿â
 class HTTP_API HTTPClient
 {
 public:
 	HTTPClient(const shared_ptr<IOWorker>& worker,const std::string& useragent);
 	~HTTPClient();

	const shared_ptr<HTTPClientResponse> request(const shared_ptr<HTTPClientRequest>& req,const std::string& saveasfile = "");
private:	 
	 struct HTTPClientInternal;
	 HTTPClientInternal* internal;
 };

 class HTTP_API HTTPAsyncClient
 {
 public:
	 typedef Function2<void, const shared_ptr<HTTPClientRequest>&, const shared_ptr<HTTPClientResponse>& > HTTPCallback;
 public:
	 HTTPAsyncClient(const shared_ptr<IOWorker>& worker, const std::string& useragent);
	 ~HTTPAsyncClient();
	 bool request(const shared_ptr<HTTPClientRequest>& req, const HTTPCallback& callback, const std::string& saveasfile = "");
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
	 typedef Function3<void, WebSocketClient*,bool,const std::string&> ConnnectCallback;
 public:
	 WebSocketClient(const shared_ptr<IOWorker>& worker, const std::string& useragent,const std::map<std::string, Value>& headers = std::map<std::string, Value>());
	 ~WebSocketClient();

	 bool connect(const std::string& url, uint32_t timout_ms, const RecvDataCallback& datacallback, const DisconnectCallback& disconnectcallback);
	 bool startConnect(const std::string& url, const ConnnectCallback& connectcallback, const RecvDataCallback& datacallback, const DisconnectCallback& disconnectcallback);
	 bool disconnect();
	 bool send(const std::string& data, WebSocketDataType type);
	 uint32_t sendListSize();
 private:
	 struct WebSocketClientInternal;
	 WebSocketClientInternal* internal;
 };

 }
 }

#endif //__XM_HTTPCLIENT_H__
