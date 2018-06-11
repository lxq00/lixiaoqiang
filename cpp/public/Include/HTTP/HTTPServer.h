#ifndef __HTTPSERVER_H__
#define __HTTPSERVER_H__
#include "HTTPPublic.h"
#include "Base/Base.h"
using namespace Public::Base;
namespace Public {
namespace HTTP {

struct WebInterface;
class HTTP_API HTTPServer
{
public:
	class HTTP_API HTTPRequest
	{
		friend struct WebInterface;
		friend class HTTPServer;
	public:
		HTTPRequest();
		~HTTPRequest();
		std::map<std::string, URI::Value> headers() const;
		std::string header(const std::string& key) const;
		std::string method() const;
		shared_ptr<URL> url() const;
		shared_ptr<HTTPBuffer> buffer() const;
	private:
		struct HTTPRequestInternal;
		HTTPRequestInternal* internal;
	};

	class HTTP_API HTTPResponse
	{
		friend struct WebInterface;
		friend class HTTPServer;
	public:
		HTTPResponse();
		~HTTPResponse();

		bool statusCode(uint32_t code, const std::string& statusMessage = "");

		//设置头回复包头信息，Content-Length 无效
		bool setHeader(const std::string& key, const URI::Value& val);

		shared_ptr<HTTPBuffer> buffer();
	private:
		//end后将会发送数据，buffer操作将会无效
		bool end();
	private:
		struct HTTPResponseInternal;
		HTTPResponseInternal* internal;
	};

	typedef Function2<void,const shared_ptr<HTTPRequest>&, shared_ptr<HTTPResponse>&> HttpListenCallback;
public:
#ifndef GCCSUPORTC11
	HTTPServer(const std::string &cert_file, const std::string &private_key_file, const std::string &verify_file = std::string(),HTTPBufferCacheType type = HTTPBufferCacheType_Mem);
#endif	
	HTTPServer(HTTPBufferCacheType type = HTTPBufferCacheType_Mem);
	~HTTPServer();	
	
	// path 为 请求的url,*为所有  ,callback监听消息的回掉,处理线程数据，先于run启用,path 格式为 regex
	// Add resources using path-regex and method-string, and an anonymous function
	bool listen(const std::string& path,const std::string& method,const HttpListenCallback& callback);

	//异步监听
	bool run(uint32_t httpport, uint32_t threadNum = 4);
private:
	struct HTTPServrInternal;
	HTTPServrInternal* internal;
};


}
}


#endif //__HTTPSERVER_H__