#include "boost/asio.hpp"
#include "HTTP/HTTPClient.h"
#include "HTTPDefine.h"
#include "HTTPCache.h"
#include "HTTPCommunication.h"
namespace Public {
namespace HTTP {

struct  HTTPClientResponse::HTTPClientResponseInternal:public WriteContenNotify
{
	shared_ptr<HTTPCommunication> commu;
	shared_ptr<HTTPHeader>	header;
	shared_ptr<ReadContent>	content;

	virtual void WriteNotify()
	{
		commu->startRecv();
	}
	virtual void setWriteFileName(const std::string& filename) {}
	virtual void ReadReady() {}
};

HTTPClientResponse::HTTPClientResponse(const shared_ptr<HTTPCommunication>& commu, HTTPCacheType type, const std::string& filename)
{
	internal = new HTTPClientResponseInternal;
	internal->header = make_shared<HTTPHeader>();
	internal->commu = commu;
	internal->content = make_shared<ReadContent>(commu->recvHeader,internal,type,filename);
}
HTTPClientResponse::~HTTPClientResponse()
{
	SAFE_DELETE(internal);
}

const std::map<std::string, Value>& HTTPClientResponse::headers() const
{
	return internal->header->headers;
}
Value HTTPClientResponse::header(const std::string& key) const
{
	return internal->header->header(key);
}
int HTTPClientResponse::statusCode() const
{
	return internal->header->statuscode;
}
const std::string& HTTPClientResponse::errorMessage() const
{
	return internal->header->statusmsg;
}

const shared_ptr<ReadContent>& HTTPClientResponse::content() const
{
	return internal->content;
}

shared_ptr<HTTPHeader>  HTTPClientResponse::header()
{
	return internal->header;
}

struct HTTPClientRequest::HTTPClientRequestInternal :public WriteContenNotify
{
	HTTPClientRequest::DisconnectCallback	disconnected;

	shared_ptr<HTTPHeader> header;
	shared_ptr<WriteContent> content;

	weak_ptr<HTTPCommunication> commu;

	uint32_t	timeout;

	virtual void WriteNotify() {}
	virtual void ReadReady() {}
	virtual void setWriteFileName(const std::string& filename)
	{
		std::string contenttype = "application/octet-stream";

		do
		{
			int pos = (int)String::lastIndexOf(filename, ".");
			if (pos == -1) break;

			std::string pres = filename.c_str() + pos + 1;

			uint32_t mimetypeslen = 0;
			ContentInfo* mimetypes = MediaType::info(mimetypeslen);

			bool haveFind = false;
			for (uint32_t i = 0; i < mimetypeslen; i++)
			{
				if (strcasecmp(pres.c_str(), mimetypes[i].filetype) == 0)
				{
					contenttype = mimetypes[i].contentType;
					break;
				}
			}

		} while (0);

		header->headers[Content_Type] = contenttype;
	}
};

HTTPClientRequest::HTTPClientRequest(const std::string& method,const std::string& url, HTTPCacheType type)
{
	internal = new HTTPClientRequestInternal;

	internal->header = make_shared<HTTPHeader>();
	internal->content = make_shared<WriteContent>(internal->header,internal, type);
	internal->header->method = method;
	internal->header->url = url;

	internal->timeout = 10000;
}
HTTPClientRequest::~HTTPClientRequest()
{
	SAFE_DELETE(internal);
}
URL& HTTPClientRequest::url()
{
	return internal->header->url;
}

std::string& HTTPClientRequest::method()
{
	return internal->header->method;
}

std::map<std::string, Value>& HTTPClientRequest::headers()
{
	return internal->header->headers;
}
Value HTTPClientRequest::header(const std::string& key)
{
	return internal->header->header(key);
}

shared_ptr<WriteContent>& HTTPClientRequest::content()
{
	return internal->content;
}

uint32_t& HTTPClientRequest::timeout()
{
	return internal->timeout;
}

HTTPClientRequest::DisconnectCallback&	HTTPClientRequest::discallback()
{
	return internal->disconnected;
}
shared_ptr<HTTPHeader> HTTPClientRequest::header()
{
	return internal->header;
}


}
}