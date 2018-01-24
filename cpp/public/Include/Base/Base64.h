//
//  Copyright (c)1998-2012, Public Technology
//  All Rights Reserved.
//
//	Description:
//	$Id: Base64.h 3 2013-01-21 06:57:38Z jiangwei $

#ifndef __BASE_BASE64_H__
#define __BASE_BASE64_H__

#include "Defs.h"

namespace Public {
namespace Base {

/// \class Base64
/// \brief Base64�㷨��صķ�����
class BASE_API Base64 	
{
public:
	/// �����ű�������ݵĻ������ֽ���
	static int encodelen(int len);

	/// Base64 ����
	/// \param [in] dst ��ű������ı�����
	/// \param [in] src ����ǰ�����ݻ���
	/// \param [in] len ����ǰ�������ֽ���
	/// \reval ���ر������ı��ֽ���, �������ַ���������
	static int encode(char* dst, const char* src, int len);

	/// �����Ž�������ݵĻ������ֽ���
	/// \param src [in] ��Ҫ���������Դָ��
	/// \reval �����Ļ������ݴ�С
	static int decodelen(const char* src);

	/// Base64 ����
	/// \param [in] dst ����ǰ���ı�����
	/// \param [in] src ���������ݻ���
	/// \reval ���ؽ����������ֽ���
	static int decode(char* dst, const char* src);
};

} // namespace Base
} // namespace Public

#endif// __BASE_BASE64_H__

