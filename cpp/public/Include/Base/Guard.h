//
//  Copyright (c)1998-2014, Public Technology
//  All Rights Reserved.
//
//	Description:
//	$Id: Guard.h 3 2013-01-21 06:57:38Z jiangwei $
//


#ifndef __BASE_GUARD_H__
#define __BASE_GUARD_H__

#include "Mutex.h"
#include "RecursiveMutex.h"
#include "ReadWriteMutex.h"
#include "Shared_ptr.h"
namespace Public{
namespace Base{


/// \class Guard
/// \brief 守护者类.

class Guard
{
	Guard(Guard const&);
	Guard& operator=(Guard const&);

public:
	/// 构造函数,自动调用CMutex::enter
	/// \param mutex 互斥量引用,CGuard并不创建互斥量.
	inline Guard(Mutex& mutexparam)
		:mutex(&mutexparam)
	{
		if(mutex != NULL)
		{
			mutex->enter();
		}		
	};
	inline Guard(const shared_ptr<ILockerObjcPtr>& ptr)
		:mutex(ptr.get())
	{
		if (mutex != NULL)
		{
			mutex->enter();
		}
	};
	inline Guard(ILockerObjcPtr* ptr)
		:mutex(ptr)
	{
		if(mutex != NULL)
		{
			mutex->enter();
		}		
	};

	/// 析构函数,自动调用CMutex::leave
	inline ~Guard()
	{
		if(mutex != NULL)
		{
			mutex->leave();
		}
	};

private:
	ILockerObjcPtr *mutex;	///< 需要自动调用的互斥量引用
};

////////////////////////////////////////////////////////////////////////////////
/// \class RecursiveGuard
/// \brief 递归守护者类
class RecursiveGuard
{
	RecursiveGuard(RecursiveGuard const&);
	RecursiveGuard& operator=(RecursiveGuard const&);

public:
	/// 构造函数,自动调用CMutex::enter
	/// \param mutex [in] 互斥量引用,CGuard并不创建互斥量.
	inline RecursiveGuard(RecursiveMutex& mutexparam)
		:mutex(mutexparam)
	{
		mutex.enter();
	};

	/// 析构函数,自动调用CMutex::leave
	inline ~RecursiveGuard()
	{
		mutex.leave();
	};

private:
	RecursiveMutex &mutex;	///< 需要自动调用的互斥量引用
};

////////////////////////////////////////////////////////////////////////////////
/// \class GuardReadMutex
/// \brief 读锁守护者类
class GuardReadMutex
{
	GuardReadMutex(GuardReadMutex const&);
	GuardReadMutex& operator=(GuardReadMutex const&);

public:
	/// 构造函数,自动调用 CRwMutex::EnterReading
	/// \param mutex [in]互斥量引用,CGuard并不创建互斥量.
	inline GuardReadMutex(ReadWriteMutex& mutexparam)
		: mutex(mutexparam)
	{
		mutex.enterread();
	};

	/// 析构函数,自动调用 CReadWriteMutex::leave
	inline ~GuardReadMutex()
	{
		mutex.leave();
	};

private:
	ReadWriteMutex& mutex;	///< 需要自动调用的互斥量引用
};

////////////////////////////////////////////////////////////////////////////////
/// \class GuardReadMutex
/// \brief 读写锁守护者类
class GuardWriteMutex
{
	GuardWriteMutex(GuardWriteMutex const&);
	GuardWriteMutex& operator=(GuardWriteMutex const&);

public:
	/// 构造函数,自动调用 CReadWriteMutex::EnterWriting
	/// \param mutex [in]互斥量引用,CGuard并不创建互斥量.
	inline GuardWriteMutex(ReadWriteMutex& mutexparam)
		: mutex(mutexparam)
	{
		mutex.enterwrite();
	};

	/// 析构函数,自动调用 CReadWriteMutex::leave
	inline ~GuardWriteMutex()
	{
		mutex.leave();
	};

private:
	ReadWriteMutex& mutex;	///< 需要自动调用的互斥量引用
};

} // namespace Base
} // namespace Public

#endif //__BASE_GUARD_H__

