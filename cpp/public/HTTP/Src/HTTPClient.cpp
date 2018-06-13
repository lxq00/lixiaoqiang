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
using SimpleHttpClientBaseResponse = SimpleWeb::ClientBase<SimpleWeb::HTTP>::Response;
#ifdef HAVE_OPENSSL
using SimpleHttpsClient = SimpleWeb::Client<SimpleWeb::HTTPS>;
using SimpleHttpsClientBaseResponse = SimpleWeb::ClientBase<SimpleWeb::HTTPS>::Response;
#endif

class HTTPCallbackObjct
{
public:
	HTTPCallbackObjct(const shared_ptr<HTTPRequest>& _req, const HTTPClient::AsynCallback& _callback) :callback(_callback), req(_req), endofwork(false) {}
	~HTTPCallbackObjct() {}

	void HTTPCallback(shared_ptr<SimpleHttpClientBaseResponse> response, const SimpleWeb::error_code &ec)
	{
		shared_ptr<HTTPResponse> resp = buildHTTPResponse(response, ec.value(), ec.message());
		callback(req, resp);
		endofwork = true;
	}
#ifdef HAVE_OPENSSL
	void HTTPSCallback(shared_ptr<SimpleHttpsClientBaseResponse> response, const SimpleWeb::error_code &ec)
	{
		shared_ptr<HTTPResponse> resp = buildHTTPResponse(response, ec.value(), ec.message());
		callback(req, resp);
		endofwork = true;
	}
#endif
	template<typename T>
	static shared_ptr<HTTPResponse> buildHTTPResponse(const shared_ptr<T>& response, int errorcode,const std::string& errormsg)
	{
		shared_ptr<HTTPResponse> res(new HTTPResponse());
		if(errorcode == 200) res->setStatusCode(200, "OK");
		else res->setStatusCode(errorcode, errormsg);

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

template<typename T,bool HTTPS>
class SIMPHTTPObject :public HTTPObject
{
	typedef typename T::Response SIMPHTTPResponse;
public:
	SIMPHTTPObject(const std::string& host)
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
			shared_ptr<SIMPHTTPResponse> response = client->request(req->method, req->url.getPath(),req->buf.read() , ackhreaders);

			int code = 200;
			std::string msg;
			{
				const char* tmp = response->status_code.c_str();
				const char* tmp1 = strchr(tmp, ' ');
				code = atoi(std::string(tmp, tmp1 - tmp).c_str());
				msg = tmp1 + 1;
			}


			return HTTPCallbackObjct::buildHTTPResponse(response, code,msg);
		}
		catch (const SimpleWeb::system_error &e) {
			return HTTPCallbackObjct::buildHTTPResponse(shared_ptr<SIMPHTTPResponse>(), e.code().value(),e.code().message());
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

		webRequest(req, ackhreaders, callbackobj);

		{
			Guard locker(m_mutex);
			responseList.push_back(callbackobj);
		}

		return true;
	}
private:
	virtual void webRequest(const shared_ptr<HTTPRequest>& req, SimpleWeb::CaseInsensitiveMultimap& ackhreaders, shared_ptr<HTTPCallbackObjct>& callbackobj) = 0;
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
protected:
	shared_ptr<Timer>						  m_timer;
	Mutex									  m_mutex;
	shared_ptr<T>							  client;
	std::list<shared_ptr<HTTPCallbackObjct> > responseList;
};

class WEBHTTPOBJECT :public SIMPHTTPObject<SimpleHttpClient, false>
{
public:
	WEBHTTPOBJECT(const std::string& host, bool _https):SIMPHTTPObject(host){}
	~WEBHTTPOBJECT() {}
	void webRequest(const shared_ptr<HTTPRequest>& req, SimpleWeb::CaseInsensitiveMultimap& ackhreaders, shared_ptr<HTTPCallbackObjct>& callbackobj)
	{
		std::function<void(std::shared_ptr<SimpleHttpClientBaseResponse>, const SimpleWeb::error_code &)> funccall = std::bind(&HTTPCallbackObjct::HTTPCallback, callbackobj, std::placeholders::_1, std::placeholders::_2);
		SimpleWeb::string_view data = req->buf.read();
		client->request(String::toupper(req->method), req->url.getPath(), data, ackhreaders, static_cast<std::function<void(std::shared_ptr<SimpleHttpClientBaseResponse>, const SimpleWeb::error_code &)>&&>(funccall));
	}
};

#ifdef HAVE_OPENSSL
class WEBHTTPSOBJECT :public SIMPHTTPObject<SimpleHttpsClient, true>
{
public:
	WEBHTTPSOBJECT(const std::string& host, bool _https) :SIMPHTTPObject(host) {}
	~WEBHTTPSOBJECT() {}
	void webRequest(const shared_ptr<HTTPRequest>& req, SimpleWeb::CaseInsensitiveMultimap& ackhreaders, shared_ptr<HTTPCallbackObjct>& callbackobj)
	{
		std::function<void(std::shared_ptr<SimpleHttpsClientBaseResponse>, const SimpleWeb::error_code &)> funccall = std::bind(&HTTPCallbackObjct::HTTPSCallback, callbackobj, std::placeholders::_1, std::placeholders::_2);
		SimpleWeb::string_view data = req->buf.read();
		client->request(String::toupper(req->method), req->url.getPath(), data, ackhreaders, static_cast<std::function<void(std::shared_ptr<SimpleHttpsClientBaseResponse>, const SimpleWeb::error_code &)>&&>(funccall));
	}
};
#endif

struct HTTPClient::HTTPClientInternal
{
	shared_ptr<HTTPObject>		clientObjct;
	URI							url;
};

HTTPClient::HTTPClient(const std::string& url)
{
	internal = new HTTPClientInternal;
	internal->url.href(url);

	stringstream reqstring;
	reqstring << internal->url.getHost() << ":" << internal->url.getPort();
	
	if (strcasecmp(internal->url.getProtocol().c_str(), "http") == 0)
	{
		internal->clientObjct = make_shared<WEBHTTPOBJECT>(reqstring.str(), false);
	}
#ifdef HAVE_OPENSSL
	else if (strcasecmp(internal->url.getProtocol().c_str(), "https") == 0)
	{
		internal->clientObjct = make_shared<WEBHTTPSOBJECT>(reqstring.str(), true);
	}
#endif
}
HTTPClient::HTTPClient(const URI& url):HTTPClient(url.href())
{
}
HTTPClient::~HTTPClient()
{
	SAFE_DELETE(internal);
}

const shared_ptr<HTTPResponse> HTTPClient::request(const shared_ptr<HTTPRequest>& req)
{
	req->url = internal->url;
	return internal->clientObjct->request(req);
}
const shared_ptr<HTTPResponse> HTTPClient::request(const std::string & method, const HTTPBuffer& buf, const std::map<std::string, URI::Value>& headers)
{
	shared_ptr<HTTPRequest> req = make_shared<HTTPRequest>();
	req->method = method;
	req->url = internal->url;
	req->buf.write(buf.read());
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