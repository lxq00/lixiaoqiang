#ifndef __HTTPSERVER_H__
#define __HTTPSERVER_H__
#include "HTTPPublic.h"
#include "Base/Base.h"
using namespace Public::Base;
namespace Public {
namespace HTTP {

class HTTP_API HTTPServer
{
public:
	class HTTP_API HTTPRequest
	{
		friend class HTTPServer;
	public:
		HTTPRequest(HTTPBufferCacheType type);
		~HTTPRequest();

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
		friend class HTTPServer;
	public:
		HTTPResponse(HTTPBufferCacheType type);
		~HTTPResponse();

		bool statusCode(uint32_t code, const std::string& statusMessage = "");

		//设置头回复包头信息，Content-Length 无效
		bool setHeader(const std::string& key, const URI::Value& val);

		shared_ptr<HTTPBuffer> buffer();

		//end后将会发送数据，buffer操作将会无效
		bool end();
	private:
		struct HTTPResponseInternal;
		HTTPResponseInternal* internal;
	};

	typedef Function2<void,const shared_ptr<HTTPRequest>&,shared_ptr<HTTPResponse>&> HttpListenCallback;
public:

	HTTPServer(const std::string& webrootpath,const std::string& cachedir="./", HTTPBufferCacheType type = HTTPBufferCacheType_Mem);
	~HTTPServer();	
	
	// path 为 请求的url,*为所有  ,callback监听消息的回掉,处理线程数据，先于run启用
	bool listen(const std::string& path,const HttpListenCallback& callback);

	//异步监听
	bool run(uint32_t httpport);

	static std::string getXMQWebVersion(const std::string& webaddr);

	///构造一个中转代理url
	static std::string buildProxyUrl(const std::string& webaddr, const std::string& url);

	static std::string parseProxyUrl(const std::string& proxyUrl);
private:
	struct HTTPServrInternal;
	HTTPServrInternal* internal;
};


}
}


#endif //__HTTPSERVER_H__