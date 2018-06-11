//
//  Copyright (c)1998-2012, Public Technology
//  All Rights Reserved.
//
//	Description:
//	$Id: Base.h 3 2013-01-21 06:57:38Z  $
//   基础库统一头文件
#ifndef _BASE_CLASS_H_
#define _BASE_CLASS_H_

#include "Base/Defs.h"
#include "Base/AtomicCount.h"
#include "Base/Base64.h"
#include "Base/ConsoleCommand.h"
#include "Base/BaseTemplate.h"
#include "Base/ByteOrder.h"
#include "Base/Crc.h"
#include "Base/DynamicLib.h"
#include "Base/DynamicMemPool.h"
#include "Base/File.h"
#include "Base/FileBrowser.h"
#include "Base/Func.h"
#include "Base/Guard.h"
#include "Base/IntTypes.h"
#include "Base/Math.h"
#include "Base/Md5.h"
#include "Base/Mutex.h"
#include "Base/PrintLog.h"
#include "Base/Process.h"
#include "Base/ReadWriteMutex.h"
#include "Base/RecursiveMutex.h"
#include "Base/RecycleBin.h"
#include "Base/Semaphore.h"
#include "Base/Sha1.h"
#include "Base/ShareMem.h"
#include "Base/IPC.h"
#include "Base/SimPipe.h"
#include "Base/StaticMemPool.h"
#include "Base/String.h"
#include "Base/System.h"
#include "Base/Shared_ptr.h"
#include "Base/Thread.h"
#include "Base/ThreadEx.h"
#include "Base/ThreadPool.h"
#include "Base/Time.h"
#include "Base/Timer.h"
#include "Base/Unicode.h"
#include "Base/URLEncoding.h"
#include "Base/Version.h"
#include "Base/Expression.h"
#include "Base/CoreDump.h"
#include "Base/URI.h"
#include "Base/Guid.h"
#include "Base/Host.h"

/*
			Public 基础库介绍
一：线程、线程池、互斥、同步
	Thread.h			线程对象，继承使用，包含sleep；   
	ThreadEx.h			独立创建线程；   
	ThreadPool			线程池
	Muetx.h				线程互斥锁
	Guard.h				自动对互斥锁加锁解锁
	ReadWriteMutex.h	线程读写锁
	RecursiveMutex.h	线程递归锁
	Semaphore.h			线程同步信号量

三：内存管理
	DynamicMemPool.h	内存大小自动调整的内存池
	StaticMemPool.h		内存大小限定的内存池、并且内存地址可以外部分配
	ByteOrder.h			大小端判断及调整
	TempCache.h			临时缓存

四：系统相关
	AtomicCount.h		原子计数
	BaseTemplate.h		安全释放
	Shared_ptr.h		智能指针
	DynamicLib.h		动态库动态加载
	ConsoleCommand.h	终端输入
	File.h				文件操作
	FileBrowser.h		文件浏览器、文件夹操作
	PrintLog.h			打印接口
	String.h			字符串处理
	SimPipe.h			仿真管道、目前不能用于进程通讯
	Func.h				回调函数
	Callback.h			回调调用
	InstanceObjectDefine.h		单件定义及初始化

五：进程及进程通讯
	Process.h			进程创建相关
	ShareMem.h			共享内存管理
	ShareMemBuffer.h	进程间互斥锁，进程间信号量，进程通讯

六：时间、定时器
	Time.h				时间定义
	Timer.h				定时器管理
	TimeRecord.h		时间使用记录

七：编码解码
	Base64.h			base64编码、解码
	Crc.h				CRC编码
	Sha1.h				sha1编解码
	Md5.h				MD5加密
	URLEncoding.h		URL编解码

八：讯美定义定义
	URI.h				讯美Uri通讯解析
	Packet.h			讯美视频打包基础包
	MediaDefine.h		讯美视频定义
	MediaFraming.h		内存组成讯美视频帧
	MediaPackage.h		讯美视频帧
*/

namespace Public {
namespace Base {

///Base库的接口类
class BASE_API BaseSystem
{
public:
	//关闭事件定义
	enum CloseEvent
	{
		CloseEvent_INPUT,	//用户终端退出，需要用gConsoleCommand
		CloseEvent_CTRLC,	//程序收到ctrl+C杀死命令
		CloseEvent_CTRLBREAK,//程序收到ctrl+BREAK杀死命令
		CloseEvent_ONCLOSE,	//程序收到右上角关闭命令
		CloseEvent_LOGOFF,	//用户注销
	};

	//关闭事件回调通知函数
	typedef Function2<void,void*,CloseEvent> 	closeEventCallback;
public:
	/// 打印 Base库 版本信息
	static void  printVersion();

	//初始化Base、并注册一个关闭事件通知回调
	static void init(const closeEventCallback& closeEvent = NULL,void* userdata = NULL);
	//反初始化Base库
	static void uninit();

	//主动退出
	static void consoleCommandClose(CloseEvent cmd);

	//等待base退出接口、当使用输入终端时不适用该接口、该接口未堵塞模式
	static void waitSystemClose();

	//自动延时强制退出
	static void autoExitDelayer(uint64_t delaySecond = 30);
};

} // namespace Component
} // namespace Public

#endif // _BASE_CLASS_H_

