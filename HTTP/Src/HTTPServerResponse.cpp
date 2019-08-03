#include "boost/asio.hpp"
#include "HTTP/HTTPServer.h"
#include "HTTPDefine.h"
#include "HTTPCache.h"
#include "HTTPCommunication.h"
#include "HTTPDefine.h"
namespace Public {
namespace HTTP {

struct HTTPServerResponse::HTTPServerResponseInternal:public WriteContenNotify
{
	shared_ptr<HTTPHeader> header;
	shared_ptr<WriteContent> content;

	weak_ptr<HTTPCommunication> commu;

	virtual void WriteNotify()
	{
		shared_ptr<HTTPCommunication> commutmp = commu.lock();
		if (commutmp) commutmp->setSendHeaderContentAndPostSend(header, content);
	}
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

HTTPServerResponse::HTTPServerResponse(const shared_ptr<HTTPCommunication>& commu, HTTPCacheType type)
{
	internal = new HTTPServerResponseInternal;

	internal->header = make_shared<HTTPHeader>();
	internal->content = make_shared<WriteContenNotify>(internal, type);

	internal->header->statuscode = 200;
	internal->header->statusmsg = "OK";
}
HTTPServerResponse::~HTTPServerResponse()
{
	internal->WriteNotify();
	SAFE_DELETE(internal);
}

int& HTTPServerResponse::statusCode()
{
	return internal->header->statuscode;
}
std::string& HTTPServerResponse::errorMessage()
{
	return internal->header->statusmsg;
}

std::map<std::string, Value>& HTTPServerResponse::headers()
{
	return internal->header->headers;
}
Value HTTPServerResponse::header(const std::string& key)
{
	return internal->header->header(key);
}

shared_ptr<WriteContent>& HTTPServerResponse::content()
{
	return internal->content;
}

}
}