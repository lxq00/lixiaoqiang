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
	WebSocketSession*				session;
	char*							bufferAddr;
	int								bufferUsedLen;

	
	WebSocketSessionInternal(const shared_ptr<HTTPSession_Service>& session):WebSocketProtocol(false)
	{
		session->request->content()->setReadCallback(HTTPContent::ReadDataCallback(&WebSocketSessionInternal::httpDataRecvCallback, this));
		bufferAddr = new char[MAXSESSIONCACHESIZE + 100];
		bufferUsedLen = 0;
	}
	~WebSocketSessionInternal()
	{
		WebSocketProtocol::close();
		SAFE_DELETE(bufferAddr);
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

	void socketDisConnectcallback(const weak_ptr<Socket>& /*connectSock*/, const std::string&)
	{
		disconnectcallback(session);
		errcallback(session);
	}
	void dataCallback(const std::string& data, WebSocketDataType type)
	{
		datacallbck(session, data,type);
	}
	void headerParseReady() 
	{
		bool ret = readycallback(session);
		sendResponseHeader(ret);
	}
	void sendResponseHeader(bool result)
	{
		std::string mask = "258EAFA5-E914-47DA-95CA-C5AB0DC85B11";

		std::map<std::string, Value>::iterator iter = headers.find("Sec-WebSocket-Key");
		if (iter == headers.end())
		{
			errcallback(session);
			return;
		}

        bool findProtocol = false;
        std::string SecWebSocketProtocol;
        std::map<std::string, Value>::iterator fiter = headers.find("Sec-WebSocket-Protocol");
        if (fiter != headers.end())
        {
            SecWebSocketProtocol = fiter->second.readString();
            findProtocol = true;
        }

		std::string value = iter->second.readString() + mask;
		Base::Sha1 sha1;
		sha1.input(value.c_str(), value.length());

		std::map<std::string, Value> header;
		{
			header["Access-Control-Allow-Origin"] = "*";
			header["Connection"] = "Upgrade";
			header["Upgrade"] = "websocket";
			header["Sec-WebSocket-Accept"] = Base64::encode(sha1.report(Sha1::REPORT_BIN));
            if(findProtocol)
            {
               header["Sec-WebSocket-Protocol"] = SecWebSocketProtocol;
            }
		}

		std::string reqeuststr;
		if(result) reqeuststr = std::string("HTTP/1.1 101 Switching Protocols") + HTTPHREADERLINEEND;
		else  reqeuststr = std::string("HTTP/1.1 404 NOT FOUND") + HTTPHREADERLINEEND;

		for (std::map<std::string, Value>::iterator iter = header.begin(); iter != header.end(); iter++)
		{
			reqeuststr += iter->first + ": " + iter->second.readString() + HTTPHREADERLINEEND;
		}
		reqeuststr += HTTPHREADERLINEEND;

		addDataAndCheckSend(reqeuststr.c_str(), reqeuststr.length());

		if (!result)
		{
			//seel 10 ms ,wait async send success
			Thread::sleep(10);

			errcallback(session);
		}
	}
};
//web socket session
WebSocketSession::WebSocketSession(const shared_ptr<Socket>& sock, const ReadyCallback& readycallback, const ErrorCallback& errcallback)
{
	internal = new WebSocketSessionInternal();
	internal->session = this;
	internal->readycallback = readycallback;
	internal->errcallback = errcallback;

	internal->setSocketAndStart(sock);
}
WebSocketSession::~WebSocketSession()
{
	SAFE_DELETE(internal);
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

struct WebSocketServer::WebSocketServerInternal
{
	shared_ptr<IOWorker> worker;
	shared_ptr<Socket>  sock;
	
	shared_ptr<Timer>	poolTimer;

	Mutex				mutex;
	std::map<std::string, AcceptCallback>						acceptCallbackList;
	std::map< WebSocketSession*, shared_ptr< WebSocketSession> > waitList;
	std::set< shared_ptr< WebSocketSession> > errlist;

	WebSocketServerInternal()
	{
		poolTimer = make_shared<Timer>("WebSocketServerInternal");
		poolTimer->start(Timer::Proc(&WebSocketServerInternal::onPoolTimerProc, this), 0, 1000);
	}
	~WebSocketServerInternal()
	{
		poolTimer = NULL;
		sock = NULL;
	}
	void socktAcceptCallback(const weak_ptr<Socket>& /*oldSock*/, const shared_ptr<Socket>& newSock)
	{
		Guard locker(mutex);

		shared_ptr<WebSocketSession> session(new WebSocketSession(newSock,
			WebSocketSession::ReadyCallback(&WebSocketServerInternal::readyCallback, this),
			WebSocketSession::ErrorCallback(&WebSocketServerInternal::errCallback, this)));

		waitList[session.get()] = session;
	}
	bool readyCallback(WebSocketSession* session)
	{
		shared_ptr< WebSocketSession> sessionobj;
		AcceptCallback acceptcallback;

		{
			Guard locker(mutex);
			std::map< WebSocketSession*, shared_ptr< WebSocketSession> >::iterator iter = waitList.find(session);
			if (iter == waitList.end()) return false;

			sessionobj = iter->second;
			waitList.erase(iter);

			std::string flag = String::tolower(sessionobj->url().getPathName());
			for (std::map<std::string, AcceptCallback>::iterator iter = acceptCallbackList.begin(); iter != acceptCallbackList.end(); iter++)
			{
				boost::regex oRegex(iter->first);
				if (!boost::regex_match(flag, oRegex))
				{
					continue;
				}

				acceptcallback = iter->second;
				break;
			}
			
			if (acceptcallback == NULL)
			{
				errlist.insert(sessionobj);
			}
		}
		bool ret = acceptcallback(sessionobj);
		if (!ret)
		{
			errlist.insert(sessionobj);
		}
		
		return acceptcallback != NULL;
	}

	void errCallback(WebSocketSession* session)
	{
		Guard locker(mutex);
		std::map< WebSocketSession*, shared_ptr< WebSocketSession> >::iterator iter = waitList.find(session);
		if (iter == waitList.end()) return;

		errlist.insert(iter->second);
		waitList.erase(iter);
	}

	void onPoolTimerProc(unsigned long)
	{
		std::set<shared_ptr< WebSocketSession> > freelist;
		{
			Guard locker(mutex);
			freelist = errlist;
			errlist.clear();
		}
	}
};

WebSocketServer::WebSocketServer(const shared_ptr<IOWorker>& worker)
{
	internal = new WebSocketServerInternal();
	internal->worker = worker;
}
WebSocketServer::~WebSocketServer()
{
	stopAccept();
	SAFE_DELETE(internal);
}
bool WebSocketServer::listen(const std::string& path, const AcceptCallback& callback)
{
	std::string flag = String::tolower(path);

	Guard locker(internal->mutex);
	if (callback == NULL)
	{
		internal->acceptCallbackList.erase(flag);
	}
	else
	{
		internal->acceptCallbackList[flag] = callback;
	}

	return true;
}
bool WebSocketServer::startAccept(uint32_t port)
{
	internal->sock = TCPServer::create(internal->worker, port);
    if (internal->sock == shared_ptr<Socket>())
    {
        return false;
    }
	internal->sock->async_accept(Socket::AcceptedCallback(&WebSocketServerInternal::socktAcceptCallback, internal));

	return true;
}
bool WebSocketServer::stopAccept()
{
	internal->sock = NULL;

	return true;
}

struct WebSocketClient::WebSocketClientInternal :public WebSocketProtocol
{
	std::map<std::string, Value>	extheaders;
	shared_ptr<IOWorker>			worker;
	shared_ptr<Socket>				sock;

	RecvDataCallback				datacallback;
	DisconnectCallback				disconnectcallback;
	ConnnectCallback				connectcallback;

	WebSocketClient*				client;

	WebSocketClientInternal():WebSocketProtocol(true){}
	~WebSocketClientInternal()
	{
		if (sock != NULL) sock->disconnect();
		sock = NULL;
		WebSocketProtocol::close();
	}

	void socketConnectCallback(const weak_ptr<Socket>& /*sock*/)
	{
		setSocketAndStart(sock);
		sendRequest();
	}
	void headerParseReady()
	{
		connectcallback(client);
	}
	void socketDisConnectcallback(const weak_ptr<Socket>& /*connectSock*/, const std::string&)
	{
		disconnectcallback(client);
	}
	void dataCallback(const std::string& data, WebSocketDataType type)
	{
		datacallback(client,data,type);
	}
	void sendRequest()
	{
		std::string key = Guid::createGuid().getStringStream();

		Base::Sha1 sha1;
		sha1.input(key.c_str(), key.length());


		std::map<std::string, Value> header;
		{
			header["Host"] = url.getHost();
			header["Connection"] = "Upgrade";
			header["Pragma"] = "no-cache";
			header["Cache-Control"] = "no-cache";
			header["User-Agent"] = "public_websocket";
			header["Upgrade"] = "websocket";
			header["Origin"] = url.getProtocol() + "://" + url.getHost();
			header["Sec-WebSocket-Version"] = 13;
			header["Sec-WebSocket-Key"] = Base64::encode(sha1.report(Sha1::REPORT_BIN));
			header["Sec-WebSocket-Extensions"] = "permessage-deflate; client_max_window_bits";

			for (std::map<std::string, Value>::iterator iter = extheaders.begin(); iter != extheaders.end(); iter++)
			{
				header[iter->first] = iter->second;
			}
		}
		
		std::string requrl = url.getPath() == "" ? "/" : url.getPath();
		std::string reqeuststr = std::string() +
			"GET " + requrl +" "+HTTPVERSION + HTTPHREADERLINEEND;
		for (std::map<std::string, Value>::iterator iter = header.begin(); iter != header.end(); iter++)
		{
			reqeuststr += iter->first + ": " + iter->second.readString() + HTTPHREADERLINEEND;
		}
		reqeuststr += HTTPHREADERLINEEND;


		addDataAndCheckSend(reqeuststr.c_str(), reqeuststr.length());
	}
};

struct waitConnectItem
{
	Base::Semaphore waitsem;
	void connectCallback(WebSocketClient*)
	{
		waitsem.post();
	}
};

WebSocketClient::WebSocketClient(const shared_ptr<IOWorker>& worker, const std::map<std::string, Value>&headers)
{
	internal = new WebSocketClientInternal();
	internal->extheaders = headers;
	internal->client = this;
	internal->worker = worker;
	internal->sock = TCPClient::create(worker);
}
WebSocketClient::~WebSocketClient()
{
	disconnect();
	
	SAFE_DELETE(internal);
}

bool WebSocketClient::connect(const URL& url, uint32_t timout_ms, const RecvDataCallback& datacallback, const DisconnectCallback& disconnectcallback)
{
	shared_ptr< waitConnectItem> waiting = make_shared<waitConnectItem>();

	startConnect(url, WebSocketClient::ConnnectCallback(&waitConnectItem::connectCallback, waiting), datacallback, disconnectcallback);

	waiting->waitsem.pend(timout_ms);
	internal->connectcallback = WebSocketClient::ConnnectCallback();

	return internal->errorCode <= 200;
}
bool WebSocketClient::startConnect(const URL& url, const ConnnectCallback& connectcallback, const RecvDataCallback& datacallback, const DisconnectCallback& disconnectcallback)
{
	internal->url = url;
	internal->connectcallback = connectcallback;
	internal->datacallback = datacallback;
	internal->disconnectcallback = disconnectcallback;

	internal->sock->async_connect(NetAddr(url.getHostname(), url.getPort(80)), Socket::ConnectedCallback(&WebSocketClientInternal::socketConnectCallback, internal));
	

	return true;
}
bool WebSocketClient::disconnect()
{
	internal->sock = NULL;

	return true;
}
bool WebSocketClient::send(const std::string& data, WebSocketDataType type)
{
	shared_ptr<Socket> tmp = internal->sock;
	if (tmp == NULL || !internal->ready()) return false;

	return internal->buildAndSend(data, type);
}

}
}
