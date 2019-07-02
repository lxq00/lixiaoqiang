//
//  Copyright (c)1998-2014, Public Technology
//  All Rights Reserved.
//
//	Description:
//	$Id: Mutex.h 3 2013-01-21 06:57:38Z  $

#ifndef __BASE_MUTEX_H__
#define __BASE_MUTEX_H__

#include "Defs.h"
#include "IntTypes.h"
namespace Public{
namespace Base{

/// \class Mutex
//// \brief 锁
class BASE_API Mutex:public IMutexInterface
{
	Mutex(Mutex const&);
	Mutex& operator=(Mutex const&);

public:
	/// 构造函数,会创建系统互斥量
	Mutex();

	/// 析构函数,会销毁系统互斥量
	~Mutex();

	/// 进入临界区
	/// \retval true 成功
	/// \retval false 失败
	bool enter();

	/// 离开临界区
	/// \retval true 成功
	/// \retval false 失败
	bool leave();

private:
	struct MutexInternal;
	MutexInternal *internal;
};

} // namespace Base
} // namespace Public

#endif //__BASE_MUTEX_H__

