#include "boost/regex.hpp"
#include "boost/shared_ptr.hpp"
#include "boost/make_shared.hpp"
#include <boost/asio.hpp>
#include <websocketpp/server.hpp>
#include <websocketpp/client.hpp>
#include <websocketpp/endpoint.hpp>
#include <websocketpp/config/asio_no_tls.hpp>
#include <websocketpp/config/asio_no_tls_client.hpp>
#include "Network/Network.h"

using namespace Public::Base;
using namespace Public::Network;


#if 0
websocketpp::server<websocketpp::config::asio> server;

int onhttpfunc(websocketpp::connection_hdl hdl)
{
	int a = 0;
	websocketpp::server<websocketpp::config::asio>::connection_ptr con = server.get_con_from_hdl(hdl);
	websocketpp::http::parser::request rt = con->get_request();
	std::string uri = con->get_uri()->get_resource();
	const string& strUri = rt.get_uri();
	const string& strMethod = rt.get_method();
	const string& strBody = rt.get_body();  //只针对post时有数据  
	const string& strHost = rt.get_header("host");
	const string& strVersion = rt.get_version();

	int code = websocketpp::http::status_code::not_found;
	con->set_body("<html><body><p style=\"color:#F000FF;font:14px;font-family: 'Microsoft YaHei';\">温馨提示：</p><p style=\"padding-left: 80px;color:#333333;font:14px;font-family: 'Microsoft YaHei';\">页面被外星人带走啦！</p></body></html>");
	con->set_status(websocketpp::http::status_code::value(code));//HTTP返回码  

	return 1;
}

int main()
{
	shared_ptr<IOWorker> worker = make_shared<IOWorker>(2);

	server.init_asio(((boost::shared_ptr<boost::asio::io_service>*)worker->getBoostASIOIOServerSharedptr())->get());
	server.set_access_channels(websocketpp::log::alevel::none);
	server.set_http_handler(onhttpfunc);
	server.listen(8081);
	server.start_accept();


	getchar();

	return 0;
}

#endif
#if 0
typedef websocketpp::client<websocketpp::config::asio_client> wsclient;
typedef websocketpp::config::asio_client::message_type::ptr message_ptr;

boost::shared_ptr<wsclient> client;

int onopen(websocketpp::connection_hdl hdl)
{
	websocketpp::server<websocketpp::config::asio_client>::connection_ptr con = client->get_con_from_hdl(hdl);
	int code = con->get_state();
	websocketpp::http::parser::response rp = con->get_response();
	int code1 = rp.get_status_code();
	int bodysize = rp.get_body().length();
	int a = 0;
	std::string tmp = rp.raw();

	return 0;
}
int onclose(websocketpp::connection_hdl hdl)
{
	websocketpp::server<websocketpp::config::asio_client>::connection_ptr con = client->get_con_from_hdl(hdl);
	int code = con->get_state();
	websocketpp::http::parser::response rp = con->get_response();
	int code1 = rp.get_status_code();
	int bodysize = rp.get_body().length();
	int a = 0;
	std::string tmp = rp.raw();

	return 0;
}
int on_fail(websocketpp::connection_hdl hdl)
{
	websocketpp::server<websocketpp::config::asio_client>::connection_ptr con = client->get_con_from_hdl(hdl);
	int code = con->get_state();
	websocketpp::http::parser::response rp = con->get_response();
	int code1 = rp.get_status_code();
	int bodysize = rp.get_body().length();
	int a = 0;
	std::string tmp = rp.raw();

	return 0;
}
int on_message(websocketpp::connection_hdl& hdl, message_ptr& msg)
{
	int a = 0;
	return 0;
}
void on_pong(websocketpp::connection_hdl hdl,std::string err)
{
	int a = 0;
}
void on_pong_timeout(websocketpp::connection_hdl hdl, std::string err)
{
	int a = 0;
}
int onhttpfunc(websocketpp::connection_hdl hdl)
{
	int a = 0;

	return 0;
}
int main()
{
	shared_ptr<IOWorker> worker = make_shared<IOWorker>(2);

	client = boost::make_shared<wsclient>();


	client->set_access_channels(websocketpp::log::alevel::none);
	client->set_error_channels(websocketpp::log::elevel::none);
	client->init_asio(((boost::shared_ptr<boost::asio::io_service>*)worker->getBoostASIOIOServerSharedptr())->get());
	client->set_http_handler(onhttpfunc);

	websocketpp::lib::error_code ec;
	wsclient::connection_ptr conn = client->get_connection("http://192.168.2.168", ec);
	if (ec)
	{
		int a = 0;
	}


	//conn->set_http_handler(onhttpfunc);
	//conn->set_open_handler(onopen);
	//conn->set_close_handler(onclose);
	//conn->set_fail_handler(on_fail);
	//conn->set_message_handler(on_message);
	//conn->set_pong_handler(on_pong);
	//conn->set_pong_timeout_handler(on_pong_timeout);
	conn->set_open_handshake_timeout(3000);
	conn->set_pong_timeout(3000);
	client->connect(conn);

	getchar();

	return 0;
}

#endif