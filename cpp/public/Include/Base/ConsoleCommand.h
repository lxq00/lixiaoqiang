#ifndef _BASE_CONSOLE_COMMAND_H
#define _BASE_CONSOLE_COMMAND_H

#include <iostream>
#include <list>
using namespace std;

#include "Base/Defs.h"
#include "Base/Thread.h"
using namespace Public::Base;

namespace Public {
namespace Base {

//命令回调函数
typedef Function2<void, int, char**> ConsoleCmdCallback;

class BASE_API ConsoleCommand : public Thread
{
public:
	ConsoleCommand(void);
	~ConsoleCommand(void);

	//设置Console模块标题，在命令行前会自动显示该标题
	//标题默认为空
	void setModuleTitle(const std::string& title);

	//判断Console命令读取线程是否还在执行
	//true表示正在运行
	//false表示停止运行
	bool getValid(void);

	//启动Console命令读取线程
	//standalone:
	//			false表示对象使用独立线程
	//			true表示对象使用调用者线程
	void run();

	//添加用户自定义命令
	//name : 命令名称
	//desc : 命令描述
	//func : 命令回调函数
	bool addUserCmd(const std::string& name, const std::string& desc,const ConsoleCmdCallback& func);

protected:
	/// 线程执行体,是一个虚函数,派生的线程类中重载此函数,实现各自的行为.
	virtual void threadProc();

	void runing();
	//命令解析
	virtual bool parseCmd(const char* command);

private:
	//分发命令
	//?,help,exit命令由内部处理，其他命令使用回调函数由用户处理
	bool dispatchCmd(const int argc, char** argv, ConsoleCmdCallback func);

	//分离命令和命令参数
	bool separatePara(const char* command, int& argc, char** argv);

private:
	//数据集合
	struct tagConsoleInternal_t;
	tagConsoleInternal_t* consoleInternal;
	bool finished;
};//class ConsoleCommand
}//namespace Base
}//namespace Public

#endif//_BASE_CONSOLE_COMMAND_H
