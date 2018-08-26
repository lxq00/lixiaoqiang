#ifndef __XM_HTTPCLIENT_H__
#define __XM_HTTPCLIENT_H__

#include "HTTPPublic.h"

 namespace Public{
 namespace HTTP{

 ///HTTP¿Í»§¶Ë·â×°¿â
 class HTTP_API HTTPClient
 {
 public:
 	HTTPClient(const shared_ptr<IOWorker>& worker,const std::string& useragent);
 	~HTTPClient();

	const shared_ptr<HTTPResponse> request(const shared_ptr<HTTPRequest>& req, HTTPBuffer::HTTPBufferType type = HTTPBuffer::HTTPBufferType_String, const std::string& outfile = "");
	const shared_ptr<HTTPResponse> request(const std::string& url , const std::string& method, const shared_ptr<HTTPBuffer> buf = shared_ptr<HTTPBuffer>(), const std::map<std::string, Value>& headers = std::map<std::string, Value>(), int timeout = 30000, HTTPBuffer::HTTPBufferType type = HTTPBuffer::HTTPBufferType_String, const std::string& outfile = "");
private:	 
	 struct HTTPClientInternal;
	 HTTPClientInternal* internal;
 };

 class HTTP_API HTTPAsyncClient
 {
 public:
	 HTTPAsyncClient(const shared_ptr<IOWorker>& worker, const std::string& useragent);
	 ~HTTPAsyncClient();
	 bool request(const shared_ptr<HTTPRequest>& req, const HTTPCallback& callback,HTTPBuffer::HTTPBufferType type = HTTPBuffer::HTTPBufferType_String,const std::string& outfile = "");
 private:
	 struct HTTPAsyncClientInternal;
	 HTTPAsyncClientInternal*internal;
 };
 
 }
 }

#endif //__XM_HTTPCLIENT_H__
