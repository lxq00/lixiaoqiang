//
//  Copyright (c)1998-2012, Public Technology
//  All Rights Reserved.
//
//
//	Description:
//	$Id: Thread.h 159 2013-09-03 05:37:51Z  $


#ifndef __BASE_THREAD_H_
#define __BASE_THREAD_H_
#include <stdio.h>
#include <string>
#include "Base/Defs.h"
#include "Defs.h"
#include "Func.h"

namespace Public{
namespace Base{


typedef struct
{
	int errorCode;				///错误码
	std::string info;			///错误信息
	std::string stacks;			///错误堆栈
} XM_ErrorInfo;

/// 按线程设置最后的错误码
/// \param errCode [in] 错误码
/// \param info [in] 错误码的描述信息
/// \retval true 成功
/// \retval false 失败	
bool BASE_API XM_SetLastError(int errCode, const char *info);

bool BASE_API XM_SetLastErrorInfo(int errCode, const char *fmt, ...);


/// 按线程设置最后的错误码
/// \param errCode [in] 错误码
/// \param info [in] 错误码的描述信息
/// \retval true 成功
/// \retval false 失败	
bool BASE_API XM_SetLastErrorEx(const XM_ErrorInfo &lastinfo);

/// 获得最后的错误码
/// \param errinfo [out] 错误码
/// \retval true 成功
/// \retval false 失败	
bool BASE_API XM_GetLastError(XM_ErrorInfo &errinfo);

/// 清空最后的错误码
/// \retval true 成功
/// \retval false 失败
bool BASE_API XM_ClearLastError();

/// 往错误信息中添加附加的调用栈信息
/// \param detail [in] 附加信息
/// \retval true 成功
/// \retval false 失败	
bool BASE_API XM_AddLastErrorStack(const char *detail);


struct ThreadInternal;

/// \class Thread
/// \brief 多平台线程类; 提供继承方式创建线程
class BASE_API Thread
{
	Thread(Thread const&);
	Thread& operator=(Thread const&);

public:
	enum Priority
	{
		priorTop = 1,
		priorBottom = 127,
		priorDefault = 64,
	};

	enum Policy
	{
		policyNormal = 0,			///< 普通线程
		policyRealtime = 1		///< 实时线程
	};

	/// 构造函数,并没有创建系统线程
	/// \param name [in]线程名称字符串,名称不一定会传给操作系统,但CThreadManager
	///		   管理线程时会用到.
	/// \param priority [in]线程优先级,值越小表示优先级越高,会被转化成对应操作系统
	///        平台的优先级.取值priorTop到priorBottom,默认值priorDefault.
	/// \param policy [in] 线程调度策略
	/// \param stackSize [in] 为线程指定的堆栈大小,如果等于0或者小于平台要求必须的值,
	///        则使用平台缺省值.
	Thread(const std::string& name, int priority = priorDefault, int policy = policyNormal, int stackSize = 0);

	/// 析构函数,如果线程还在执行,会销毁线程
	virtual ~Thread();

	/// 线程执行体,是一个虚函数,派生的线程类中重载此函数,实现各自的行为.
	virtual void threadProc() = 0;

	/// 创建线程
	/// \retval ture 成功
	/// \retval false 失败
	bool createThread();

	/// 销毁线程,设置退出标志,线程的执行体需要在各退出点判断这个标志.
	/// \retval true 成功
	/// \retval false 失败
	bool destroyThread();

	/// 终止线程,和销毁线程不同在于它是由操作系统强制销毁线程,不保证用户数据安全.
	/// \retval true 成功
	/// \retval false 失败 
	bool terminateThread();

	/// 取消线程,设置线程退出标志,非阻塞方式,不等待线程结束
	/// \retval true 成功
	/// \retval false 失败
	bool cancelThread();

	/// 判断线程已经结束还是正在执行
	/// \retval true 已经结束
	/// \retval false 还在执行
	bool isThreadOver();

	/// 得到线程ID
	/// \retval 线程ID
	int	getThreadID();

	/// 设置线程名称
	/// \param name [in] 新的线程名称
	void setThreadName(const std::string& name);

	/// 设置超时时间
	/// \param milliSeconds [in] 超时毫秒数,设置为0表示清空设置
	void setTimeout(int milliSeconds);

	/// 判断是否超时,也就是判断从最后一次SetTimeout到这次调用IsTimeout的时间间隔
	/// 是否已经在超时时间之外.这两个接口目的是提供给用户监视和调度线程的方法,
	/// CThread类并不会根据是否超时对线程执行做任何干预.CThreadManager打印所有
	/// 线程状态时会调用这个接口.
	/// \retval true 已经超时
	/// \retval false 没有超时
	bool isTimeout();

	/// 线程退出标志, 线程体根据这个标志退出
	/// \retval true 成功
	/// \retval false 失败
	bool looping() const;

	/// 得到调用线程的ID,而不是某个线程对象的ID,是一个静态函数.
	/// \retval 线程ID
	static int getCurrentThreadID();

	/// 让调用线程阻塞一段时间
	/// \param milliSeconds [in] 期望阻塞的毫秒数
	static void sleep(int milliSeconds);

public:
	ThreadInternal* internal;
};

} // namespace Base
} // namespace Public


#ifndef MAX_PATH
#define MAX_PATH 260 // add by ,no fond by linux
#endif

#ifndef WIN32
namespace {
#include <stdio.h>
inline void GetModuleFileName(char *name, char *path, int len)
{
	FILE *stream = fopen("/proc/self/cmdline", "r");
	fgets(path, len, stream);
	fclose(stream);
}
}

#endif


#endif //_BASE_THREAD_H_

