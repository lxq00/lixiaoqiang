#ifndef __XM_HTTPCLIENT_H__
#define __XM_HTTPCLIENT_H__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <vector>
#include "HTTPPublic.h"

 namespace Xunmei{
 namespace HTTP{
 
 ///HTTP¿Í»§¶Ë·â×°¿â
 class HTTP_API HTTPClient
 {
 public:
	 struct HTTPClientInternal;

 	typedef enum{
 		HTTPHeader_Username,
 		HTTPHeader_Password,
 		HTTPHeader_Content_Encoding,
 		HTTPHeader_User_Agent,
 		HTTPHeader_Connection,
		HTTPHeader_Cookies,
 	}HTTPHeaderType;
	class HTTP_API HTTPForm
	{
		friend class HTTPClient;
		friend  class HTTPClientAsync;

		HTTPForm();
	public:		
		~HTTPForm();
		bool addContents(const std::string& key, const std::string& data);
		bool addFile(const std::string& filename);
		bool addFile(const std::string& filename, const char* filebuffer, uint32_t bufferlen);
	private:
		struct HTTPFormInternal;
		HTTPFormInternal* internal;
	};
	class HTTP_API HTTPRequest
	{
		friend class HTTPClient;
		friend  class HTTPClientAsync;
	public:
		HTTPRequest(HTTPBufferCacheType type = HTTPBufferCacheType_Mem);
		~HTTPRequest();

		bool setHeader(HTTPHeaderType key, const URI::Value& val);
		bool setHeader(const std::string& key, const URI::Value& val);
		shared_ptr<HTTPBuffer> buffer();
		shared_ptr<HTTPForm>  form();
	private:
		struct HTTPRequestInternal;
		HTTPRequestInternal* internal;
	};

	class HTTP_API HTTPResponse
	{
		friend class HTTPClient;
		friend  class HTTPClientAsync;
		HTTPResponse(HTTPBufferCacheType type);
	public:		
		~HTTPResponse();

		uint32_t statucCode() const;
		std::string errorMessage() const;
		const shared_ptr<HTTPBuffer> buffer() const;
		URI::Value getHeader(const std::string& key) const;
		const std::map<std::string, URI::Value> getHeader() const;
	private:
		struct HTTPResponseInternal;
		HTTPResponseInternal* internal;
	};	
 public:
 	HTTPClient(const URL& url,int timeout = 5000, HTTPBufferCacheType type = HTTPBufferCacheType_Mem);
 	~HTTPClient();

	const shared_ptr<HTTPResponse> request(const std::string& method,const shared_ptr<HTTPRequest>& req = shared_ptr<HTTPRequest>());	
private:	 
	 HTTPClientInternal* internal;
 };
 
 class HTTP_API HTTPClientAsync
 {
 public:
	 typedef Function3<void,const URL&,const shared_ptr<HTTPClient::HTTPResponse>&, void*> AsynHttpRequest;
 public:
	 HTTPClientAsync(const AsynHttpRequest& callback, HTTPBufferCacheType type = HTTPBufferCacheType_Mem);
	 ~HTTPClientAsync();

	 bool requestAsyn(const URL& url, const std::string &method, int timeout, void* content, const shared_ptr<HTTPClient::HTTPRequest>& req = shared_ptr<HTTPClient::HTTPRequest>());
 private:
	 struct MultiHttpClient;
	 MultiHttpClient* internal;
 };

 }
 }

#endif //__XM_HTTPCLIENT_H__
