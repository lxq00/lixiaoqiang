#include "HTTP/WebSocket.h"
#include "WebSocketProtocol.h"
#include "boost/regex.hpp" 
#include "HTTPSession.h"

namespace Public {
namespace HTTP {

#define MAXSESSIONCACHESIZE		1024

struct WebSocketSession::WebSocketSessionInternal:public WebSocketProtocol
{
	RecvDataCallback				datacallbck;
	DisconnectCallback				disconnectcallback;
private:
	WebSocketSession*				session;
	char*							bufferAddr;
	int								bufferUsedLen;
	weak_ptr<HTTPSession_Service>	httpsession;
public:	
	WebSocketSessionInternal(WebSocketSession* _session,const shared_ptr<HTTPSession_Service>& _httpsession):WebSocketProtocol(false), session(_session),httpsession(_httpsession)
	{
		_httpsession->request->content()->setReadCallback(HTTPContent::ReadDataCallback(&WebSocketSessionInternal::httpDataRecvCallback, this));
		bufferAddr = new char[MAXSESSIONCACHESIZE + 100];
		bufferUsedLen = 0;
	}
	~WebSocketSessionInternal()
	{
		WebSocketProtocol::close();
		SAFE_DELETE(bufferAddr);
	}

	bool initProtocol()
	{
		shared_ptr<HTTPSession_Service> sessiontmp = httpsession.lock();
		if (sessiontmp == NULL) return false;


		Value key = sessiontmp->request->header("Sec-WebSocket-Key");
		Value protocol = sessiontmp->request->header("Sec-WebSocket-Protocol");

		if (key.empty())
		{
			return false;
		}

		std::string value = key.readString() + WEBSOCKETMASK;
		Base::Sha1 sha1;
		sha1.input(value.c_str(), value.length());

		std::map<std::string, Value> header;
		{
			sessiontmp->response->headers()["Access-Control-Allow-Origin"] = "*";
			sessiontmp->response->headers()[CONNECTION] = CONNECTION_Upgrade;
			sessiontmp->response->headers()[CONNECTION_Upgrade] = "websocket";
			sessiontmp->response->headers()["Sec-WebSocket-Accept"] = Base64::encode(sha1.report(Sha1::REPORT_BIN));
			if (!protocol.empty())
			{
				sessiontmp->response->headers()["Sec-WebSocket-Protocol"] = protocol;
			}
		}

		sessiontmp->response->statusCode() = 101;
		sessiontmp->response->errorMessage() = "Switching Protocols";

		sessiontmp->onPoolTimerProc();

		return true;
	}

	void httpDataRecvCallback(const char* buffer, uint32_t len)
	{
		while (len > 0)
		{
			const char* buffertmp = buffer;
			uint32_t datalen = len;
	
			if (bufferUsedLen != 0)
			{
				uint32_t needlen = min(MAXSESSIONCACHESIZE - bufferUsedLen, datalen);
				memcpy(bufferAddr + bufferUsedLen, buffertmp, needlen);
				bufferUsedLen += needlen;

				buffertmp = bufferAddr;
				datalen = bufferUsedLen;

				buffer += needlen;
				len -= needlen;
			}

			const char* usedtmp = parseProtocol(buffertmp, datalen);
			if (usedtmp == NULL) break;

			int haveparselen = usedtmp - buffertmp;

			if (buffertmp == buffer)
			{
				buffer += haveparselen;
				len -= haveparselen;
			}
			else
			{
				int bufferleavlen = bufferUsedLen - haveparselen;
				if (bufferUsedLen > 0)
				{
					memmove(bufferAddr, bufferAddr + haveparselen, bufferleavlen);
				}
				bufferUsedLen = bufferleavlen;
			}
		}
	}
	void httpDisconnectCallback(HTTPRequest*, const std::string&)
	{
		disconnectcallback(session);
	}
	void dataCallback(const std::string& data, WebSocketDataType type)
	{
		datacallbck(session, data,type);
	}
};
//web socket session
WebSocketSession::WebSocketSession(const shared_ptr<HTTPSession_Service>& session)
{
	internal = new WebSocketSessionInternal(this,session);
}
WebSocketSession::~WebSocketSession()
{
	SAFE_DELETE(internal);
}
bool WebSocketSession::initProtocol()
{
	return internal->initProtocol();
}
void WebSocketSession::start(const RecvDataCallback& datacallback, const DisconnectCallback& disconnectcallback)
{
	internal->datacallbck = datacallback;
	internal->disconnectcallback = disconnectcallback;
}
void WebSocketSession::stop()
{

}
bool WebSocketSession::connected() const
{
	shared_ptr<Socket> tmp = internal->getsocket();
	if (tmp == NULL) return false;

	return tmp->getStatus() == NetStatus_connected;
}
	
bool WebSocketSession::send(const std::string& data, WebSocketDataType type)
{
	shared_ptr<Socket> tmp = internal->getsocket();
	if (tmp == NULL || !internal->ready()) return false;

	return internal->buildAndSend(data, type);
}

const std::map<std::string, Value>& WebSocketSession::headers() const
{
	return internal->headers;
}
Value WebSocketSession::header(const std::string& key) const
{
	std::map<std::string, Value>::const_iterator iter = internal->headers.find(key);
	if (iter == internal->headers.end())
	{
		return Value();
	}

	return iter->second;
}
const URL& WebSocketSession::url() const
{
	return internal->url;
}
NetAddr WebSocketSession::remoteAddr() const
{
	shared_ptr<Socket> tmp = internal->getsocket();
	if (tmp == NULL) return NetAddr();

	return tmp->getOtherAddr();
}

uint32_t WebSocketSession::sendListSize()
{
    return internal->sendListSize();
}

}
}
