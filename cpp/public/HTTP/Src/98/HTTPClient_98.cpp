#include "Base/Defs.h"
#include "HTTP/Defs.h"

#ifndef GCCSUPORTC11
#include "curl/curl.h"
#include "HTTP/HTTPClient.h"
#include "HTTPClient_Async_98.h"

 namespace Public{
 namespace HTTP{



struct HTTPClient::HTTPClientInternal:public HTTPClientAsyncInternal
{
	HTTPClientInternal() :HTTPClientAsyncInternal() {}

	URI url;
	shared_ptr<HTTPClientAsync> async;


	static std::string strip(const std::string& tmp)
	{
		const char* tmpptr = tmp.c_str();
		while (*tmpptr == ' ') tmpptr++;
		int len = strlen(tmpptr);
		while (len > 0 && tmpptr[len - 1] == ' ') len--;

		return std::string(tmpptr, len);
	}

	static void parseHttpHeader(const shared_ptr<HTTPResponse>& res,const shared_ptr<HTTPBuffer>& buffer)
	{
		if (buffer == NULL) return;

		std::string datatmp = buffer->read();

		const char* tmp = datatmp.c_str();
		while (1)
		{
			const char* nexttmp = NULL;
			const char* tmp1 = strstr(tmp, "\r\n");
			if (tmp1 != NULL)
			{
				nexttmp = tmp1 + 2;
			}
			else if (tmp1 == NULL && (tmp1 = strchr(tmp, '\n')) != NULL)
			{
				nexttmp = tmp1 + 1;
			}
			std::string headerdata = (tmp1 == NULL) ? tmp : std::string(tmp, tmp1 - tmp);
			const char* headerptr = headerdata.c_str();
			const char* posptr = strchr(headerptr, ':');
			if (posptr != NULL)
			{
				std::string key = strip(std::string(headerptr, posptr - headerptr));
				std::string val = strip(posptr + 1);

				res->headers[key] = val;
			}
			if (nexttmp == NULL) break;
			tmp = nexttmp;
		}
	}
};
 
HTTPClient::HTTPClient(const URI& url):HTTPClient(url.href()){}
HTTPClient::HTTPClient(const std::string& url)
{
	internal = new HTTPClientInternal();
	internal->url.href(url);
}

HTTPClient::~HTTPClient()
{
	SAFE_DELETE(internal);
}

const shared_ptr<HTTPResponse> HTTPClient::request(const shared_ptr<HTTPRequest>& req)
{
	if (internal->curl == NULL)
	{
		return internal->response;
	}

	internal->buildRequest(req);

	CURLcode res = curl_easy_perform(internal->curl);
	curl_easy_getinfo(internal->curl, CURLINFO_HTTP_CODE, &internal->response->statusCode);
	if (res == CURLE_OK)
	{
		internal->response->errorMessage = "";
	}
	else
	{
		internal->response->statusCode = res;
		internal->response->errorMessage = curl_easy_strerror(res);
	}
	HTTPClientInternal::parseHttpHeader(internal->response,internal->headerbuf);

	return internal->response;
}

const shared_ptr<HTTPResponse> HTTPClient::request(const std::string& method, const HTTPBuffer& buf, const std::map<std::string, URI::Value>& headers)
{
	shared_ptr<HTTPRequest> req = make_shared<HTTPRequest>();
	req->method = method;
	req->buf.write(buf.read());
	req->headers = headers;

	return request(req);
}

static weak_ptr<HTTPClientAsync> g_httpAsync;
static Mutex					 g_httpAsyncmutex;

bool HTTPClient::request(const shared_ptr<HTTPRequest>& req, const AsynCallback& callback)
{
	{
		Guard locker(g_httpAsyncmutex);
		internal->async = g_httpAsync.lock();
		if (internal->async == NULL)
		{
			g_httpAsync = internal->async = make_shared<HTTPClientAsync>();
		}
	}

	return internal->async->requestAsyn(req, callback);
}



}
}
 
#endif