#ifndef __HTTPPUBLIC_H__
#define __HTTPPUBLIC_H__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <vector>
#include <fstream>     // std::cout  
#include <sstream>      // std::istringstream  

#ifndef WIN32
#include<string>
#endif

#include "Defs.h"
#include "Base/Base.h"
using namespace Public::Base;

namespace Public {
namespace HTTP {


//���� http://user:pass@host.com:8080/p/a/t/h?query=string#hash
class HTTP_API URL
{
public:
	struct AuthenInfo
	{
		std::string Username;
		std::string Password;
	};
public:
	URL();
	URL(const std::string& href);
	URL(const URL& url);
	virtual ~URL();

	void clean();

	//http://user:pass@host.com:8080/p/a/t/h?query=string#hash
	std::string href() const;
	void href(const std::string& url);

	//http
	std::string protocol;
	const std::string& getProtocol() const;
	void setProtocol(const std::string& protocolstr);

	//user:pass
	AuthenInfo	authen;
	std::string getAuhen() const;
	const AuthenInfo& getAuthenInfo() const;
	void setAuthen(const std::string& authenstr);
	void setAuthen(const std::string& username, std::string& password);
	void setAuthen(const AuthenInfo& info);
	

	//host.com:8080
	std::string getHost() const;
	void setHost(const std::string& hoststr);

	//host.com
	std::string hostname;
	const std::string& getHostname() const;
	void setHostname(const std::string& hostnamestr);

	//8080
	uint32_t port;
	uint32_t getPort() const;
	void setPort(uint32_t portnum);

	///p/a/t/h?query=string#hash
	std::string getPath() const;
	void setPath(const std::string& pathstr);

	//p/a/t/h
	std::string pathname;
	const std::string& getPathname() const;
	void setPathname(const std::string& pathnamestr);

	//?query=string#hash
	std::string getSearch() const;
	void setSearch(const std::string& searchstr);

	//<query,string#assh>
	std::map<std::string, URI::Value> query;
	const std::map<std::string, URI::Value>& getQuery() const;
	void setQuery(const std::map<std::string, URI::Value>& queryobj);
};


//HTMLģ���滻
//{{name}}	��������Ϊname
//{{#starttmp}} {{/starttmp}}		ѭ������ʼ�ͽ��� ѭ������Ϊstarttmp
class HTTP_API HTTPTemplate
{
public:
	struct TemplateObject
	{
		TemplateObject() {}
		virtual ~TemplateObject() {}

		//�����������ģ������ֵ std::map<��������,����ֵ> 
		virtual bool toTemplateData(std::map<std::string, URI::Value>& datamap) = 0;
	};
	//ѭ�����������ֵ�
	struct TemplateDirtionary
	{
		TemplateDirtionary() {}
		virtual ~TemplateDirtionary() {}
		virtual TemplateDirtionary* setValue(const std::string& key, const URI::Value&  value) = 0;
	};
public:
	HTTPTemplate(const std::string& tmpfilename);
	virtual ~HTTPTemplate();
	//����������ֵ
	HTTPTemplate& setValue(const std::string& key, const URI::Value&  value);
	//ѭ������������ѭ�� HTTPTemplate
	HTTPTemplate& setValue(const std::string& key, const std::vector<TemplateObject*>&  valuelist);
	//���ѭ������
	shared_ptr<TemplateDirtionary> addSectionDirtinary(const std::string& starttmpkey);

	std::string toValue() const;
private:
	struct HTTPTemplateInternal;
	HTTPTemplateInternal* internal;
};


class HTTP_API HTTPBuffer
{
public:
	HTTPBuffer();
	virtual ~HTTPBuffer();

	uint32_t size();

	//when end of file return ""
	int read(char* buffer, int len) const;
	int read(std::ofstream& outfile) const;
	bool readToFile(const std::string& filename) const;


	bool write(const char* buffer, int len);
	bool write(const HTTPTemplate& temp);
	bool write(std::ifstream& infile);
	bool writeFromFile(const std::string& filename,bool deleteFile);

	std::stringstream buf;
};


class HTTP_API HTTPRequest
{
public:
	HTTPRequest();
	virtual ~HTTPRequest();

	std::map<std::string, URI::Value> headers;
	virtual std::map<std::string, URI::Value> getHeaders() const;
	virtual std::string getHeader(const std::string& key) const;

	std::string method;
	virtual std::string getMethod() const;

	URL url;
	virtual const URL& getUrl() const;

	HTTPBuffer buf;
	virtual const HTTPBuffer& buffer() const;

	uint32_t timeout;
	virtual void setTimeout(uint32_t timeout_ms);
};

class HTTP_API HTTPResponse
{
public:
	HTTPResponse();
	~HTTPResponse();

	uint32_t statusCode;
	std::string errorMessage;
	bool setStatusCode(uint32_t code, const std::string& statusMessage = "");

	std::map<std::string, URI::Value> headers;
	//����ͷ�ظ���ͷ��Ϣ��Content-Length ��Ч
	bool setHeader(const std::string& key, const URI::Value& val);
	virtual std::string getHeader(const std::string& key) const;

	HTTPBuffer buf;
	virtual const HTTPBuffer& buffer() const;
};



}
}

#endif