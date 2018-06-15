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
	 typedef Function2<void, const shared_ptr<HTTPRequest>&, const shared_ptr<HTTPResponse>&> AsynCallback;
 public:
 	HTTPClient(
#ifdef GCCSUPORTC11
		const shared_ptr<IOWorker>& worker,
#endif
		const URI& url
	);
	HTTPClient(
#ifdef GCCSUPORTC11
		const shared_ptr<IOWorker>& worker,
#endif
		const std::string& url
	);

 	~HTTPClient();

	const shared_ptr<HTTPResponse> request(const shared_ptr<HTTPRequest>& req);
	const shared_ptr<HTTPResponse> request(const std::string& method, const HTTPBuffer& buf = HTTPBuffer(), const std::map<std::string, URI::Value>& headers = std::map<std::string, URI::Value>(),int timeout = 30000);
	bool request(const shared_ptr<HTTPRequest>& req, const AsynCallback& callback);
private:	 
	 struct HTTPClientInternal;
	 HTTPClientInternal* internal;
 };
 
 }
 }

#endif //__XM_HTTPCLIENT_H__
