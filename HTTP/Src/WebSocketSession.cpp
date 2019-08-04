#include "HTTP/HTTPServer.h"
#include "WebSocketProtocol.h"
#include "HTTPCommunication.h"

namespace Public {
namespace HTTP {

class WebSocketRecvContent :public IContent,public WebSocketProtocol
{
public:
	WebSocketServerSession*						session;

	WebSocketServerSession::RecvDataCallback	recvcallback;
	
	WebSocketRecvContent(WebSocketServerSession* _session):WebSocketProtocol(false,WebSocketProtocol::ParseDataCallback(&WebSocketRecvContent::parseDataCallback,this)), session(session)
	{}
	~WebSocketRecvContent() {}

	void parseDataCallback(const std::string& data, WebSocketDataType type)
	{
		recvcallback(session,data, type);
	}

	uint32_t append(const char* buffer, uint32_t len)
	{
		const char* usedaddr = parseProtocol(buffer, len);

		return usedaddr - buffer;
	}
	void read(String& data) {}
};

class WebSocketSendContent :public IContent, public WebSocketProtocol
{
public:
	WebSocketSendContent() :WebSocketProtocol(false, NULL){}
	~WebSocketSendContent() {}

	Mutex							mutex;
	std::list<std::string>			sendlist;

	uint32_t append(const char* buffer, uint32_t len) { return len; }
	void read(String& data) 
	{
		Guard locker(mutex);

		if (sendlist.size()) return;

		data = sendlist.front();
		sendlist.pop_front();
	}
};

struct WebSocketServerSession::WebSocketServerSessionInternal
{
	shared_ptr<HTTPCommunication>	commusession;

	DisconnectCallback				disconnectcallback;
	

	shared_ptr<WebSocketRecvContent>	recvcontent;

	shared_ptr<HTTPHeader>				sendheader;
	shared_ptr<WebSocketSendContent>	sendcontent;
};
WebSocketServerSession::WebSocketServerSession(const shared_ptr<HTTPCommunication>& commuSession)
{
	internal = new WebSocketServerSessionInternal;
	internal->commusession = commuSession;
}
WebSocketServerSession::~WebSocketServerSession()
{
	stop();

	SAFE_DELETE(internal);
}

bool WebSocketServerSession::checkWebsocketHeader(const shared_ptr<HTTPHeader>& header)
{
	if (strcasecmp(header->method.c_str(), "POST") != 0) return false;

	Value connectionval = header->header(CONNECTION);
	if (connectionval.empty() || strcasecmp(connectionval.readString().c_str(), CONNECTION_Upgrade) != 0) return false;

	Value upgradval = header->header(CONNECTION_Upgrade);
	if (upgradval.empty() || strcasecmp(upgradval.readString().c_str(), "websocket") != 0) return false;

	Value keyval = header->header("Sec-WebSocket-Key");

	if (keyval.empty()) return false;

	return true;
}

void WebSocketServerSession::start(const RecvDataCallback& datacallback, const DisconnectCallback& disconnectcallback)
{
	internal->disconnectcallback = disconnectcallback;

	internal->recvcontent = make_shared<WebSocketRecvContent>(this);
	internal->recvcontent->recvcallback = datacallback;

	internal->sendcontent = make_shared<WebSocketSendContent>();
	internal->sendheader = make_shared<HTTPHeader>();
	{
		Value key = internal->commusession->recvHeader->header("Sec-WebSocket-Key");
		Value protocol = internal->commusession->recvHeader->header("Sec-WebSocket-Protocol");

		std::string value = key.readString() + WEBSOCKETMASK;
		Base::Sha1 sha1;
		sha1.input(value.c_str(), value.length());

		std::map<std::string, Value> header;
		{
			internal->sendheader->headers["Access-Control-Allow-Origin"] = "*";
			internal->sendheader->headers[CONNECTION] = CONNECTION_Upgrade;
			internal->sendheader->headers[CONNECTION_Upgrade] = "websocket";
			internal->sendheader->headers["Sec-WebSocket-Accept"] = Base64::encode(sha1.report(Sha1::REPORT_BIN));
			if (!protocol.empty())
			{
				internal->sendheader->headers["Sec-WebSocket-Protocol"] = protocol;
			}
		}

		internal->sendheader->statuscode = 101;
		internal->sendheader->statusmsg = "Switching Protocols";
	}

	internal->commusession->recvContent = internal->recvcontent;
	internal->commusession->startRecv();
	internal->commusession->setSendHeaderContentAndPostSend(internal->sendheader, internal->sendcontent);
}
void WebSocketServerSession::stop()
{
	if (internal->recvcontent) internal->recvcontent->recvcallback = NULL;
}

bool WebSocketServerSession::send(const std::string& data, WebSocketDataType type)
{
	if (!internal->sendcontent) return false;
	{
		std::string protocol = internal->sendcontent->buildProtocol(data, type);

		Guard locker(internal->sendcontent->mutex);
		internal->sendcontent->sendlist.push_back(protocol);
	}
	
	internal->commusession->onPoolTimerProc();

	return true;
}

const std::map<std::string, Value>& WebSocketServerSession::headers() const
{
	return internal->commusession->recvHeader->headers;
}
Value WebSocketServerSession::header(const std::string& key) const
{
	return internal->commusession->recvHeader->header(key);
}
const URL& WebSocketServerSession::url() const
{
	return internal->commusession->recvHeader->url;
}
NetAddr WebSocketServerSession::remoteAddr() const
{
	return internal->commusession->socket->getOtherAddr();
}

uint32_t WebSocketServerSession::sendListSize()
{
	if (!internal->sendcontent) return 0;

	uint32_t cachesize = 0;

	Guard locker(internal->sendcontent->mutex);
	for (std::list<std::string>::iterator iter = internal->sendcontent->sendlist.begin(); iter != internal->sendcontent->sendlist.end(); iter++)
	{
		cachesize += (*iter).length();
	}

	return cachesize;
}

void WebSocketServerSession::disconnected()
{
	internal->disconnectcallback(this);
}

}
}
