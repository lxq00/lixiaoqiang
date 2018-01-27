//
//  Copyright (c)1998-2014, Public Technology
//  All Rights Reserved.
//
//	Description:
//	$Id: PrintLog.h 3 2013-01-21 06:57:38Z  $
//

#ifndef __BASE_PRINT_LOG_H__
#define __BASE_PRINT_LOG_H__

#include <stdio.h>
#include "Defs.h"
#include "Func.h"
#include "IntTypes.h"
#include <string>

namespace Public{
namespace Base{

enum LOG_Level{
	LOG_Level_FATAL,
	LOG_Level_ERROR,
	LOG_Level_WARN,
	LOG_Level_INFO,
	LOG_Level_TRACE,
	LOG_Level_DEBUG
};


/// 打印输出回调函数类型
/// 参数为要打印的字符串
typedef Function2<void,LOG_Level,char const*> LogPrinterProc;

std::string BASE_API getLogTimeAndLevelString(LOG_Level lev);


///设置打印等级
//0:off 1:fatal 2:error 3:warn 4:info 5:trace 6:debug
void BASE_API setprintloglevel(int level);

/// 设置打印的输出回调函数
/// \param printer [in]  输出回调函数, 为空时设置打印输出到标准输出设备
/// \retval 0 成功
/// \retval -1 失败
int BASE_API setlogprinter(const LogPrinterProc& printer);

/// 清除打印的输出回调函数
int BASE_API cleanlogprinter(const LogPrinterProc& printer);

/// 记录调试信息，级别6，不直接调用，用于实现debgf宏
/// \return 返回打印的字节数
int BASE_API logdebug(const char* fmt, ...);

/// 记录跟踪信息，级别5，不直接调用，用于实现tracef宏
/// \return 返回打印的字节数
int BASE_API logtrace(const char* fmt, ...);

/// 记录调试信息，级别4，不直接调用，用于实现infof宏
/// \return 返回打印的字节数
int BASE_API loginfo(const char* fmt, ...);

/// 记录警告信息，级别3，不直接调用，用于实现warnf宏
/// \return 返回打印的字节数
int BASE_API logwarn(const char* fmt, ...);

/// 记录错误信息，级别2，不直接调用，用于实现errorf宏
/// \return 返回打印的字节数
int BASE_API logerror(const char* fmt, ...);

/// 记录致命错误信息，级别1，不直接调用，用于实现fatalf宏
/// \return 返回打印的字节数
int BASE_API logfatal(const char* fmt, ...);


} // namespace Base
} // namespace Public

#endif //__BASE_PRINT_LOG_H__


