#include "Base/Base.h"
#include "HTTP/Defs.h"
#ifndef GCCSUPORTC11

#ifndef __HTTPRESPONSE_H__
#define __HTTPRESPONSE_H__
#include "HTTP/HTTPServer.h"


namespace Public {
namespace HTTP {

struct HTTPServer::HTTPResponse::HTTPResponseInternal
{
	shared_ptr<HTTPBuffer>		buffer;
	uint32_t					code;
	std::string					error;
	std::map<std::string, URI::Value> header;

	HTTPResponseInternal(HTTPBufferCacheType type):code(404),error("Not Found")
	{
		buffer = make_shared<HTTPBuffer>(type);
	}

	~HTTPResponseInternal()
	{
		buffer = NULL;
	}
};

HTTPServer::HTTPResponse::HTTPResponse()
{
	internal = new HTTPResponseInternal(HTTPBufferCacheType_Mem);
}
HTTPServer::HTTPResponse::~HTTPResponse()
{
	SAFE_DELETE(internal);
}

bool HTTPServer::HTTPResponse::statusCode(uint32_t code,const std::string& statusMessage)
{
	internal->code = code;
	internal->error = statusMessage;

	return true;
}
bool HTTPServer::HTTPResponse::setHeader(const std::string& key, const URI::Value& val)
{
	internal->header[key] = val;

	return true;
}
shared_ptr<HTTPBuffer> HTTPServer::HTTPResponse::buffer()
{
	return internal->buffer;
}

bool HTTPServer::HTTPResponse::end()
{
	return true;
}

}
}


#endif //__HTTPREQUEST_H__
#endif
