//
//  Copyright (c)1998-2012, Xunmei Technology
//  All Rights Reserved.
//
//	Description:
//	$Id: Md5.h 3 2013-01-21 06:57:38Z  $
//

#ifndef __BASE_SHA1_H__
#define __BASE_SHA1_H__

#include <stddef.h>
#include "Base/IntTypes.h"
#include "Defs.h"


namespace Xunmei {
namespace Base {


/// \class Sha1 
/// \brief Sha1 �㷨������
class BASE_API Sha1
{
public:
	typedef enum
	{
		REPORT_HEX = 0,
		REPORT_DIGIT = 1,
		REPORT_HEX_SHORT = 2
	}REPORT_TYPE;
public:
	Sha1();
	~Sha1();

	bool hashFile(const std::string& file);

	///	���ݱ��롣
	///	\param [in] data ����ָ��
	/// \param [in] size ���ݳ���
	bool input(uint8_t const* data, size_t size);

	///����һ�ж��źã�����hashֵ
	bool Final();

	/// ��ȡ���
	/// \param [in] result 40λ��������
	bool result(uint32_t ret[5]);

	///��ʽ�����ֵ
	bool report(std::string& str,REPORT_TYPE type);
private:
	class Sha1Internal;
	Sha1Internal* internal;
};


} // namespace Base
} // namespace Xunmei

#endif// __BASE_SHA1_H__


