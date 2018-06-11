#include "Base/Defs.h"
#include "HTTP/Defs.h"

#ifdef GCCSUPORTC11

#include "Simple-Web-Server-master/client_http.hpp"
#ifdef HAVE_OPENSSL
#include "Simple-Web-Server-master/client_https.hpp"
#endif
#include "HTTP/HTTPClient.h"


namespace Public {
namespace HTTP {

using SimpleHttpClient = SimpleWeb::Client<SimpleWeb::HTTP>;
#ifdef HAVE_OPENSSL
using SimpleHttpsClient = SimpleWeb::Client<SimpleWeb::HTTPS>;
#endif

class HTTPCallbackObjct
{
public:
	HTTPCallbackObjct(const shared_ptr<HTTPRequest>& _req, const HTTPClient::AsynCallback& _callback) :callback(_callback), req(_req), endofwork(false) {}
	~HTTPCallbackObjct() {}

	void HTTPCallback(shared_ptr<SimpleHttpClient::Response> response, const SimpleWeb::error_code &ec)
	{
		shared_ptr<HTTPResponse> resp = buildHTTPResponse(response, ec);
		callback(req, resp);
		endofwork = true;
	}
#ifdef HAVE_OPENSSL
	void HTTPSCallback(shared_ptr<SimpleHttpsClient::Response> response, const SimpleWeb::error_code &ec)
	{
		shared_ptr<HTTPResponse> resp = buildHTTPResponse(response, ec);
		callback(req, resp);
		endofwork = true;
	}
#endif
	template<typename T>
	static shared_ptr<HTTPResponse> buildHTTPResponse(const shared_ptr<T>& response, const SimpleWeb::error_code &ec)
	{
		shared_ptr<HTTPResponse> res(new HTTPResponse());
		if(ec.value() == 0) res->setStatusCode(200, "OK");
		else res->setStatusCode(ec.value(), ec.message());

		if (response != NULL)
		{
			std::map<std::string, URI::Value> headerstmp;
			for (SimpleWeb::CaseInsensitiveMultimap::const_iterator iter = response->header.begin(); iter != response->header.end(); iter++)
			{
				res->headers[iter->first] = iter->second;
			}

			res->buf.write(response->content.string());
		}

		return res;
	}
private:
	HTTPClient::AsynCallback	callback;
	shared_ptr<HTTPRequest>		req;
public:
	bool						endofwork;
};

class HTTPObject
{
public:
	HTTPObject() {}
	virtual ~HTTPObject() {}
	virtual shared_ptr<HTTPResponse> request(const shared_ptr<HTTPRequest>& req) = 0;
	virtual bool request(const shared_ptr<HTTPRequest>& req, const HTTPClient::AsynCallback& callback) = 0;
};

template<typename T>
class SIMPHTTPObject :public HTTPObject
{
public:
	SIMPHTTPObject(const std::string& host,bool _https) :https(_https)
	{
		client = make_shared<T>(host);

		m_timer = make_shared<Timer>("SIMPHTTPObject");
		m_timer->start(Timer::Proc(&SIMPHTTPObject::onPoolTimerProc, this), 0, 10000);
	}
	~SIMPHTTPObject()
	{
		m_timer = NULL;
		client = NULL;
	}
	virtual shared_ptr<HTTPResponse> request(const shared_ptr<HTTPRequest>& req)
	{
		SimpleWeb::CaseInsensitiveMultimap ackhreaders;
		for (std::map<std::string, URI::Value>::const_iterator iter = req->headers.begin(); iter != req->headers.end(); iter++)
		{
			ackhreaders.insert(make_pair(iter->first, iter->second.readString()));
		}

		try
		{
			shared_ptr<T::Response> response = client->request(req->method, req->url.getPath(), req->buf.buf.str(), ackhreaders);

			return HTTPCallbackObjct::buildHTTPResponse(response, SimpleWeb::error_code());
		}
		catch (const SimpleWeb::system_error &e) {
			return HTTPCallbackObjct::buildHTTPResponse(shared_ptr<T::Response>(), e.code());
		}

		
	}
	virtual bool request(const shared_ptr<HTTPRequest>& req, const HTTPClient::AsynCallback& callback)
	{
		shared_ptr<HTTPCallbackObjct> callbackobj = make_shared<HTTPCallbackObjct>(req, callback);
		SimpleWeb::CaseInsensitiveMultimap ackhreaders;
		for (std::map<std::string, URI::Value>::const_iterator iter = req->headers.begin(); iter != req->headers.end(); iter++)
		{
			ackhreaders.insert(make_pair(iter->first, iter->second.readString()));
		}

		if (!https)
		{
			client->request(req->method, req->url.getPath(), req->buf.buf.str(), ackhreaders, 
				std::bind(&HTTPCallbackObjct::HTTPCallback, callbackobj, std::placeholders::_1, std::placeholders::_2));
		}
#ifdef HAVE_OPENSSL
		else
		{
			client->request(req->method, req->url.getPath(), req->buf.buf.str(), ackhreaders, 
				std::bind(&HTTPCallbackObjct::HTTPSCallback, callbackobj, std::placeholders::_1, std::placeholders::_2));
		}
#endif

		{
			Guard locker(m_mutex);
			responseList.push_back(callbackobj);
		}

		return true;
	}
private:
	void onPoolTimerProc(unsigned long)
	{
		Guard locker(m_mutex);
		for (std::list<shared_ptr<HTTPCallbackObjct> >::iterator iter = responseList.begin(); iter != responseList.end();)
		{
			if ((*iter)->endofwork)
			{
				responseList.erase(iter++);
			}
			else
			{
				iter++;
			}
		}
	}
private:
	shared_ptr<Timer>						  m_timer;
	Mutex									  m_mutex;
	shared_ptr<T>							  client;
	std::list<shared_ptr<HTTPCallbackObjct> > responseList;
	bool									  https;
};

struct HTTPClient::HTTPClientInternal
{
	shared_ptr<HTTPObject>		clientObjct;
};

HTTPClient::HTTPClient(HTTPClient::Type type, const std::string& serverip, uint32_t port)
{
	internal = new HTTPClientInternal;

	stringstream sstream;
	sstream << serverip << ":" << port;

	if (type == HTTPClient_HTTP)
	{
		internal->clientObjct = make_shared<SIMPHTTPObject<SimpleHttpClient> >(sstream.str(),false);
	}
#ifdef HAVE_OPENSSL
	else if (type == HTTPClient_HTTPS)
	{
		internal->clientObjct = make_shared<SIMPHTTPObject<SimpleHttpsClient> >(sstream.str(), true);
	}
#endif
}
HTTPClient::~HTTPClient()
{
	SAFE_DELETE(internal);
}

const shared_ptr<HTTPResponse> HTTPClient::request(const shared_ptr<HTTPRequest>& req)
{
	return internal->clientObjct->request(req);
}
const shared_ptr<HTTPResponse> HTTPClient::request(const std::string & method, const std::string& url, const HTTPBuffer& buf, const std::map<std::string, URI::Value>& headers)
{
	shared_ptr<HTTPRequest> req = make_shared<HTTPRequest>();
	req->method = method;
	req->url = url;
	req->buf.write(buf.buf.str());
	req->headers = headers;

	return request(req);
}
bool HTTPClient::request(const shared_ptr<HTTPRequest>& req, const AsynCallback& callback)
{
	return internal->clientObjct->request(req, callback);
}

}
}

#endif