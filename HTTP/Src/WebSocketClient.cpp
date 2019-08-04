#include "HTTP/HTTPClient.h"
#include "WebSocketProtocol.h"
#include "HTTPCommunication.h"

namespace Public {
namespace HTTP {

class WebSocketClientRecvContent :public IContent,public WebSocketProtocol
{
public:
	WebSocketClient*						client;

	WebSocketClient::RecvDataCallback		recvcallback;
	
	WebSocketClientRecvContent(WebSocketClient* _client)
		:WebSocketProtocol(true,WebSocketProtocol::ParseDataCallback(&WebSocketClientRecvContent::parseDataCallback,this)), client(_client)
	{}
	~WebSocketClientRecvContent() {}

	void parseDataCallback(const std::string& data, WebSocketDataType type)
	{
		recvcallback(client,data, type);
	}

	uint32_t size() { return 0; }
	uint32_t append(const char* buffer, uint32_t len)
	{
		const char* usedaddr = parseProtocol(buffer, len);

		return usedaddr - buffer;
	}
	void read(String& data) {}
};

class WebSocketClientSendContent :public IContent, public WebSocketProtocol
{
public:
	WebSocketClientSendContent() :WebSocketProtocol(true, NULL){}
	~WebSocketClientSendContent() {}

	Mutex							mutex;
	std::list<std::string>			sendlist;

	uint32_t size() { return 0; }
	uint32_t append(const char* buffer, uint32_t len) { return len; }
	void read(String& data) 
	{
		Guard locker(mutex);

		if (sendlist.size()) return;

		data = sendlist.front();
		sendlist.pop_front();
	}
};

struct WebSocketClientManager:public HTTPCommunicationHandler,public enable_shared_from_this<WebSocketClientManager>
{
	WebSocketClient*					client;
	shared_ptr<HTTPCommunication>	commusession;

	shared_ptr<HTTPHeader>					sendheader;

	shared_ptr<WebSocketClientRecvContent>	recvcontent;
	shared_ptr<WebSocketClientSendContent>	sendcontent;

	shared_ptr<IOWorker>	worker;
	shared_ptr<Socket>		socket;
	uint64_t				startconnecttime;
	std::string				useragent;

	WebSocketClient::ConnnectCallback		connectcalblack;
	WebSocketClient::RecvDataCallback		datacallback;
	WebSocketClient::DisconnectCallback		disconnectcallback;

	shared_ptr<Timer>		pooltimer;

	bool					socketconnectsuccess;
	uint32_t				timeout;

	WebSocketClientManager() :startconnecttime(Time::getCurrentMilliSecond()), socketconnectsuccess(false), timeout(10000){}
	~WebSocketClientManager() {}

	bool startConnect(const URL& url, const WebSocketClient::ConnnectCallback& _connectcallback, const WebSocketClient::RecvDataCallback& _datacallback, const WebSocketClient::DisconnectCallback& _disconnectcallback)
	{
		sendheader->method = "POST";
		sendheader->url = url;

		{
			std::string key = Guid::createGuid().getStringStream();

			Base::Sha1 sha1;
			sha1.input(key.c_str(), key.length());

			sendheader->headers["Host"] = url.getHost();
			sendheader->headers["Connection"] = "Upgrade";
			sendheader->headers["Pragma"] = "no-cache";
			sendheader->headers["Cache-Control"] = "no-cache";
			sendheader->headers["User-Agent"] = "public_websocket";
			sendheader->headers["Upgrade"] = "websocket";
			sendheader->headers["Origin"] = url.getProtocol() + "://" + url.getHost();
			sendheader->headers["Sec-WebSocket-Version"] = 13;
			sendheader->headers["Sec-WebSocket-Key"] = Base64::encode(sha1.report(Sha1::REPORT_BIN));
			sendheader->headers["Sec-WebSocket-Extensions"] = "permessage-deflate; client_max_window_bits";
		}

		connectcalblack = _connectcallback;
		datacallback = _datacallback;
		disconnectcallback = _disconnectcallback;

		startconnecttime = Time::getCurrentMilliSecond();

		socket = TCPClient::create(worker);
		socket->async_connect(NetAddr(url.getHostname(), url.getPort(80)), Socket::ConnectedCallback(&WebSocketClientManager::socketconnectcallback, this));

		pooltimer = make_shared<Timer>("WebSocketClientManager");
		pooltimer->start(Timer::Proc(&WebSocketClientManager::onPoolTimerProc, this), 0, 1000);

		return true;
	}

	void socketconnectcallback(const weak_ptr<Socket>& sock, bool status, const std::string& err)
	{
		connectcalblack(client, status, err);

		if (!status)
		{
			return;
		}

		socketconnectsuccess = true;

		sendcontent = make_shared<WebSocketClientSendContent>();
		recvcontent = make_shared<WebSocketClientRecvContent>(client);
		recvcontent->recvcallback = datacallback;

		//HTTPCommunication(bool _isserver,const shared_ptr<Socket> & _sock,const shared_ptr<HTTPCommunicationHandler>& _handler,const std::string& _useragent)
		commusession = make_shared<HTTPCommunication>(false,socket,shared_from_this(), useragent);
		commusession->recvHeader = make_shared<HTTPHeader>();
		commusession->recvContent = recvcontent;
		commusession->sendHeader = sendheader;
		commusession->sendContent = sendcontent;
	}

	void onPoolTimerProc(unsigned long)
	{
		uint64_t nowtime = Time::getCurrentMilliSecond();
		if (!socketconnectsuccess && nowtime > startconnecttime && nowtime - startconnecttime >= timeout)
		{
			socket->disconnect();
			dependConnectCallback(client, false, "connect timeout");
			return;
		}
		if (commusession) commusession->onPoolTimerProc();
	}

	Semaphore	connectsem;
	void dependConnectCallback(WebSocketClient*, bool status, const std::string& err)
	{
		connectsem.post();
	}


	virtual bool onRecvHeaderOk(HTTPCommunication* commu) { return true; }
	virtual void onRecvContentOk(HTTPCommunication* commu) {}
	virtual void onDisconnect(HTTPCommunication* commu)
	{
		socketconnectsuccess = false;

		disconnectcallback(client);
	}
};
struct WebSocketClient::WebSocketClientInternal
{
	shared_ptr<WebSocketClientManager> manager;
};

WebSocketClient::WebSocketClient(const shared_ptr<IOWorker>& worker, const std::string& useragent, const std::map<std::string, Value>& headers)
{
	internal = new WebSocketClientInternal;
	internal->manager = make_shared<WebSocketClientManager>();
	internal->manager->useragent = useragent;
	internal->manager->sendheader = make_shared<HTTPHeader>();
	internal->manager->sendheader->headers = headers;
	internal->manager->worker = worker;
	internal->manager->client = this;

	if (internal->manager->worker == NULL) internal->manager->worker = IOWorker::defaultWorker();
}
WebSocketClient::~WebSocketClient()
{
	disconnect();

	SAFE_DELETE(internal);
}
bool WebSocketClient::connect(const URL& url, uint32_t timout_ms, const RecvDataCallback& datacallback, const DisconnectCallback& disconnectcallback)
{
	internal->manager->timeout = timout_ms;

	startConnect(url, ConnnectCallback(&WebSocketClientManager::dependConnectCallback, internal->manager), datacallback, disconnectcallback);

	if (internal->manager->connectsem.pend(timout_ms) < 0)
	{
		internal->manager->pooltimer = NULL;
		internal->manager->socket->disconnect();

		return false;
	}

	return true;
}
bool WebSocketClient::startConnect(const URL& url, const ConnnectCallback& connectcallback, const RecvDataCallback& datacallback, const DisconnectCallback& disconnectcallback)
{
	return internal->manager->startConnect(url, connectcallback, datacallback, disconnectcallback);
}
bool WebSocketClient::disconnect()
{
	internal->manager->pooltimer = NULL;
	internal->manager->socket->disconnect();
	internal->manager->commusession = NULL;

	return true;
}
bool WebSocketClient::send(const std::string& data, WebSocketDataType type)
{
	if (!internal->manager->sendcontent) return false;
	{
		std::string protocol = internal->manager->sendcontent->buildProtocol(data, type);

		Guard locker(internal->manager->sendcontent->mutex);
		internal->manager->sendcontent->sendlist.push_back(protocol);
	}

	internal->manager->commusession->onPoolTimerProc();

	return true;
}
uint32_t WebSocketClient::sendListSize()
{
	if (!internal->manager->sendcontent) return 0;

	uint32_t cachesize = 0;

	Guard locker(internal->manager->sendcontent->mutex);
	for (std::list<std::string>::iterator iter = internal->manager->sendcontent->sendlist.begin(); iter != internal->manager->sendcontent->sendlist.end(); iter++)
	{
		cachesize += (*iter).length();
	}

	return cachesize;
}

}
}
