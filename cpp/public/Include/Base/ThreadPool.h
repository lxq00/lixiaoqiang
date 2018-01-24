//
//  Copyright (c)1998-2012, Public Technology
//  All Rights Reserved.
//
//	Description:
//	$Id: ThreadPool.h 3 2013年8月21日10:18:30 lixiaoqiang $

#ifndef __BASE_THREADPOOL_H__
#define __BASE_THREADPOOL_H__

#include "Defs.h"
#include "Base/Mutex.h"
#include "Base/Thread.h"
#include "Base/Callback.h"

#include <list>
#include <map>

using namespace std;

namespace Public{
namespace Base{

///线程池执行回调函数
typedef Function1<void, void*> ThreadPoolHandler;

/// \class TreadPool
/// \brief 多平台线程池类
class BASE_API ThreadPool
{
public:
	struct ThreadPoolInternal;
	typedef enum 
	{
		Type_Manager,	//管理模式，内部包含有dispathcer池，Dispatch函数将信息放入池中，主线程获取信息创建线程管理
		Type_Normal,	//普通模式，Dispatch接口创建线程执行线程
	}Type;
public:
	/// 析构函数，创建和销毁线程池
	/// param[in] type    线程池工作模式
	/// param[in] maxDiapathcerSize 当type == Type_Manager，dispathcer池最大存放个数
	/// param[in] liveTime 线程执行后空闲存活时间/0表示执行后自动关闭，< 0表示一直空闲一直挂起  单位秒
	ThreadPool(Type type = Type_Normal,uint32_t maxDiapathcerSize = 64,uint32_t liveTime = 60);
	~ThreadPool(); 

	///线程池执行新函数接口
	/// param func[in] 需要执行的函数指针
	/// param param[n] 需要执行该函数指针的参数
	/// param [out] 0成功、-1失败
	bool Dispatch(const ThreadPoolHandler& func,void* param);

	void uninit();
private:
	ThreadPoolInternal* internal;
};

#define gThreadPool (*(ThreadPool::instance()))

};//Base
};//Public



#endif //__BASE_THREADPOOL_H__

