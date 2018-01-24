//
//  Copyright (c)1998-2014, Xunmei Technology
//  All Rights Reserved.
//
//	Description:
//	$Id: RecursiveMutex.h 3 2013-01-21 06:57:38Z jiangwei $

#ifndef __BASE_RECURSIVE_MUTEX_H__
#define __BASE_RECURSIVE_MUTEX_H__

#include "Defs.h"
#include "IntTypes.h"

namespace Xunmei {
namespace Base {

/// \class RecursiveMutex �ݹ���
class BASE_API RecursiveMutex:public ILockerObjcPtr
{
	RecursiveMutex(RecursiveMutex const&);
	RecursiveMutex& operator=(RecursiveMutex const&);

public:
	/// ���캯��,�ᴴ��ϵͳ������
	RecursiveMutex();

	/// ��������,������ϵͳ������
	~RecursiveMutex();

	/// �����ٽ���.
	/// \retval true �ɹ�
	/// \retval false ʧ��
	bool enter();

	/// �뿪�ٽ���.
	/// \retval true �ɹ�
	/// \retval false ʧ��
	bool leave();

private:
	struct RecursiveMutexInternal;
	RecursiveMutexInternal *internal;
};

} // namespace Base
} // namespace Xunmei

#endif //__BASE_RECURSIVE_MUTEX_H__


