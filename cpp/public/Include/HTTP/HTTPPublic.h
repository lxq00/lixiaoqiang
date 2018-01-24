#ifndef __HTTPPUBLIC_H__
#define __HTTPPUBLIC_H__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <vector>

#ifndef WIN32
#include<string>
#endif

#include "Defs.h"
#include "Base/Base.h"
using namespace Xunmei::Base;

namespace Xunmei {
namespace HTTP {


//举例 http://user:pass@host.com:8080/p/a/t/h?query=string#hash
class HTTP_API URL
{
public:
	URL(const std::string& urlstr);
	URL(const URL& url);
	~URL();

	//http://user:pass@host.com:8080/p/a/t/h?query=string#hash
	std::string href() const;

	//http
	std::string protocol() const;

	//user:pass
	std::string auhen() const;

	//host.com:8080
	std::string host() const;
	//host.com
	std::string hostname() const;
	//8080
	uint32_t port() const;

	///p/a/t/h?query=string#hash
	std::string path() const;
	//p/a/t/h
	std::string pathname() const;
	//?query=string#hash
	std::string search() const;

	//<query,string#assh>
	std::map<std::string, std::string> query() const;
private:
	struct URLInternal;
	URLInternal* internal;
};


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
	~HTTPTemplate();
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


typedef enum
{
	HTTPBufferCacheType_Mem,
	HTTPBufferCacheType_File,
}HTTPBufferCacheType;

class HTTP_API HTTPBuffer
{
public:
	HTTPBuffer(HTTPBufferCacheType type);
	~HTTPBuffer();

	//when end of file return ""
	int read(char* buffer, int len) const;
	int read(std::string& data) const;
	std::string read() const;
	uint64_t totalSize() const;
	bool readToFile(const std::string& filename) const;

	bool write(const char* buffer, int len);
	bool write(const std::string& data);
	bool write(const HTTPTemplate& temp);
	bool writeFromFile(const std::string& filename,bool deleteFile);
private:
	struct HTTPBufferInternal;
	HTTPBufferInternal *internal;
};


}
}

#endif