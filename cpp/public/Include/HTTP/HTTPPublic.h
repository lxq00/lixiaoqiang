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


//HTML模板替换
//{{name}}	变量名称为name
//{{#starttmp}} {{/starttmp}}		循环的起始和结束 循环名称为starttmp
class HTTP_API HTTPTemplate
{
public:
	struct TemplateObject
	{
		TemplateObject() {}
		virtual ~TemplateObject() {}

		//将对象解析成模板所需值 std::map<变量名称,变量值> 
		virtual bool toTemplateData(std::map<std::string, URI::Value>& datamap) = 0;
	};
	//循环变量处理字典
	struct TemplateDirtionary
	{
		TemplateDirtionary() {}
		virtual ~TemplateDirtionary() {}
		virtual TemplateDirtionary* setValue(const std::string& key, const URI::Value&  value) = 0;
	};
public:
	HTTPTemplate(const std::string& tmpfilename);
	virtual ~HTTPTemplate();
	//更换变量到值
	HTTPTemplate& setValue(const std::string& key, const URI::Value&  value);
	//循环更换变量，循环 HTTPTemplate
	HTTPTemplate& setValue(const std::string& key, const std::vector<TemplateObject*>&  valuelist);
	//添加循环变量
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
	std::string read() const;
	bool readToFile(const std::string& filename) const;


	bool write(const char* buffer, int len);
	bool write(const std::string& buffer);
	bool write(const HTTPTemplate& temp);
	bool writeFromFile(const std::string& filename,bool deleteFile);
private:
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

	URI url;
	virtual const URI& getUrl() const;

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
	//设置头回复包头信息，Content-Length 无效
	bool setHeader(const std::string& key, const URI::Value& val);
	virtual std::string getHeader(const std::string& key) const;

	HTTPBuffer buf;
	virtual const HTTPBuffer& buffer() const;
};



}
}

#endif