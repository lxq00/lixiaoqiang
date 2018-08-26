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


class HTTP_API HTTPBuffer
{
public:
	typedef enum {
		HTTPBufferType_String,
		HTTPBufferType_File,
		HTTPBufferType_Stream
	}HTTPBufferType;
	typedef Function3<void, HTTPBuffer*, const char*, int> StreamCallback;
public:
	HTTPBuffer(HTTPBufferType type = HTTPBufferType_String);
	virtual ~HTTPBuffer();

	//-1 not support
	int size();

	//when end of file return ""
	HTTPBufferType readtype();
	std::string read(int start, int maxlen);
	std::string read();
	bool readToFile(const std::string& filename, bool deleteFile = false);
	void setReadCallback(const StreamCallback& callback);


	HTTPBufferType writetype();
	void writetype(HTTPBufferType type);
	bool write(const char* buffer, int len);
	bool write(const std::string& buffer);
	/*bool write(const HTTPTemplate& temp);*/
	bool writeFromFile(const std::string& filename,bool deleteFile);
private:
	struct HTTPBufferInternal;
	HTTPBufferInternal* internal;
};


class HTTP_API HTTPRequest
{
public:
	typedef Function2<void, HTTPRequest*, const std::string&> DisconnectCallback;
public:
	HTTPRequest(HTTPBuffer::HTTPBufferType type = HTTPBuffer::HTTPBufferType_String);
	virtual ~HTTPRequest();

	std::map<std::string, Value>& headers();
	Value header(const std::string& key);

	std::string& method();

	URL& url();

	shared_ptr<HTTPBuffer>& content();

	uint32_t& timeout();

	NetAddr& remoteAddr();

	NetAddr& myAddr();

	DisconnectCallback&	discallback();

	virtual bool push();
private:
	struct  HTTPRequestInternal;
	HTTPRequestInternal* internal;
};

class HTTP_API HTTPResponse
{
public:
	HTTPResponse(HTTPBuffer::HTTPBufferType type = HTTPBuffer::HTTPBufferType_String);
	virtual ~HTTPResponse();

	uint32_t& statusCode();
	std::string& errorMessage();

	std::map<std::string, Value>& headers();
	Value header(const std::string& key);

	shared_ptr<HTTPBuffer>& content();

	virtual bool push();
private:
	struct HTTPResponseInternal;
	HTTPResponseInternal* internal;
};

typedef Function2<void, const shared_ptr<HTTPRequest>&, shared_ptr<HTTPResponse>&> HTTPCallback;

}
}

#endif