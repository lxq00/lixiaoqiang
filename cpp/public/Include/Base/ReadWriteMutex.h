//
//  Copyright (c)1998-2014, Xunmei Technology
//  All Rights Reserved.
//
//	Description:
//	$Id: ReadWriteMutex.h 3 2013-01-21 06:57:38Z jiangwei $

#ifndef __BASE_RW_MUTEX_H__
#define __BASE_RW_MUTEX_H__

#include "Defs.h"
#include "IntTypes.h"

namespace Xunmei{
namespace Base{


/// \class Mutex
/// \brief 多平台互斥量类
class BASE_API ReadWriteMutex:public ILockerObjcPtr
{
	ReadWriteMutex(ReadWriteMutex const&);
	ReadWriteMutex& operator=(ReadWriteMutex const&);

public:
	/// 构造函数，会创建系统互斥量
	ReadWriteMutex();

	/// 析构函数，会销毁系统互斥量
	~ReadWriteMutex();

	/// 进入临界区。
	/// \retval true 	成功
	/// \retval false 失败
	bool enterread();

	/// 进入临界区。
	/// \retval true 	成功
	/// \retval false 失败
	bool enterwrite();

	/// 离开临界区。
	/// \retval true 	成功
	/// \retval false 失败
	bool leave();

private:
	struct ReadWriteMutexInternal;
	ReadWriteMutexInternal* internal;
};

} // namespace Base
} // namespace Xunmei

#endif //__BASE_RW_MUTEX_H__

