#ifndef __HTTPREQUEST_H__
#define __HTTPREQUEST_H__
#include "HTTP/HTTPServer.h"


namespace Xunmei {
namespace HTTP {

typedef Function1<std::string,const std::string& >	QueryCallback;

struct HTTPServer::HTTPRequest::HTTPRequestInternal
{
	QueryCallback				callback;
	std::string					method;
	shared_ptr<URL>				url;
	shared_ptr<HTTPBuffer>	buffer;

	HTTPRequestInternal(HTTPBufferCacheType type):buffer(new HTTPBuffer(type)){}
	~HTTPRequestInternal()
	{
	}

	bool init(const std::string& _url,const std::string& cachedir,const QueryCallback& _callback)
	{
		callback = _callback;
		url = new URL(_url);

		return true;
	}
};

HTTPServer::HTTPRequest::HTTPRequest(HTTPBufferCacheType type)
{
	internal = new HTTPRequestInternal(type);
}
HTTPServer::HTTPRequest::~HTTPRequest()
{
	SAFE_DELETE(internal);
}
std::string HTTPServer::HTTPRequest::header(const std::string& key) const
{
	return internal->callback(key);
}
std::string HTTPServer::HTTPRequest::method() const
{
	return internal->method;
}
shared_ptr<URL> HTTPServer::HTTPRequest::url() const
{
	return internal->url;
}
shared_ptr<HTTPBuffer> HTTPServer::HTTPRequest::buffer() const
{
	return internal->buffer;
}

}
}


#endif //__HTTPREQUEST_H__
