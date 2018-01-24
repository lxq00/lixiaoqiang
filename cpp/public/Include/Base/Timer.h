//
//  Copyright (c)1998-2012, Xunmei Technology
//  All Rights Reserved.
//
//
//	Description:
//	$Id: Timer.h 3 2013-01-21 06:57:38Z jiangwei $


#ifndef _BASE_TIMER_H_
#define _BASE_TIMER_H_

#include "Base/IntTypes.h"
#include "Defs.h"
#include "Func.h"

namespace Xunmei{
namespace Base{


/// \class Timer
/// \brief 多平台定时器，支持延时、周期、异步等多种过程执行方式

struct TimerInternal;

class BASE_API Timer
{
	/// 禁止拷贝构造函数和赋值操作
	Timer(Timer const&);
	Timer& operator=(Timer const&);

public:
	typedef Function1<void, unsigned long> Proc;

	/// 构造函数
	/// \param name [in] 定时器名称
	Timer(const char * name);

	/// 析构函数
	virtual ~Timer();

	/// 启动定时器
	/// \param fun 		[in]	定时器回调函数
	/// \param delay 	[in]	指定启动后延时多少时间调用,单位为毫秒,如果为0表示立即开始调用
	/// \param period [in]	定时器的周期,指定距上次调用多少时间后再次调用,单位为毫秒,
	///		   如果为0表示是非周期定时器,第一次调用完毕后会自动停止
	/// \param param 	[in]	回调函数参数,在回调被触发是会传给回调函数
	/// \param timeout 回调函数执行的超时时间,这个时间会被设置给回调时的定时器线程,
	///        毫秒为单位. 0表示永不超时,默认值为1分钟也就是60秒
	/// \reval false 在非延时的定时器的上次调用还没有完成的情况下
	/// \reval true  其他情况
	/// \note 把延时参数和周期参数都设置为0,即可实现异步调用
	bool start(const Proc& fun, uint32_t delay, uint32_t period, unsigned long param = 0, uint32_t timeout = 60000);

	/// 关闭定时器
	/// \param callNow [in]	定时器停止时同时再调用一下回调函数，只对带延时的非周期定时器有效。
	/// \retval false 定时器没有开启的情况下调用
	/// \retval ture  成功
	bool stop(bool callNow = false);

	/// 得到定时器名称
	/// \retval 定时器名称
	const char* getName();

	/// 设置定时器名称
	/// \param name [in]	新的定时器名称
	void setName(const char* name);

	/// 判断定时器是否开启
	/// \reval true 已经开启
	/// \reval false 没有开启
	/// \note 非周期定时器在调用过后会自动将状态关闭
	bool isStarted();

	/// 判断非周期定时器是否已经调用过
	/// \reval true 已经调用
	/// \reval false 没有调用
	bool isCalled();

	/// 判断回调函数是否正在执行
	/// \retval true 正在执行
	/// \retval false 没有执行
	bool isRunning();

	/// 关闭定时器并等待,直到回调函数结束才返回
	/// \retval true 成功
	/// \retval false 失败
	/// \note 一般在用户类析构的时候调用,调用要特别小心,防止死锁
	bool stopAndWait();

private:
	TimerInternal* internal;
};


////////////////////////////////////////////////////////////////////////////////


/// \class TimerManager
/// \brief 定时器管理类，使用高精度系统定时其来驱动应用定时器工作
struct TimerManagerInternal;

class BASE_API TimerManager
{
	TimerManager(TimerManager const&);
	TimerManager& operator=(TimerManager const&);

public:
	TimerManager();
	/// 创建定时器管理对象
	static TimerManager* instance();
	
	/// 析构函数
	~TimerManager();

	/// 打印所有线程信息
	void dumpTimers();


	void uninit();

	TimerManagerInternal* internal;
};

#define gTimerManager (*TimerManager::instance())

} // namespace Base
} // namespace Xunmei

#endif


