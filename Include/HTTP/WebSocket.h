#pragma once

#include "Defs.h"
#include "Base/Base.h"
#include "Network/Network.h"
#include "HTTP/HTTPPublic.h"
using namespace Public::Base;
using namespace Public::Network;

namespace Public {
namespace HTTP {

typedef enum {
	WebSocketDataType_Txt,
	WebSocketDataType_Bin,
}WebSocketDataType;

class HTTPSession_Service;
//web socket session
class HTTP_API WebSocketSession
{
public:
	typedef Function1<void, WebSocketSession*> DisconnectCallback;
	typedef Function3<void, WebSocketSession*, const std::string&, WebSocketDataType> RecvDataCallback;
public:
	WebSocketSession(const shared_ptr<HTTPSession_Service>& session);
	virtual ~WebSocketSession();

	bool initProtocol();

	void start(const RecvDataCallback& datacallback,const DisconnectCallback& disconnectcallback);
	void stop();
	bool connected() const;
	
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
