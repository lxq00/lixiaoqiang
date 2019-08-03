#include "boost/asio.hpp"
#include "HTTP/HTTPServer.h"
#include "HTTPDefine.h"
#include "HTTPCache.h"
namespace Public {
namespace HTTP {

struct  HTTPServerRequest::HTTPServerRequestInternal
{
	DisconnectCallback		disconnectcallback;
	shared_ptr<HTTPHeader>	header;
	shared_ptr<ReadContent>	content;

	weak_ptr<Socket>		sock;
};

HTTPServerRequest::HTTPServerRequest(const shared_ptr<HTTPHeader>& header, const shared_ptr<ReadContent>& content, const shared_ptr<Socket>& sock)
{
	internal = new HTTPServerRequestInternal;
	internal->header = header;
	internal->content = content;
	internal->sock = sock;
}
HTTPServerRequest::~HTTPServerRequest()
{
	SAFE_DELETE(internal);
}

const std::map<std::string, Value>& HTTPServerRequest::headers() const
{
	return internal->header->headers;
}
Value HTTPServerRequest::header(const std::string& key) const
{
	return internal->header->header(key);
}

const std::string& HTTPServerRequest::method() const
{
	return internal->header->method;
}

const URL& HTTPServerRequest::url()const
{
	return internal->header->url;
}

const shared_ptr<ReadContent>& HTTPServerRequest::content() const
{
	return internal->content;
}

NetAddr HTTPServerRequest::remoteAddr() const
{
	shared_ptr<Socket> sock = internal->sock.lock();
	if (sock == NULL) return NetAddr();
		
	return sock->getOtherAddr();
}

NetAddr HTTPServerRequest::myAddr() const
{
	shared_ptr<Socket> sock = internal->sock.lock();
	if (sock == NULL) return NetAddr();

	return sock->getMyAddr();
}

HTTPServerRequest::DisconnectCallback&	HTTPServerRequest::discallback()
{
	return internal->disconnectcallback;
}

}
}