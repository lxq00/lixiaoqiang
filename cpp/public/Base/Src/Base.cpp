//
//  Copyright (c)1998-2012, Chongqing Xunmei Technology
//  All Rights Reserved.
//
//	Description:
//	$Id: Base.cpp 11 2013-01-22 08:42:03Z lixiaoqiang $
//

#include "Base/Base.h"
#include "../version.inl"

namespace Xunmei {
namespace Base {

/// 打印 Base库 版本信息
void BaseSystem::printLibVersion()
{
	Xunmei::Base::AppVersion appVersion("Base Lib", r_major, r_minor, r_build, versionalias, __DATE__);
	appVersion.print();
}

void BaseSystem::AutoExitDelayer(uint64_t delaySecond /* = 30 */)
{
#ifdef WIN32
	uint32_t processId = GetCurrentProcessId();
	char tmpStr[512] = { 0 };
	snprintf_x(tmpStr, 512, "ping -n %llu 127.0.0.1 >nul&&taskkill /pid %u /f",
		delaySecond,
		processId
	);

	char systemPath[256] = { 0 };
	GetSystemDirectory(systemPath, 256);	

	const char* execargv[3];
	execargv[0] = "/c";
	execargv[1] = tmpStr;
	Process::createProcess((std::string(systemPath)+PATH_SEPARATOR+std::string("cmd.exe")).c_str(), 2, execargv);

#else //#ifdef x86

	uint32_t processId = getpid();
	char tmpStr[512] = { 0 };
	snprintf_x(tmpStr, 512, "sleep %llu&&kill -9 %u",
		delaySecond,
		processId
	);

	const char* execargv[3];
	execargv[0] = "-c";
	execargv[1] = tmpStr;

	Process::createProcess("/bin/sh", 2, execargv);
#endif
}

static BaseSystem::closeEventCallback	closeEvent = NULL;
static void*								closeParam = NULL;
static bool 								systemIsClose = false;
static Semaphore							closeSem;
static BaseSystem::CloseEvent			closeCmd;

void BaseSystem::ConsoleCommandClose(CloseEvent cmd)
{
	closeCmd = cmd;
	systemIsClose = true;
	closeSem.post();
}

void BaseSystem::WaitSystemClose()
{
	closeSem.pend();
	if(systemIsClose)
	{
		closeEvent(closeParam,closeCmd);
	}
	exit(0);
}

#ifdef WIN32
#include <Windows.h>
bool consoleHandler(DWORD event)
{
	if(systemIsClose)
	{
		return true;
	}
	switch(event)
	{
	case CTRL_C_EVENT:
		BaseSystem::ConsoleCommandClose(BaseSystem::CloseEvent_CTRLC);
		break;
	case CTRL_BREAK_EVENT:
		BaseSystem::ConsoleCommandClose(BaseSystem::CloseEvent_CTRLBREAK);
		break;
	case CTRL_CLOSE_EVENT:
		BaseSystem::ConsoleCommandClose(BaseSystem::CloseEvent_ONCLOSE);
		break;
	case CTRL_SHUTDOWN_EVENT:
	case CTRL_LOGOFF_EVENT:
		BaseSystem::ConsoleCommandClose(BaseSystem::CloseEvent_LOGOFF);
		break;
	default:
		break;
	}

	return true;
}
#else
#include <signal.h>
void consoleHandler(int event)
{
	if(systemIsClose)
	{
		return;
	}
	switch(event)
	{
	case SIGQUIT:
		BaseSystem::ConsoleCommandClose(BaseSystem::CloseEvent_CTRLBREAK);
		break;
	case SIGSTOP:
	case SIGTERM:
	case SIGINT:
		BaseSystem::ConsoleCommandClose(BaseSystem::CloseEvent_CTRLC);
		break;
	}
}
#endif

void BaseSystem::init(const closeEventCallback& _closeEvent,void* userdata)
{
	closeEvent = _closeEvent;
	closeParam = userdata;
#ifdef WIN32
	SetConsoleCtrlHandler((PHANDLER_ROUTINE)consoleHandler,true);
#else
	signal(SIGPIPE,SIG_IGN);
	signal(SIGQUIT,consoleHandler);
	signal(SIGSTOP,consoleHandler);
	signal(SIGTERM,consoleHandler);
	signal(SIGINT,consoleHandler);
#endif
}

void BaseSystem::uninit()
{
	SingleHolderInstance::uninit();
}

} // namespace Base
} // namespace Xunmei

