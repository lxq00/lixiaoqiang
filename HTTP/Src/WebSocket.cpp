#include "boost/regex.hpp"
#include "boost/shared_ptr.hpp"
#include "boost/make_shared.hpp"
#include <boost/asio.hpp>
#include <websocketpp/server.hpp>
#include <websocketpp/client.hpp>
#include <websocketpp/endpoint.hpp>
#include <websocketpp/config/asio_no_tls.hpp>
#include <websocketpp/config/asio_no_tls_client.hpp>
#include "HTTP/WebSocket.h"
#include "HTTPParser.h"

namespace Public {
namespace HTTP {

using websocketpp::connection_hdl;
typedef websocketpp::server<websocketpp::config::asio> service;
using std::placeholders::_1;
using std::placeholders::_2;
using websocketpp::lib::bind;
using namespace std;

struct WebSocketCallbackAgent
{
	typedef Function2<void, connection_hdl&, service::message_ptr&> WebSocketMessageCallback;
	typedef Function1<void, connection_hdl&> WebSocketCallback;
	uint32_t threadid;

	WebSocketCallbackAgent()
	{

	}

	virtual ~WebSocketCallbackAgent()
	{

	}

	void on_open(connection_hdl hdl)
	{
		GuardReadMutex locker(mutex);
		threadid = Thread::getCurrentThreadID();
		if (onopen)
		{
			onopen(hdl);
		}
	}

	void on_message(connection_hdl hdl, service::message_ptr msg)
	{
		GuardReadMutex locker(mutex);
		threadid = Thread::getCurrentThreadID();
		if (onmessage)
		{
			onmessage(hdl, msg);
		}
	}

	void on_close(connection_hdl hdl)
	{
		GuardReadMutex locker(mutex);
		threadid = Thread::getCurrentThreadID();
		if (onclose)
		{
			onclose(hdl);
		}
	}

	void on_fail(connection_hdl hdl)
	{
		GuardReadMutex locker(mutex);
		threadid = Thread::getCurrentThreadID();
		if (onfail)
		{
			onfail(hdl);
		}
	}

	void on_pong(connection_hdl hdl)
	{
		GuardReadMutex locker(mutex);
		threadid = Thread::getCurrentThreadID();
		if (onpong)
		{
			onpong(hdl);
		}
	}

	void clear()
	{
		onopen = NULL;
		onmessage = NULL;
		onclose = NULL;
		onfail = NULL;
		onpong = NULL;
		if (Thread::getCurrentThreadID() != threadid)
		{
			GuardWriteMutex locker(mutex);
		}
	}

	ReadWriteMutex mutex;
	WebSocketCallback onopen;
	WebSocketMessageCallback onmessage;
	WebSocketCallback onclose;
	WebSocketCallback onfail;
	WebSocketCallback onpong;

};

struct WebSocketSession::WebSocketSessionInternal
{
	RecvDataCallback				datacallbck;
	DisconnectCallback				disconnectcallback;
	Mutex							callbackmutex;

	WebSocketSession*				session;
	Mutex							mutex;
	service::connection_ptr			conn;
	URL								url;
	std::map<std::string, Value>	_headers;
	bool							isconnted;
	uint32_t						max_buffer_size;
	uint32_t						nBufferOverFrames;
	shared_ptr<WebSocketCallbackAgent> callback;

	//unsigned char*					buffer;
	//unsigned int					bufsize;
	//unsigned int					buffoffset;
	
	WebSocketSessionInternal(const service::connection_ptr & con, const std::string &uri)
		: isconnted(true)
//		, buffoffset(0)
		, conn(con)
		, url(uri)
		, max_buffer_size(1024 * 1024 * 4)
		, nBufferOverFrames(0)
		, callback(make_shared<WebSocketCallbackAgent>())
	{
		callback->onmessage = WebSocketCallbackAgent::WebSocketMessageCallback(&WebSocketSessionInternal::on_message, this);
		callback->onclose = WebSocketCallbackAgent::WebSocketCallback(&WebSocketSessionInternal::on_close, this);
		callback->onfail = WebSocketCallbackAgent::WebSocketCallback(&WebSocketSessionInternal::on_fail, this);
		callback->onpong = WebSocketCallbackAgent::WebSocketCallback(&WebSocketSessionInternal::on_pong_timeout, this);

		conn->set_message_handler(bind(&WebSocketCallbackAgent::on_message, callback, _1, _2));
		conn->set_close_handler(bind(&WebSocketCallbackAgent::on_close, callback, _1));
		conn->set_fail_handler(bind(&WebSocketCallbackAgent::on_fail, callback, _1));
		conn->set_pong_timeout_handler(bind(&WebSocketCallbackAgent::on_pong, callback, _1));
		getHeader();
		//bufsize = 1024 * 1024;
		//buffer = new unsigned char[bufsize];
	}

	~WebSocketSessionInternal()
	{
		close();
		callback->clear();
		//if (buffer != NULL)
		//{
		//	delete[] buffer;
		//	buffer = NULL;
		//}
	}

	void getHeader()
	{
		std::string host = conn->get_host();
		uint32_t port = conn->get_port();
		char szPort[64] = { 0 };
		sprintf(szPort, "%s:%d", host.c_str(), port);
		_headers.insert(std::make_pair("Host", szPort));
	}

	const std::map<std::string, Value>& headers()
	{
		return _headers;
	}

	Value header(const std::string& key)
	{
		std::map<std::string, Value>::iterator it = _headers.find(key);
		if (it != _headers.end())
		{
			return it->second;
		}
		logerror("not find websocket header key:%s", key.c_str());
		return Value();
	}

	bool send(const std::string& data, WebSocketDataType type)
	{
		service::connection_ptr tmp = conn;
		if (tmp == NULL || !isconnted)
		{
			return false;
		}

		if (tmp->get_send_frame_size() >= 500 || tmp->get_buffered_amount() >= max_buffer_size)
		{
			if (++nBufferOverFrames % 2000 == 0)
			{
				NetAddr addr = remoteAddr();
				logerror("remote address:%s:%d session buffer is over!\r\n", addr.getIP().c_str(), addr.getPort());
			}
			return false;
		}

		websocketpp::frame::opcode::value datatype = type == WebSocketDataType_Txt ? websocketpp::frame::opcode::text : websocketpp::frame::opcode::binary;
		error_code err = tmp->send(data.c_str(), data.size(), datatype);
		if (err)
		{
			NetAddr addr = remoteAddr();
			logerror("remote address:%s:%d send fail! errmsg:%s", addr.getIP().c_str(), addr.getPort(), err.message().c_str());
			return false;
		}
		return true;
	}

	void on_message(connection_hdl &hdl, service::message_ptr &msg)
	{
		if (!msg->get_fin())
		{
			return;
		}

		const std::string &data = msg->get_payload();
		if (datacallbck != NULL)
		{
			WebSocketDataType dataType = msg->get_opcode() == websocketpp::frame::opcode::value::binary ? WebSocketDataType_Bin : WebSocketDataType_Txt;
			datacallbck(session, data, dataType);
		}
	}

	void on_close(connection_hdl &hdl)
	{
		isconnted = false;
		if (disconnectcallback == NULL)
		{
			logerror("Œ¥◊¢≤·µÙœﬂªÿµ˜");
			return;
		}
		disconnectcallback(session);
	}

	void on_fail(connection_hdl &hdl)
	{
		isconnted = false;
		if (disconnectcallback != NULL)
		{
			disconnectcallback(session);
		}
	}

	void on_pong_timeout(connection_hdl &hdl)
	{
		isconnted = false;
		if (disconnectcallback != NULL)
		{
			disconnectcallback(session);
		}
	}

	void close()
	{
		if (conn != NULL)
		{
			error_code ec;
			conn->close(websocketpp::close::status::no_status, "", ec);
			conn = NULL;
		}
	}

	void disConnectcallback(const shared_ptr<WebSocketSession>& session)
	{
		Guard locker(callbackmutex);
		if (disconnectcallback == NULL)
		{
			return;
		}
		disconnectcallback(&*session);
	}
	void dataCallback(const std::string& data, WebSocketDataType type)
	{
		Guard locker(callbackmutex);
		if (datacallbck == NULL)
		{
			return;
		}
		datacallbck(session, data, type);
	}

	uint32_t sendListSize()
	{
		service::connection_ptr tmp = conn;
		if (tmp == NULL)
		{
			return -1;
		}
		return (uint32_t)tmp->get_send_frame_size();
	}

	NetAddr remoteAddr()
	{
		service::connection_ptr tmp = conn;
		if (tmp == NULL)
		{
			return NetAddr();
		}
		std::string ip = tmp->get_socket().remote_endpoint().address().to_string();
		std::vector<std::string> hostVec = String::split(ip, ":");
		if (hostVec.size() > 1)
		{
			ip = *hostVec.rbegin();
		}
		int port = tmp->get_socket().remote_endpoint().port();
		return NetAddr(ip, port);
	}
};
//web socket session
WebSocketSession::WebSocketSession(void* con, const std::string& uri)
{
	internal = new WebSocketSessionInternal(*(service::connection_ptr*)con, uri);
	internal->session = this;
}
WebSocketSession::~WebSocketSession()
{
	SAFE_DELETE(internal);
}

void WebSocketSession::start(const RecvDataCallback& datacallback, const DisconnectCallback& disconnectcallback)
{
	Guard locker(internal->callbackmutex);
	internal->datacallbck = datacallback;
	internal->disconnectcallback = disconnectcallback;
}

void WebSocketSession::stop()
{
	{
		Guard locker(internal->callbackmutex);
		internal->datacallbck = NULL;
		internal->disconnectcallback = NULL;
	}
	internal->close();
}

bool WebSocketSession::connected() const
{
	return internal->isconnted;
}
	
bool WebSocketSession::send(const std::string& data, WebSocketDataType type)
{
	return internal->send(data, type);
}


const std::map<std::string, Value>& WebSocketSession::headers() const
{
	return internal->headers();
}
Value WebSocketSession::header(const std::string& key) const
{
	return internal->header(key);
}
const URL& WebSocketSession::url() const
{
	return internal->url;
}

NetAddr WebSocketSession::remoteAddr() const
{
	return internal->remoteAddr();
}

uint32_t WebSocketSession::sendListSize()
{
	return internal->sendListSize();
}

WebSocketSession::WebSocketSessionInternal* WebSocketSession::getinternal()
{
	return internal;
}

struct WebSocketServer::WebSocketServerInternal
{
	shared_ptr<IOWorker> worker;
	service server;
	std::map<std::string, AcceptCallback> acceptCallbackList;
	Mutex mutex;


	WebSocketServerInternal()
	{

	}

	~WebSocketServerInternal()
	{
		stopAccept();
	}

	bool listen(const std::string& path, const AcceptCallback& callback)
	{
		std::string flag = String::tolower(path);

		Guard locker(mutex);
		if (callback == NULL)
		{
			acceptCallbackList.erase(flag);
		}
		else
		{
			acceptCallbackList[flag] = callback;
		}
		return true;
	}

	bool startAccept(uint32_t port)
	{
		server.init_asio(((boost::shared_ptr<boost::asio::io_service>*)worker->getBoostASIOIOServerSharedptr())->get());
		server.set_access_channels(websocketpp::log::alevel::none);
//		server.clear_access_channels(websocketpp::log::alevel::frame_payload);
		server.set_open_handler(bind(&WebSocketServerInternal::on_open, this, _1));
		server.set_fail_handler(bind(&WebSocketServerInternal::on_fail, this, _1));
		server.listen(port);
		server.start_accept();
		return true;
	}

	bool stopAccept()
	{
		error_code err;
		server.stop_listening(err);
		if (err)
		{
			logerror("stopLlinstening fail! errmsg:%s\r\n", err.message().c_str());
			return false;
		}
		return true;
	}

	AcceptCallback getcallback(const std::string& url)
	{
		Guard locker(mutex);
		for (std::map<std::string, AcceptCallback>::iterator it = acceptCallbackList.begin(); it != acceptCallbackList.end(); it++)
		{
			boost::regex oRegex(it->first);
			if (boost::regex_match(url, oRegex))
			{
				return it->second;
			}
		}
		return AcceptCallback();
	}

	void on_open(connection_hdl hdl)
	{
		service::connection_ptr con = server.get_con_from_hdl(hdl);
		std::string uri = con->get_uri()->get_resource();
		AcceptCallback callback = getcallback(uri);
		if (callback == NULL)
		{
			logerror("∑√Œ ¥ÌŒÛ uri:%s", uri.c_str());
			return;
		}
		shared_ptr<WebSocketSession> session = make_shared<WebSocketSession>(&con, uri);
		callback(session);
	}

	void on_fail(websocketpp::connection_hdl hdl)
	{
		logerror("on fail!");
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
	return internal->listen(path, callback);
}
bool WebSocketServer::startAccept(uint32_t port)
{
	return internal->startAccept(port);
}
bool WebSocketServer::stopAccept()
{
	return internal->stopAccept();
}



struct WebSocketClient::WebSocketClientInternal
{
	typedef websocketpp::client<websocketpp::config::asio_client> wsclient;
	typedef websocketpp::config::asio_client::message_type::ptr message_ptr;

	ConnnectCallback _connectcallback;
	RecvDataCallback _datacallback;
	DisconnectCallback _disconnectcallback;
	WebSocketClient* websocketclient;
	//unsigned char*					buffer;
	//unsigned int					bufsize;
	//unsigned int					bufoffset;
//	bool							isconnect;
	boost::shared_ptr<wsclient>		client;
	shared_ptr<IOWorker>			ioworker;
	wsclient::connection_ptr		conn;
	uint32_t						max_buffer_size;
	Mutex							mutex;
	bool							isconnected;
	uint32_t						nBufferOverFrames;
	shared_ptr<WebSocketCallbackAgent> callback;

	WebSocketClientInternal(const shared_ptr<IOWorker> &worker)
		: ioworker(worker)
		, max_buffer_size(1024 * 1024 * 4)
		, isconnected(false)
		, nBufferOverFrames(0)
		, callback(make_shared<WebSocketCallbackAgent>())
	{
		callback->onopen = WebSocketCallbackAgent::WebSocketCallback(&WebSocketClientInternal::on_open, this);
		callback->onclose = WebSocketCallbackAgent::WebSocketCallback(&WebSocketClientInternal::on_close, this);
		callback->onfail = WebSocketCallbackAgent::WebSocketCallback(&WebSocketClientInternal::on_fail, this);
		callback->onmessage = WebSocketCallbackAgent::WebSocketMessageCallback(&WebSocketClientInternal::on_message, this);
		callback->onpong = WebSocketCallbackAgent::WebSocketCallback(&WebSocketClientInternal::on_pong_timeout, this);
	}

	virtual ~WebSocketClientInternal()
	{
		disconnect();
		callback->clear();
	}

	bool startConnect(const URL& url, const ConnnectCallback& connectcallback, const RecvDataCallback& datacallback, const DisconnectCallback& disconnectcallback)
	{
		Guard locker(mutex);
		if (client != NULL)
		{
			return false;
		}

		_connectcallback = connectcallback;
		_datacallback = datacallback;
		_disconnectcallback = disconnectcallback;
		client = boost::make_shared<wsclient>();
		client->set_access_channels(websocketpp::log::alevel::none);
		client->set_error_channels(websocketpp::log::elevel::none);
		client->init_asio(((boost::shared_ptr<boost::asio::io_service>*)ioworker->getBoostASIOIOServerSharedptr())->get());
		//client->set_fail_handler(bind(&WebSocketClientInternal::on_fail, this, _1));

		websocketpp::lib::error_code ec;
		conn = client->get_connection(url.href(), ec);
		if (ec)
		{
			logerror("get connection fail! errmsg:%s", ec.message().c_str());
			return false;
		}
		conn->set_open_handler(bind(&WebSocketCallbackAgent::on_open, callback, _1));
		conn->set_message_handler(bind(&WebSocketCallbackAgent::on_message, callback, _1, _2));
		conn->set_close_handler(bind(&WebSocketCallbackAgent::on_close, callback, _1));
		conn->set_fail_handler(bind(&WebSocketCallbackAgent::on_fail, callback, _1));
		conn->set_pong_timeout_handler(bind(&WebSocketCallbackAgent::on_pong, callback, _1));
		conn->set_open_handshake_timeout(3000);
		conn->set_pong_timeout(3000);
		client->connect(conn);
		return true;
	}

	bool disconnect()
	{
		Guard locker(mutex);
		if (conn != NULL)
		{
			error_code ec;
			conn->close(websocketpp::close::status::no_status, "", ec);
			conn = NULL;
			client = NULL;
		}
		_connectcallback = NULL;
		_datacallback = NULL;
		_disconnectcallback = NULL;
		return true;
	}

	uint32_t sendListSize()
	{
		wsclient::connection_ptr tmp = conn;
		if (tmp == NULL)
		{
			return 0;
		}
		return (uint32_t)tmp->get_send_frame_size();
	}

	bool send(const std::string &data, WebSocketDataType type)
	{
		wsclient::connection_ptr tmp = conn;
		if (tmp == NULL || !isconnected)
		{
			return false;
		}

		if (tmp->get_send_frame_size() >= 500 || tmp->get_buffered_amount() >= max_buffer_size)
		{
			if (++nBufferOverFrames % 2000 == 0)
			{
				logerror("buffer is over! frame:%d\r\n", nBufferOverFrames);
			}
			return false;
		}

		websocketpp::frame::opcode::value opcode = type == WebSocketDataType_Txt ? websocketpp::frame::opcode::text : websocketpp::frame::opcode::binary;
		error_code err = tmp->send(data.c_str(), data.size(), opcode);
		if (err)
		{
//			logerror("client send fail!errmsg:%s", err.message().c_str());
			return false;
		}
		return true;
	}

	void on_open(websocketpp::connection_hdl& hdl)
	{
		isconnected = true;
		boost::shared_ptr<wsclient> tmp = client;
		if (tmp == NULL)
		{
			return;
		}
		conn = tmp->get_con_from_hdl(hdl);
		if (_connectcallback != NULL)
		{
			Guard locker(mutex);
			_connectcallback(websocketclient);
		}
	}

	void on_message(websocketpp::connection_hdl& hdl, message_ptr& msg)
	{
		if (!msg->get_fin())
		{
			return;
		}

		if (_datacallback != NULL)
		{
			Guard locker(mutex);
			_datacallback(websocketclient, msg->get_payload(), msg->get_opcode() == websocketpp::frame::opcode::text ? WebSocketDataType_Txt :WebSocketDataType_Bin);
		}
	}

	void on_close(websocketpp::connection_hdl& hdl)
	{
		isconnected = false;
		if (_disconnectcallback != NULL)
		{
			Guard locker(mutex);
			_disconnectcallback(websocketclient);
		}
	}

	void on_fail(websocketpp::connection_hdl& hdl)
	{
		isconnected = false;
		if (_disconnectcallback != NULL)
		{
			Guard locker(mutex);
			_disconnectcallback(websocketclient);
		}

		//logerror("Fail handler");
		//logerror("get_state:%d",con->get_state());
		//logerror("get_local_close_code:%d", con->get_local_close_code());
		//logerror("get_local_close_reason:%s", con->get_local_close_reason().c_str());
		//logerror("get_remote_close_code:%d", con->get_remote_close_code());
		//logerror("get_remote_close_reason:%s", con->get_remote_close_reason().c_str());
		//logerror("get_ec:%d - %s", con->get_ec().value(), con->get_ec().message().c_str());
	}

	void on_pong_timeout(websocketpp::connection_hdl& hdl)
	{
		isconnected = false;
		if (_disconnectcallback != NULL)
		{
			Guard locker(mutex);
			_disconnectcallback(websocketclient);
		}
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

WebSocketClient::WebSocketClient(const shared_ptr<IOWorker>& ioworker, const std::map<std::string, Value>&headers)
{
	internal = new WebSocketClientInternal(ioworker);
	internal->websocketclient = this;
}
WebSocketClient::~WebSocketClient()
{
	delete internal;
	//delete internal;
}

bool WebSocketClient::connect(const URL& url, uint32_t timout_ms, const RecvDataCallback& datacallback, const DisconnectCallback& disconnectcallback)
{
    shared_ptr< waitConnectItem> waiting = make_shared<waitConnectItem>();

    startConnect(url, WebSocketClient::ConnnectCallback(&waitConnectItem::connectCallback, waiting), datacallback, disconnectcallback);

	if (0 > waiting->waitsem.pend(timout_ms))
	{
		disconnect();
		return false;
	}

    internal->_connectcallback = WebSocketClient::ConnnectCallback();
	return true;
}
bool WebSocketClient::startConnect(const URL& url, const ConnnectCallback& connectcallback, const RecvDataCallback& datacallback, const DisconnectCallback& disconnectcallback)
{
	return internal->startConnect(url, connectcallback, datacallback, disconnectcallback);
}
bool WebSocketClient::disconnect()
{
	return internal->disconnect();
}
bool WebSocketClient::send(const std::string& data, WebSocketDataType type)
{
	return internal->send(data, type);
}

uint32_t WebSocketClient::sendListSize()
{
	return internal->sendListSize();
}

}
}
