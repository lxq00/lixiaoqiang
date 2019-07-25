# 1. C++跨平台基础库
这是一套C++跨平台的基础服务库

# 2. 备注
 `windows`下第一次下载代码需要编译时，请运行`~firstCompile.bat`文件解压依赖包。

# 3. 包含内容
## 3.1. Base
> 这是一个系统基础接口的封装
### 3.1.1. 线程、线程池、互斥、同步
	Thread.h			线程对象，继承使用，包含sleep；   
	ThreadEx.h			独立创建线程；   
	ThreadPool			线程池
	Muetx.h				线程互斥锁
	Guard.h				自动对互斥锁加锁解锁
	ReadWriteMutex.h	线程读写锁
	RecursiveMutex.h	线程递归锁
	Semaphore.h			线程同步信号量

### 3.1.2. 内存管理
	DynamicMemPool.h	内存大小自动调整的内存池
	StaticMemPool.h		内存大小限定的内存池、并且内存地址可以外部分配
	ByteOrder.h			大小端判断及调整
	TempCache.h			临时缓存

### 3.1.3. 系统相关
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

### 3.1.4. 进程及进程通讯
	Process.h			进程创建相关
	ShareMem.h			共享内存管理
	ShareMemBuffer.h	进程间互斥锁，进程间信号量，进程通讯

### 3.1.5. 时间、定时器
	Time.h				时间定义
	Timer.h				定时器管理
	TimeRecord.h		时间使用记录

### 3.1.6. 编码解码
	Base64.h			base64编码、解码
	Crc.h				CRC编码
	Sha1.h				sha1编解码
	Md5.h				MD5加密
	URLEncoding.h		URL编解码

### 3.1.7. 定义
	URL.h				URL通讯解析

## 3.2. Network
基于Boost.ASIO实现的封装，包含异步同步的网络接口。
