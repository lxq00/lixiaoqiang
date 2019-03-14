#pragma once

#include "Defs.h"
#include "Base/Base.h"
#include "Network/Network.h"
using namespace Public::Base;
using namespace Public::Network;

namespace Public {
namespace HTTP {

typedef enum {
	WebSocketDataType_Txt,
	WebSocketDataType_Bin,
}WebSocketDataType;

//web socket session
class HTTP_API WebSocketSession
{
public:
	typedef Function1<bool, WebSocketSession*> ReadyCallback;
	typedef Function1<void, WebSocketSession*> ErrorCallback;
	WebSocketSession(const shared_ptr<Socket>& sock,const ReadyCallback& readycallback,const ErrorCallback& errcallback);
public:
	typedef Function1<void, WebSocketSession*> DisconnectCallback;
	typedef Function3<void, WebSocketSession*, const std::string&, WebSocketDataType> RecvDataCallback;
public:
	virtual ~WebSocketSession();

	void start(const RecvDataCallback& datacallback,const DisconnectCallback& disconnectcallback);
	bool connected() const;
	
	bool send(const std::string& data, WebSocketDataType type);

	const std::map<std::string, Value>& headers() const;
	Value header(const std::string& key) const;
	const URL& url() const;
	NetAddr remoteAddr() const;
private:
	struct WebSocketSessionInternal;
	WebSocketSessionInternal* internal;
};

//web socket server
class HTTP_API WebSocketServer
{
public:
	typedef Function1<bool, const shared_ptr< WebSocketSession> &> AcceptCallback;
public:
	WebSocketServer(const shared_ptr<IOWorker>& worker);
	~WebSocketServer();

	// path 为 请求的url,*为所有  ,callback监听消息的回掉,处理线程数据，先于run启用
	// Add resources using path and method-string, and an anonymous function
	bool listen(const std::string& path, const AcceptCallback& callback);

	bool startAccept(uint32_t port);
	bool stopAccept();
private:
	struct WebSocketServerInternal;
	WebSocketServerInternal* internal;
};

//web socket client
class HTTP_API WebSocketClient
{
public:
	typedef Function1<void, WebSocketClient*> DisconnectCallback;
	typedef Function3<void, WebSocketClient*, const std::string&, WebSocketDataType> RecvDataCallback;
	typedef Function1<void, WebSocketClient*> ConnnectCallback;
public:
	WebSocketClient(const shared_ptr<IOWorker>& worker,const std::map<std::string,Value>&headers= std::map<std::string, Value>());
	~WebSocketClient();

	bool connect(const URL& url,uint32_t timout_ms, const RecvDataCallback& datacallback, const DisconnectCallback& disconnectcallback);
	bool startConnect(const URL& url, const ConnnectCallback& connectcallback, const RecvDataCallback& datacallback, const DisconnectCallback& disconnectcallback);
	bool disconnect();
	bool send(const std::string& data, WebSocketDataType type);
private:
	struct WebSocketClientInternal;
	WebSocketClientInternal* internal;
};

}
}
