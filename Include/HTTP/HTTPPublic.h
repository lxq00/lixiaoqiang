#ifndef __HTTPPUBLIC_H__
#define __HTTPPUBLIC_H__

#include "Defs.h"
#include "Base/Base.h"
#include "Network/Network.h"
#include "HTTP/HTTPParse.h"
using namespace Public::Base;
using namespace Public::Network;

namespace Public {
namespace HTTP {

//
////HTML模板替换
////{{name}}	变量名称为name
////{{#starttmp}} {{/starttmp}}		循环的起始和结束 循环名称为starttmp
//class HTTP_API HTTPTemplate
//{
//public:
//	struct TemplateObject
//	{
//		TemplateObject() {}
//		virtual ~TemplateObject() {}
//
//		//将对象解析成模板所需值 std::map<变量名称,变量值> 
//		virtual bool toTemplateData(std::map<std::string, Value>& datamap) = 0;
//	};
//	//循环变量处理字典
//	struct TemplateDirtionary
//	{
//		TemplateDirtionary() {}
//		virtual ~TemplateDirtionary() {}
//		virtual TemplateDirtionary* setValue(const std::string& key, const Value&  value) = 0;
//	};
//public:
//	HTTPTemplate(const std::string& tmpfilename);
//	virtual ~HTTPTemplate();
//	//更换变量到值
//	HTTPTemplate& setValue(const std::string& key, const Value&  value);
//	//循环更换变量，循环 HTTPTemplate
//	HTTPTemplate& setValue(const std::string& key, const std::vector<TemplateObject*>&  valuelist);
//	//添加循环变量
//	shared_ptr<TemplateDirtionary> addSectionDirtinary(const std::string& starttmpkey);
//
//	std::string toValue() const;
//private:
//	struct HTTPTemplateInternal;
//	HTTPTemplateInternal* internal;
//};


typedef enum {
	WebSocketDataType_Txt,
	WebSocketDataType_Bin,
}WebSocketDataType;

typedef enum {
	HTTPCacheType_Mem = 0,
	HTTPCacheType_File,
}HTTPCacheType;

class HTTPCommunication;

class HTTP_API IContent
{
public:
	IContent() {}
	virtual ~IContent() {}

	virtual uint32_t size()= 0;
	virtual uint32_t append(const char* buffer, uint32_t len) = 0;
	virtual void read(String& data) = 0;
};

class HTTP_API WriteContenNotify
{
public:
	WriteContenNotify() {}
	virtual ~WriteContenNotify() {}

	virtual void WriteNotify() = 0;
	virtual void ReadReady() = 0;
};

class HTTP_API ChunkData
{
public:
	typedef Function2<bool, const char*, int> ReadCallback;
	typedef Function2<bool, const char*, int> WriteCallback;

	ChunkData();
	ChunkData(const WriteCallback& writecallback);
	~ChunkData();

	void setReadCallback(const ReadCallback& readcallback);

	uint32_t append(const char* buffer, uint32_t len);
	void write(const char* buffer, uint32_t len);
private:
	struct ChunkDataInternal;
	ChunkDataInternal* internal;
};

class HTTP_API ReadContent :public IContent
{
public:
	typedef Function2<void, const char*, uint32_t> DataCalback;
public:
	ReadContent(const shared_ptr<HTTPHeader>& header, WriteContenNotify* notify, HTTPCacheType type,const std::string& filename = "");
	~ReadContent();

	uint32_t size();

	std::string cacheFileName() const;
	int read(char* buffer, int maxlen) const;
	std::string read() const;
	bool readToFile(const std::string& filename) const;

	void setDataCallback(const DataCalback& callback);
private:
	virtual uint32_t append(const char* buffer, uint32_t len);
	virtual void read(String& data);
private:
	struct ReadContentInternal;
	ReadContentInternal* internal;
};

class HTTP_API WriteContent :public IContent
{
public:
	WriteContent(const shared_ptr<HTTPHeader>& header, WriteContenNotify* notify, HTTPCacheType type);
	~WriteContent();

	bool write(const char* buffer, int len);
	bool writeString(const std::string& buffer);
	bool writeFromFile(const std::string& filename, bool needdeletefile = false);

	void writeChunk(const char* buffer, uint32_t len);
	void writeChunkEnd();
private:
	virtual uint32_t size();
	virtual uint32_t append(const char* buffer, uint32_t len);
	virtual void read(String& data);
private:
	struct WriteContentInternal;
	WriteContentInternal* internal;
};


}
}

#endif