//
//  Copyright (c)1998-2012, Chongqing Public Technology
//  All Rights Reserved.
//
//	Description:
//	$Id: PrintLog.cpp 11 2013-01-22 08:42:03Z jiangwei $
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
	int s_printLogLevel;

	std::list<Public::Base::LogPrinterProc> s_printList;
	Mutex logMutex;

	LogPrintInfoInternal():s_printLogLevel(6){}
};

static LogPrintInfoInternal logmanager;



std::string getLogTimeAndLevelString(LOG_Level lev)
{
	static const char* leveArray[]={"fatal","error","warn","info","trace","debug"};

	const char* levestring = "";
	if(lev >= LOG_Level_FATAL && lev <= LOG_Level_DEBUG)
	{
		levestring = leveArray[lev];
	}

	char fmttmp[64] = {0};
	Time t = Time::getCurrentTime(); 
	print_log_snprintf(fmttmp,64,"%02d:%02d:%02d|%s ",t.hour, t.minute, t.second,levestring);

	return fmttmp;
}

inline void print(LOG_Level lev,int color,char const* s)
{
	if (logmanager.s_printList.size() == 0)
	{
		set_console_color(color);
		
		int len = strlen(s);
		bool ishaveenter = false;
		if(len > 1 && (s[len-1] == '\n' || s[len-1] == '\r'))
		{
			ishaveenter = true;
		}
		fputs(getLogTimeAndLevelString(lev).c_str(), stdout);
		fputs(s, stdout);
		if(!ishaveenter)
		{
			fputs("\r\n", stdout);
		}

		reset_console_color();
	}
	else
	{
		std::list<Public::Base::LogPrinterProc>::iterator iter;
		for(iter = logmanager.s_printList.begin();iter != logmanager.s_printList.end();iter ++)
		{
			Public::Base::LogPrinterProc s_print = *iter;
			if(!s_print.empty())
			{
				s_print(lev,s);
			}
		}
	}
}
int BASE_API setlogprinter(const LogPrinterProc& printer)
{
	Guard locker(logmanager.logMutex);

	logmanager.s_printList.push_back(printer);

	return 0;
}

int BASE_API cleanlogprinter(const LogPrinterProc& printer)
{
	Guard locker(logmanager.logMutex);

	std::list<Public::Base::LogPrinterProc>::iterator iter;
	for (iter = logmanager.s_printList.begin(); iter != logmanager.s_printList.end(); iter++)
	{
		Public::Base::LogPrinterProc s_print = *iter;
		if(s_print == printer)
		{
			logmanager.s_printList.erase(iter);
			break;
		}
	}
	return 0;
}

void BASE_API setprintloglevel(int level)
{
	logmanager.s_printLogLevel = level;
}

////////////////////////////////////////////////////////////////////////////////
#define LOG_MSG(x, y,l) {\
	Guard locker(logmanager.logMutex);\
	char buffer[8192]; \
	buffer[8191] = 0; \
	va_list ap; \
	va_start(ap, fmt); \
	int n = print_log_vsnprintf(buffer, sizeof(buffer) - 1,fmt, ap); \
	va_end(ap);	\
	print(l,y,buffer); \
	return int(n);	\
}

#ifndef DEBUG
#define DEBUGPrintLoeveCheck(lev) /*{if(lev == LOG_Level_DEBUG) return -1;}*/
#else
#define DEBUGPrintLoeveCheck(lev)
#endif

int BASE_API logdebug(const char* fmt, ...)
{
	DEBUGPrintLoeveCheck(LOG_Level_DEBUG);
	
	if(logmanager.s_printLogLevel < 6)return 0;

	LOG_MSG("debug ", COLOR_DEBUG,LOG_Level_DEBUG);
}

int BASE_API logtrace(const char* fmt, ...)
{
	DEBUGPrintLoeveCheck(LOG_Level_TRACE);

	if(logmanager.s_printLogLevel < 5)return 0;

	LOG_MSG("trace ", COLOR_TRACE,LOG_Level_TRACE);
}

int BASE_API loginfo(const char* fmt, ...)
{
	DEBUGPrintLoeveCheck(LOG_Level_INFO);
	
	if(logmanager.s_printLogLevel < 4)return 0;

	LOG_MSG("info  ", COLOR_INFO,LOG_Level_INFO);
}

int BASE_API logwarn(const char* fmt, ...)
{
	DEBUGPrintLoeveCheck(LOG_Level_WARN);
	
	if(logmanager.s_printLogLevel < 3)return 0;

	LOG_MSG("warn  ", COLOR_WARN,LOG_Level_WARN);
}

int BASE_API logerror(const char* fmt, ...)
{
	DEBUGPrintLoeveCheck(LOG_Level_ERROR);
	
	if(logmanager.s_printLogLevel < 2)return 0;

	LOG_MSG("error ", COLOR_ERROR,LOG_Level_ERROR);
}

int BASE_API logfatal(const char* fmt, ...)
{
	DEBUGPrintLoeveCheck(LOG_Level_FATAL);
	
	if(logmanager.s_printLogLevel < 1)return 0;

	LOG_MSG("fatal ", COLOR_FATAL,LOG_Level_FATAL);
}

} // namespace Base
} // namespace Public

