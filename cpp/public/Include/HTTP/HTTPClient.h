#ifndef __XM_HTTPCLIENT_H__
#define __XM_HTTPCLIENT_H__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <vector>
#include "HTTPPublic.h"

 namespace Public{
 namespace HTTP{
 
 ///HTTP¿Í»§¶Ë·â×°¿â
 class HTTP_API HTTPClient
 {
 public:
	 typedef enum
	 {
		 HTTPClient_HTTP,
		 HTTPClient_HTTPS,
	 }Type;
	 typedef Function2<void, const shared_ptr<HTTPRequest>&, const shared_ptr<HTTPResponse>&> AsynCallback;
 public:
 	HTTPClient(HTTPClient::Type type,const std::string& serverip,uint32_t port = 80);
 	~HTTPClient();

	const shared_ptr<HTTPResponse> request(const shared_ptr<HTTPRequest>& req);
	const shared_ptr<HTTPResponse> request(const std::string& method, const std::string& url, const HTTPBuffer& buf, const std::map<std::string, URI::Value>& headers = std::map<std::string, URI::Value>());
	bool request(const shared_ptr<HTTPRequest>& req, const AsynCallback& callback);
private:	 
	 struct HTTPClientInternal;
	 HTTPClientInternal* internal;
 };
 
 }
 }

#endif //__XM_HTTPCLIENT_H__
