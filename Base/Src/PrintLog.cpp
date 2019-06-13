//
//  Copyright (c)1998-2012,  Public Technology
//  All Rights Reserved.
//
//	Description:
//	$Id: PrintLog.cpp 11 2013-01-22 08:42:03Z  $
//

#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#ifdef WIN32
#include <Windows.h>
#endif
#include <time.h>
#include "Base/Defs.h"
#include "Base/PrintLog.h"
#include "Base/Time.h"
#include "Base/Mutex.h"
#include "Base/Guard.h"
#include <list>

using namespace std;

namespace {

#ifdef _WIN32
	#define print_log_snprintf		_snprintf_s
	#define print_log_vsnprintf		_vsnprintf_s
#else
	#define print_log_snprintf		snprintf
	#define print_log_vsnprintf		vsnprintf
#endif


#if defined(WIN32) && !defined(_WIN32_WCE)

enum {
	COLOR_FATAL = FOREGROUND_RED | FOREGROUND_BLUE,
	COLOR_ERROR = FOREGROUND_RED,
	COLOR_WARN = FOREGROUND_RED | FOREGROUND_GREEN,
	COLOR_INFO = FOREGROUND_GREEN,
	COLOR_TRACE = FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE,
	COLOR_DEBUG = FOREGROUND_GREEN | FOREGROUND_BLUE,
};

static HANDLE stdout_handle = 0;
static WORD old_color_attrs = 0;

inline void set_console_color(int c)
{
	if (stdout_handle == 0)
	{
		stdout_handle = GetStdHandle(STD_OUTPUT_HANDLE);
	}

	// Gets the current text color.
	CONSOLE_SCREEN_BUFFER_INFO buffer_info;
	GetConsoleScreenBufferInfo(stdout_handle, &buffer_info);
	old_color_attrs = buffer_info.wAttributes;

	// We need to flush the stream buffers into the console before each
	// SetConsoleTextAttribute call lest it affect the text that is already
	// printed but has not yet reached the console.
	fflush(stdout);
	SetConsoleTextAttribute(stdout_handle, c);
}

inline void reset_console_color()
{
	if (stdout_handle == 0)
	{
		stdout_handle = GetStdHandle(STD_OUTPUT_HANDLE);
	}

	fflush(stdout);
	// Restores the text color.
	SetConsoleTextAttribute(stdout_handle, old_color_attrs);
}

#else // defined(WIN32) && !defined(_WIN32_WCE)
enum {
	COLOR_FATAL = 35,
	COLOR_ERROR = 31,
	COLOR_WARN = 33,
	COLOR_INFO = 32,
	COLOR_TRACE = 37,
	COLOR_DEBUG = 36
};

inline void set_console_color(int c)
{
	fprintf(stdout, "\033[%d;40m", c);
}

inline void reset_console_color()
{
	fprintf(stdout, "\033[0m");
}

#endif  // defined(WIN32) && !defined(_WIN32_WCE)

////////////////////////////////////////////////////////////////////////////////




} // namespace noname

////////////////////////////////////////////////////////////////////////////////

namespace Public {
namespace Base {

struct LogPrintInfoInternal
{
	LOG_Level s_printLogLevel;

	std::map<void*,Public::Base::LogPrinterProc> s_printList;
	Mutex logMutex;

	LogPrintInfoInternal():s_printLogLevel(LOG_Level_DEBUG){}
};

static LogPrintInfoInternal logmanager;



std::string getLogTimeAndLevelString(LOG_Level lev)
{
	static const char* leveArray[]={"","FATAL","ERROR","WARN","INFO","TRACE","DEBUG"};

	const char* levestring = "";
	if(lev >= LOG_Level_FATAL && lev <= LOG_Level_DEBUG)
	{
		levestring = leveArray[lev];
	}

	char fmttmp[64] = {0};
	Time t = Time::getCurrentTime(); 
	print_log_snprintf(fmttmp,64,"%04d-%02d-%02d %02d:%02d:%02d,%03u %s - ",t.year,t.month,t.day,t.hour, t.minute, t.second,(uint32_t)(Time::getCurrentMilliSecond() % 1000),levestring);

	return fmttmp;
}

inline void print(LOG_Level lev,char const* s)
{
	LogPrintInfo info;
	info.logprex = getLogTimeAndLevelString(lev);
	info.logstr = s;
	info.loglev = lev;
	{
		static int levelColor[] = { 0, COLOR_FATAL ,COLOR_FATAL ,COLOR_ERROR,COLOR_INFO,COLOR_TRACE,COLOR_DEBUG};

		set_console_color(levelColor[lev]);
		
		int len = strlen(s);
		bool ishaveenter = false;
		if(len > 1 && (s[len-1] == '\n' || s[len-1] == '\r'))
		{
			ishaveenter = true;
		}
		fputs(info.logprex.c_str(), stdout);
		fputs(info.logstr.c_str(), stdout);
		if(!ishaveenter)
		{
			fputs("\r\n", stdout);
		}

		reset_console_color();
	}
	{
		std::map<void*,Public::Base::LogPrinterProc>::iterator iter;
		for(iter = logmanager.s_printList.begin();iter != logmanager.s_printList.end();iter ++)
		{
			Public::Base::LogPrinterProc s_print = iter->second;
			s_print(info);
		}
	}
}
int BASE_API setlogprinter(void* userflag,const LogPrinterProc& printer)
{
	Guard locker(logmanager.logMutex);

	logmanager.s_printList[userflag] = printer;

	return 0;
}

int BASE_API cleanlogprinter(void* userflag)
{
	Guard locker(logmanager.logMutex);
	logmanager.s_printList.erase(userflag);

	return 0;
}

void BASE_API setprintloglevel(LOG_Level level)
{
	logmanager.s_printLogLevel = level;
}


#ifndef DEBUG
#define DEBUGPrintLoeveCheck(lev) /*{if(lev == LOG_Level_DEBUG) return -1;}*/
#else
#define DEBUGPrintLoeveCheck(lev)
#endif


void printer(LOG_Level level, const char* filename, const char* func, int line, const char* fmt, ...)
{
	DEBUGPrintLoeveCheck(LOG_Level_DEBUG);

	if (logmanager.s_printLogLevel < level)return;

	{
		
		Guard locker(logmanager.logMutex);
#define MAXPRINTBUFFERLEN	10240
		static char buffer[MAXPRINTBUFFERLEN] = {0};
		
		va_list ap; 
		va_start(ap, fmt); 
		print_log_vsnprintf(buffer, MAXPRINTBUFFERLEN - 1, fmt, ap);
		va_end(ap);	

		const char* filenametmp = strrchr(filename, '/');
		if (filenametmp == NULL) filenametmp = strrchr(filename, '\\');

		if (filenametmp == NULL) filenametmp = "";
		else filenametmp = filenametmp + 1;

		static char buffertmp[1024];
		snprintf(buffertmp, 1023, " [%s %s:%d]", filenametmp, func, line);

		std::string printstr = buffer + std::string(buffertmp);

		print(level, printstr.c_str());
	}
}

} // namespace Base
} // namespace Public

