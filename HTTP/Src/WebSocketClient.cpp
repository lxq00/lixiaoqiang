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
#include "WebSocketCallbackAgent.h"

namespace Public {
namespace HTTP {
	
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
