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

class FileMediaInfo;

class HTTP_API HTTPContent
{
public:
	typedef enum {
		HTTPContentType_Normal = 0,
		HTTPContentType_Chunk,
	}HTTPContentType;
	typedef Function0<bool> CheckConnectionIsOk;
	typedef Function2<void, const char*, uint32_t> ReadDataCallback;
	typedef Function2<uint32_t, char*, uint32_t> WriteDataCallback;
public:
	HTTPContent(FileMediaInfo* info,const CheckConnectionIsOk& check = CheckConnectionIsOk());
	virtual ~HTTPContent();

	int size();
	
	//when end of file return ""
	bool setReadToFile(const std::string& filename,bool deletefile = false);
	std::string cacheFileName() const;
	int read(char* buffer, int maxlen);
	std::string read();
	bool readToFile(const std::string& filename);
	bool setReadCallback(const ReadDataCallback& callback);


	HTTPContentType& writetype();
	bool setChunkEOF();
	bool write(const char* buffer, int len);
	bool write(const std::string& buffer);
	/*bool write(const HTTPTemplate& temp);*/
	//writeFromFile chanegd writetype to HTTPContentType_Normal 
	bool writeFromFile(const std::string& filename,bool needdeletefile = false);
	bool setWriteCallback(const WriteDataCallback& writecallback);

	const char* inputAndParse(const char* buffer, int len,bool chunked,bool& chunedfinish);
private:
	struct HTTPContentInternal;
	HTTPContentInternal* internal;
};


class HTTP_API HTTPRequest
{
public:
	typedef Function2<void, HTTPRequest*, const std::string&> DisconnectCallback;
public:
	HTTPRequest();
	virtual ~HTTPRequest();

	std::map<std::string, Value>& headers();
	Value header(const std::string& key);

	std::string& method();

	URL& url();

	shared_ptr<HTTPContent>& content();

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
	typedef Function2<void, HTTPResponse*, const std::string&> DisconnectCallback;
public:
	HTTPResponse(const HTTPContent::CheckConnectionIsOk& check = HTTPContent::CheckConnectionIsOk());
	virtual ~HTTPResponse();

	uint32_t& statusCode();
	std::string& errorMessage();

	std::map<std::string, Value>& headers();
	Value header(const std::string& key);

	shared_ptr<HTTPContent>& content();

	virtual bool push();

	DisconnectCallback&	discallback();
private:
	struct HTTPResponseInternal;
	HTTPResponseInternal* internal;
};


typedef enum {
	WebSocketDataType_Txt,
	WebSocketDataType_Bin,
}WebSocketDataType;


}
}

#endif