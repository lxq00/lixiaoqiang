//
//  Copyright (c)1998-2014, Public Technology
//  All Rights Reserved.
//
//	Description:
//	$Id: RecursiveMutex.h 3 2013-01-21 06:57:38Z  $

#ifndef __BASE_RECURSIVE_MUTEX_H__
#define __BASE_RECURSIVE_MUTEX_H__

#include "Defs.h"
#include "IntTypes.h"

namespace Public {
namespace Base {

/// \class RecursiveMutex 递归锁
class BASE_API RecursiveMutex:public IMutexInterface
{
	RecursiveMutex(RecursiveMutex const&);
	RecursiveMutex& operator=(RecursiveMutex const&);

public:
	/// 构造函数,会创建系统互斥量
	RecursiveMutex();

	/// 析构函数,会销毁系统互斥量
	~RecursiveMutex();

	/// 进入临界区.
	/// \retval true 成功
	/// \retval false 失败
	bool enter();

	/// 离开临界区.
	/// \retval true 成功
	/// \retval false 失败
	bool leave();

private:
	struct RecursiveMutexInternal;
	RecursiveMutexInternal *internal;
};

} // namespace Base
} // namespace Public

#endif //__BASE_RECURSIVE_MUTEX_H__


