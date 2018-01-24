//
//  Copyright (c)1998-2014, Public Technology
//  All Rights Reserved.
//
//
//	Description:
//	$Id: String.h 227 2013-10-30 01:10:30Z linyang $

#ifndef __BASE_STRING_H__
#define __BASE_STRING_H__

#include <stdio.h>
#include <string.h>
#include "BaseTemplate.h"
#include "Defs.h"
#include <string>
#include "Func.h"

inline void stringToLower(const char* strcstr,char* deststr)
{
	while(*strcstr)
	{
		*deststr = tolower(*strcstr);
		strcstr++;
		deststr++;
	}
	*deststr = 0;
}

inline void stringToUpper(const char* strcstr, char* deststr)
{
	while (*strcstr)
	{
		*deststr = toupper(*strcstr);
		strcstr++;
		deststr++;
	}
	*deststr = 0;
}

inline std::string stringToLowerEx(const std::string& srcstring)
{
	std::string dststring = srcstring;
	
	stringToLower(srcstring.c_str(), (char*)dststring.c_str());

	return dststring;
}

inline std::string stringToUpperEx(const std::string& srcstring)
{
	std::string dststring = srcstring;

	stringToUpper(srcstring.c_str(), (char*)dststring.c_str());

	return dststring;
}

#ifdef WIN32
#define strcasecmp _stricmp
#define snprintf _snprintf
#define strncasecmp _strnicmp
inline const char* strcasestr(const char* srcstr,const char* substr)
{
	if(srcstr == NULL || substr == NULL) 
	{
		return NULL;
	}
	char* srcptr = new(std::nothrow) char[strlen(srcstr) + 1];
	char* subptr = new(std::nothrow) char[strlen(substr) + 1];
	if(srcptr == NULL || subptr == NULL) 
	{
		Public::Base::SAFE_DELETEARRAY(srcptr);
		Public::Base::SAFE_DELETEARRAY(subptr); 
		return NULL;
	}
	stringToLower(srcstr,srcptr);
	stringToLower(substr,subptr);

	const char* ptmp = strstr(srcptr,subptr);
	if(ptmp != NULL)
	{
		ptmp = srcstr + (ptmp - srcptr);
	}
	Public::Base::SAFE_DELETEARRAY(srcptr);
	Public::Base::SAFE_DELETEARRAY(subptr);
	return ptmp;
}
#endif


namespace Public {
namespace Base {


///	��ȫ���ַ�������
///	\param [out] dest:Ŀ��buffer,���Ŀ��bufferû��'\0'�������������һ���ַ�Ϊ'\0',����0
///	\param [in]	dstBufLen,Ŀ��buffer�ռ��С,�ú������д��dstBufLen-1���ַ���������д���ַ��������'\0'�ַ�
///	\param [in] src:
///	\param [in] srcCopyCount: ����src�ĳ���
///		ִ����ɺ������copy����ô��Destһ��������0����
///	\retval ���صĽ���Ǵ�src copy��dest�е��ַ���Ŀ
size_t BASE_API strncat(char* dest, size_t dstBufLen, const char* src, const size_t srcCopyCount);

///	��ȫ���ַ���copy����
///	\param [out] dst,Ŀ��buffer
///	\param [in]	dstBufLen,Ŀ��buffer�ռ��С,�ú������д��dstBufLen-1���ַ���������д���ַ��������'\0'�ַ�
///	\param [in]	src,Դbuffer
///	\param [in] srcCopyCount
///	\retval Ҫcopy���ַ�����,��dstBufLen-1�ռ����������£����copy���ַ���ĿΪsrcCopyCount,�����ں������'\0'�ַ�
size_t BASE_API strncpy(char* dst, size_t dstBufLen, const char* src, size_t srcCopyCount);

///	��ǿ��snprintf����֤'\0'������ʵ��д�볤�ȣ�
///	����֧�� len += snprintf_x( buf + len, maxlen - len, "xxxx", ... ); ������д��
///	��ʵ��buffer����ʱ����֤\'0'������maxlen - 1����ԭ��snprintf��VC�᷵��-1�Ҳ���֤'\0'��gcc�᷵�ؼ���buffer�㹻ʱ��д�볤�ȣ�
///	��������maxlen-1ʱ�޷����ֳ��ȸոպû��ǳ����ˣ����Լ򻯶������������߶�������������
///	Ҳ��������Ҫ�޳����ұ�֤'\0'ʱ���ַ���������ȡ��strncpy����ע��ԭ��strncpy����֤'\0'��
///	�� strncpy( dst, src, siz - 1 ); dst[size - 1] = '\0'; �൱�� snprintf( dst, siz, "%s", src );
///	\param [out] buf �������
///	\param [in] maxlen �����������ֽ���
///	\param [in] fmt ��ʽ�ַ���
///	\retval ����ʵ��д�볤��
int  BASE_API snprintf_x(char* buf, int maxlen, const char* fmt, ... );

/// Ansi(Gb2312)ת���ַ���Ϊutf8��ʽ
/// \param [in] inbuf Դ�ַ���
/// \param [in]  inlen Դ�ַ����ĳ���
/// \param [out] outbuf Ŀ���ַ���
/// \param [in] outlen �洢Ŀ����ַ������ֵ
/// \retval > 0ת����ĳ���, 
///         <= 0 ����
int BASE_API ansi2utf8(char *inbuf,size_t inlen,char *outbuf,size_t outlen);


/// utf8ת���ַ���Ϊ Ansi(Gb2312)��ʽ
/// \param [in] inbuf Դ�ַ���
/// \param [in]  inlen Դ�ַ����ĳ���
/// \param [out] outbuf Ŀ���ַ���
/// \param [in] outlen �洢Ŀ����ַ������ֵ
/// \retval > 0ת����ĳ���, 
///         <= 0 ����
int BASE_API utf82ansi(char *inbuf,size_t inlen,char *outbuf,size_t outlen);

/// Ansi(Gb2312) ת���ַ���Ϊutf8��ʽ ��չ�ӿ�(linux ֻ֧��ת������������1024bytes)
/// \param [in] inbuf Դ�ַ���
/// \param [in]  inlen Դ�ַ����ĳ���
/// \retval ��string ת��ʧ��
///         �ǿ�string ת�����
std::string BASE_API ansi2utf8Ex(char *inbuf,size_t inlen);


/// utf8ת���ַ���Ϊ Ansi(Gb2312)��ʽ  ��չ�ӿ�(linux ֻ֧��ת������������1024bytes)
/// \param [in] inbuf Դ�ַ���
/// \param [in]  inlen Դ�ַ����ĳ���
/// \retval ��string ת��ʧ��
///         �ǿ�string ת�����
std::string BASE_API utf82ansiEx(char *inbuf,size_t inlen);
/// ͨ����ʶ������url�ֶ�
/// \param [in] url Ҫ������url��
/// \param [in] key �������ֶ�����
/// \param [in] headSymbol �������ֶ������ײ��ָ��
/// \param [in] tailSymbol �������ֶ�����β���ָ��
/// \param [out] value ������Ľ������ֵ
bool BASE_API parseUrlByKey(std::string &url, char* key, char headSymbol,  char tailSymbol, std::string & value);


std::string BASE_API ansi2utf8Ex(const std::string& inString);

std::string BASE_API utf82ansiEx(const std::string& inString);

///Function3<std::string,const std::string& inputstr,const std::string& fromcharset,const std::string& tocharset>EncodeCallback
typedef Function3<std::string,const std::string&,const std::string&,const std::string& > CharsetEncodeCallback;

void BASE_API registeCharsetEncode(const CharsetEncodeCallback& callback);



} // namespace Base
} // namespace Public

#endif// __BASE_STRING_H__


