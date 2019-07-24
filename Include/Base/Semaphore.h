//
//  Copyright (c)1998-2014, Public Technology
//  All Rights Reserved.
//
//	Description:
//	$Id: Semaphore.h 33 2013-02-05 09:43:35Z  $


#ifndef __BASE_SEMAPHORE_H__
#define __BASE_SEMAPHORE_H__

#include "Defs.h"
#include "Base/IntTypes.h"

namespace Public{
namespace Base{
/// \class Semaphore
/// \brief 多平台信号量类
class BASE_API Semaphore:public ISemaphoreInterface
{
	Semaphore(Semaphore const&);
	Semaphore& operator=(Semaphore const&);

public:
	/// 构造函数，会创建系统信号量
	/// \param initialCount [in] 信号量初始计数
	explicit Semaphore(int initialCount = 0);

	/// 析构函数，会销毁系统互斥量
	~Semaphore();

	/// 如果已经减少到0，会阻塞调用的线程
	/// \return >=0 成功
	///         <0 出错
	int pend();

	/// 减少信号量计数，如果是从0累加，会唤醒其等待队列的第一个线程
	/// \timeout [in] 超时时间,单位毫秒
	/// \return >=0 成功
	///         <0  出错或者超时
	int pend(uint32_t timeout);
	
	/// 增加信号量计数，如果是从0累加，会唤醒其等待队列的第一个线程
	/// \return 当前信号量计数
	int post();

private:
	struct SemaphoreInternal;
	SemaphoreInternal* internal;
};

} // namespace Base
} // namespace Public

#endif //__BASE_SEMAPHORE_H__

