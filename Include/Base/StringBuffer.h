//
//  Copyright (c)1998-2014, Public Technology
//  All Rights Reserved.
//
//
//	Description:
//	$Id: String.h 227 2013-10-30 01:10:30Z  $

#ifndef __BASE_STRINGBUFFER_H__
#define __BASE_STRINGBUFFER_H__

#include "String.h"


namespace Public {
namespace Base {

//StringBuffer�����ڲ���������ָ��
class BASE_API StringBuffer
{
	struct StringBufferInternal;
	StringBufferInternal* internal;
public:
	//���캯��
	StringBuffer();
	StringBuffer(const char* str, const shared_ptr<IMempoolInterface>& mempool = shared_ptr<IMempoolInterface>());
	StringBuffer(const char* str,size_t size, const shared_ptr<IMempoolInterface>& mempool = shared_ptr<IMempoolInterface>());
	StringBuffer(const std::string& str, const shared_ptr<IMempoolInterface>& mempool = shared_ptr<IMempoolInterface>());
	StringBuffer(const String& dataobj,const char* str = NULL,size_t size = 0);
	StringBuffer(const StringBuffer& buffer,const char* str = NULL, size_t size = 0);

	~StringBuffer();

	//�������ݶ�������
	const char* buffer() const;
	size_t size() const;
	const String& dataobj() const;

	//����������
	StringBuffer& operator = (const char* str);
	StringBuffer& operator = (const std::string& str);
	StringBuffer& operator = (const String& str);
	StringBuffer& operator = (const StringBuffer& buffer);

	//׷������
	void set(const char* str, const shared_ptr<IMempoolInterface>& mempool = shared_ptr<IMempoolInterface>());
	void set(const char* str,size_t size, const shared_ptr<IMempoolInterface>& mempool = shared_ptr<IMempoolInterface>());
	void set(const std::string& str, const shared_ptr<IMempoolInterface>& mempool = shared_ptr<IMempoolInterface>());
	void set(const String& strtmp, const char* str = NULL, size_t size = 0);
	void set(const StringBuffer& buffer, const char* str = NULL, size_t size = 0);
};


} // namespace Base
} // namespace Public

#endif// __BASE_STRING_H__


