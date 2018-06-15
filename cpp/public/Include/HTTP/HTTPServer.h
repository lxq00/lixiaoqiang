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
	typedef Function2<void,const shared_ptr<HTTPRequest>&, shared_ptr<HTTPResponse>&> HttpListenCallback;
public:
#if  defined(HAVE_OPENSSL) && defined(GCCSUPORTC11)
	HTTPServer(const shared_ptr<IOWorker>& worker,const std::string &cert_file, const std::string &private_key_file, const std::string &verify_file = std::string());
#endif	
	HTTPServer(
#ifdef GCCSUPORTC11
		const shared_ptr<IOWorker>& worker
#endif
	);
	~HTTPServer();	
	
	// path Ϊ �����url,*Ϊ����  ,callback������Ϣ�Ļص�,�����߳����ݣ�����run����,path ��ʽΪ regex
	// Add resources using path-regex and method-string, and an anonymous function
	bool listen(const std::string& path,const std::string& method,const HttpListenCallback& callback);

	//�첽����
	bool run(uint32_t httpport, uint32_t threadNum = 4);
public:
	//function listen and map resource must usred before function run
	std::map<std::string, std::map<std::string, HTTPServer::HttpListenCallback> > resource;
private:
	struct HTTPServrInternal;
	HTTPServrInternal* internal;
};


}
}


#endif //__HTTPSERVER_H__
