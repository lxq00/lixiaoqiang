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

}
}
