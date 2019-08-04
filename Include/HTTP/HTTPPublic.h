#ifndef __HTTPPUBLIC_H__
#define __HTTPPUBLIC_H__

#include "Defs.h"
#include "Base/Base.h"
#include "Network/Network.h"
using namespace Public::Base;
using namespace Public::Network;

namespace Public {
namespace HTTP {

//
////HTMLģ���滻
////{{name}}	��������Ϊname
////{{#starttmp}} {{/starttmp}}		ѭ������ʼ�ͽ��� ѭ������Ϊstarttmp
//class HTTP_API HTTPTemplate
//{
//public:
//	struct TemplateObject
//	{
//		TemplateObject() {}
//		virtual ~TemplateObject() {}
//
//		//�����������ģ������ֵ std::map<��������,����ֵ> 
//		virtual bool toTemplateData(std::map<std::string, Value>& datamap) = 0;
//	};
//	//ѭ�����������ֵ�
//	struct TemplateDirtionary
//	{
//		TemplateDirtionary() {}
//		virtual ~TemplateDirtionary() {}
//		virtual TemplateDirtionary* setValue(const std::string& key, const Value&  value) = 0;
//	};
//public:
//	HTTPTemplate(const std::string& tmpfilename);
//	virtual ~HTTPTemplate();
//	//����������ֵ
//	HTTPTemplate& setValue(const std::string& key, const Value&  value);
//	//ѭ������������ѭ�� HTTPTemplate
//	HTTPTemplate& setValue(const std::string& key, const std::vector<TemplateObject*>&  valuelist);
//	//���ѭ������
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

	virtual uint32_t append(const char* buffer, uint32_t len) = 0;
	virtual void read(String& data) = 0;
};

class HTTP_API ReadContent :public IContent
{
public:
	ReadContent(HTTPCacheType type,const std::string& filename = "");
	~ReadContent();

	int size() const;

	std::string cacheFileName() const;
	int read(char* buffer, int maxlen) const;
	std::string read() const;
	bool readToFile(const std::string& filename) const;
private:
	virtual uint32_t append(const char* buffer, uint32_t len);
	virtual void read(String& data);
private:
	struct ReadContentInternal;
	ReadContentInternal* internal;
};

class HTTP_API WriteContenNotify
{
public:
	WriteContenNotify() {}
	virtual ~WriteContenNotify() {}

	virtual void WriteNotify() = 0;
	virtual void setWriteFileName(const std::string& filename) = 0;
};

class HTTP_API WriteContent :public IContent
{
public:
	WriteContent(WriteContenNotify* notify, HTTPCacheType type);
	~WriteContent();

	bool setChunkEOF();
	bool write(const char* buffer, int len);
	bool write(const std::string& buffer);
	bool writeFromFile(const std::string& filename, bool needdeletefile = false);
private:
	virtual uint32_t append(const char* buffer, uint32_t len);
	virtual void read(String& data);
private:
	struct WriteContentInternal;
	WriteContentInternal* internal;
};

}
}

#endif