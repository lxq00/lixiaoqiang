#include "Base/CoreDump.h"
#include "Base/File.h"
#include "Base/Thread.h"
#include "Base/String.h"
#include "Base/System.h"
#include "Base/FileFind.h"
#ifdef WIN32
#include <windows.h>
#include <DbgHelp.h>
#include <stdlib.h>
#include <stdio.h>
#include <map>
#include <io.h>
#pragma comment(lib, "dbghelp.lib")
#else
#include <signal.h>
#include <execinfo.h>
#include <sys/types.h>
#include <sys/syscall.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#endif
namespace Public {
namespace Base {

std::string dumpSavePath = "";


std::string getRecordPath(const std::string& path)
{
	if(path != "" && path != "./")
	{
		return path;
	}
	
	return File::getExcutableFileFullPath();
}

std::string getRecordFile(const std::string& path)
{
	return getRecordPath(path) + "/.coredump_record.dat";
}

void findAndCheckCore()
{
#ifdef WIN32
	std::string path = getRecordPath(dumpSavePath) + PATH_SEPARATOR + "dmp.*";
#else
	std::string path = getRecordPath(dumpSavePath) + PATH_SEPARATOR + "core.*";
#endif
	FileFind finder(path);

	while (true)
	{
		shared_ptr<FileFind::FileFindInfo> info = finder.find();
		if (info == NULL)
		{
			break;
		}

		std::string filename = info->getFilePath();

		if (File::access(filename.c_str(), File::accessExist))
		{
			FileInfo fileinfo;
			File::stat(filename.c_str(), fileinfo);
			if (fileinfo.size == 0)
			{
				File::remove(filename.c_str());
				continue;
			}

			Time creatFileTime(fileinfo.timeWrite);

			char buffer[256] = { 0 };
			snprintf_x(buffer, 127, "echo %s >> \"%s\"", creatFileTime.format().c_str(), getRecordFile(dumpSavePath).c_str());
			SystemCall(buffer);
			
			char dumpfilename[256] = { 0 };
			snprintf_x(dumpfilename, 255, "%s/%s_%04d%02d%02d%02d%02d%02d"
#ifdef WIN32
				".dmp"
#else
				".core"
#endif
				, getRecordPath(dumpSavePath).c_str(), File::getExcutableFileName().c_str(), creatFileTime.year, creatFileTime.month, creatFileTime.day, creatFileTime.hour, creatFileTime.minute, creatFileTime.second);

#ifdef WIN32
			snprintf_x(buffer, 255, "move \"%s\" \"%s\"", filename.c_str(), dumpfilename);
#else
			snprintf_x(buffer, 255, "mv %s %s", filename.c_str(), dumpfilename);
#endif

			SystemCall(buffer);
		}
	}
}

#ifdef WIN32
inline BOOL IsDataSectionNeeded(const WCHAR* pModuleName)
{
	if (pModuleName == NULL)
	{
		return FALSE;
	}

	WCHAR szFileName[_MAX_FNAME] = L"";
	_wsplitpath(pModuleName, NULL, NULL, szFileName, NULL);

	if (_wcsicmp(szFileName, L"ntdll") == 0)
	{
		return TRUE;
	}

	return FALSE;
}

inline BOOL CALLBACK MiniDumpCallback(PVOID pParam, const PMINIDUMP_CALLBACK_INPUT pInput, PMINIDUMP_CALLBACK_OUTPUT pOutput)
{
	if (pInput == NULL || pOutput == NULL)
	{
		return FALSE;
	}

	switch (pInput->CallbackType)
	{
	case ModuleCallback:
		{
			if (pOutput->ModuleWriteFlags & ModuleWriteDataSeg)
			{
				if (!IsDataSectionNeeded(pInput->Module.FullPath))
				{
					pOutput->ModuleWriteFlags &= (-ModuleWriteDataSeg);
				}
			}
		}
	case IncludeModuleCallback:
	case IncludeThreadCallback:
	case ThreadCallback:
	case ThreadExCallback:
		{
			return TRUE;
		}
	default:
		break;
	}

	return FALSE;
}
inline void CreateMiniDump(PEXCEPTION_POINTERS pep, LPCTSTR strFileName)
{
	HANDLE hFile = CreateFile(strFileName, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_WRITE, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hFile != NULL && hFile != INVALID_HANDLE_VALUE)
	{
		MINIDUMP_EXCEPTION_INFORMATION mdei;
		mdei.ThreadId = GetCurrentThreadId();
		mdei.ExceptionPointers = pep;
		mdei.ClientPointers = NULL;

		MINIDUMP_CALLBACK_INFORMATION mci;
		mci.CallbackRoutine = (MINIDUMP_CALLBACK_ROUTINE)MiniDumpCallback;
		mci.CallbackParam = 0;

		::MiniDumpWriteDump(::GetCurrentProcess(), ::GetCurrentProcessId(), hFile, MiniDumpNormal, (pep != 0) ? &mdei : 0, NULL, &mci);

		CloseHandle(hFile);
	}
}

std::string getDumpFileNameAndWriteRecord()
{
	Time currtime = Time::getCurrentTime();
	
	char dumpfilename[256] = { 0 };
	snprintf_x(dumpfilename, 255, "%s/dmp.%04d%02d%02d%02d%02d%02d",
		getRecordPath(dumpSavePath).c_str(), currtime.year, currtime.month, currtime.day, currtime.hour, currtime.minute, currtime.second);

	return dumpfilename;

}
LONG __stdcall MyUnhandleExceptionFilter(PEXCEPTION_POINTERS pExcetptionInfo)
{
	CreateMiniDump(pExcetptionInfo, getDumpFileNameAndWriteRecord().c_str());

	return EXCEPTION_EXECUTE_HANDLER;
}
void DisableSetUnhandleExceptionFilter()
{
	void* addr = (void*)GetProcAddress(LoadLibrary("kernel32.dll"), "SetUnhandleExceptionFilter");

	if (addr)
	{
		unsigned char code[16];
		int size = 0;

		code[size++] = 0x33;
		code[size++] = 0xC0;
		code[size++] = 0xC2;
		code[size++] = 0x04;
		code[size++] = 0x00;

		DWORD dwOldFlag, dwTempFlag;
		VirtualProtect(addr, size, PAGE_READWRITE, &dwOldFlag);
		WriteProcessMemory(GetCurrentProcess(), addr, code, size, NULL);
		VirtualProtect(addr, size, dwOldFlag, &dwTempFlag);
	}
}
void initAutoDump()
{
	//设置错误模式，解决未捕获到异常，弹出的错误提示框
	SetErrorMode(SEM_FAILCRITICALERRORS | SEM_NOGPFAULTERRORBOX);
	//注册异常处理函数
	SetUnhandledExceptionFilter(MyUnhandleExceptionFilter);

	//是SetUnhandleExceptionFilter无效
	DisableSetUnhandleExceptionFilter();
}
void uninitAutoDump()
{
	//恢复系统默认错误模式
	SetErrorMode(0);
	SetUnhandledExceptionFilter(NULL);
}
#else

void initAutoDump()
{
}
void uninitAutoDump()
{
}
#endif

void CoreDump::audoCoreDump(const std::string& savePath)
{
	dumpSavePath = savePath;
	findAndCheckCore();
	initAutoDump();
	cleanCoreDumpReocrdByTimeout();
}
void CoreDump::cleanAudoCoreDump()
{
	uninitAutoDump();
}
bool CoreDump::getCoreDumpRecord(std::list<Time>& record,const std::string& savePath)
{
	std::string recordfile = getRecordFile(savePath);

	FILE* fd = fopen(recordfile.c_str(),"rb+");
	if(fd == NULL)
	{
		return true;
	}

	char buffer[256];
	while(fgets(buffer,255,fd) != NULL)
	{
		Time tmp = Time::getCurrentTime();
		tmp.parse(buffer);
		record.push_back(tmp);
	}

	fclose(fd);

	return true;
}

bool CoreDump::cleanAndSyncReocrd(const std::string& savePath)
{
	std::string recordfile = getRecordFile(savePath);
	File::remove(recordfile.c_str());

	return true;
}

struct CoreDumpFileInfo
{
	uint64_t		createTime;
	std::string		filename;

	const bool operator < (const CoreDumpFileInfo& info)
	{
		return createTime < info.createTime;
	}
};

bool CoreDump::cleanCoreDumpReocrdByTimeout(uint32_t maxDay,uint32_t maxNum,const std::string& savePath)
{
#define ONDAYSEC (24*60*60)

#ifdef WIN32
	std::string path = getRecordPath(savePath) + PATH_SEPARATOR + "*.dmp";
#else
	std::string path = getRecordPath(savePath) + PATH_SEPARATOR + "*.core";
#endif // WIN32

	std::list<CoreDumpFileInfo>		coreDumpList;
	FileFind finder(path);

	while (true)
	{
		shared_ptr<FileFind::FileFindInfo> info = finder.find();
		if (info == NULL)
		{
			break;
		}

		std::string filename = info->getFilePath();

		FileInfo fileinfo;
		File::stat(filename.c_str(), fileinfo);

		CoreDumpFileInfo cinfo;
		cinfo.createTime = fileinfo.timeCreate;
		cinfo.filename = filename;

		coreDumpList.push_back(cinfo);
	}
	
	coreDumpList.sort();
	//Time currTime = Time::getCurrentTime();

	while (coreDumpList.size() > 0)
	{
		CoreDumpFileInfo info = coreDumpList.front();
		if (coreDumpList.size() > maxNum /*|| (info.createTime < currTime.makeTime() && currTime.makeTime() - info.createTime >= maxDay*ONDAYSEC)*/)
		{
			File::remove(info.filename.c_str());
			coreDumpList.pop_front();
		}
		else
		{
			break;
		}
	}

	return true;
}

//清除多有dump及先关记录
bool CoreDump::cleanCoreDumpReocrd(const std::string& savePath)
{
	std::string recordfile = getRecordFile(savePath);
	File::remove(recordfile.c_str());

#ifdef WIN32
	std::string path = getRecordPath(savePath) + PATH_SEPARATOR + "*.dmp";
#else
	std::string path = getRecordPath(savePath) + PATH_SEPARATOR + "*.core";
#endif // WIN32

	FileFind finder(path);

	while (true)
	{
		shared_ptr<FileFind::FileFindInfo> info = finder.find();
		if (info == NULL)
		{
			break;
		}

		std::string filename = info->getFilePath();

		File::remove(filename.c_str());

	}

	return true;
}

//打包压缩dump文件
bool CoreDump::getCoreDumpFileList(std::list<std::string>& filelist,const std::string& savePath)
{

#ifdef WIN32
	std::string path = getRecordPath(savePath) + PATH_SEPARATOR + "*.dmp";
#else
	std::string path = getRecordPath(savePath) + PATH_SEPARATOR + "*.core";
#endif // WIN32

	FileFind finder(path);

	while (true)
	{
		shared_ptr<FileFind::FileFindInfo> info = finder.find();
		if (info == NULL)
		{
			break;
		}

		std::string filename = info->getFilePath();

		filelist.push_back(filename);
	}

	return true;
}


}
}
