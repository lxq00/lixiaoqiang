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
	LOG_Level_NONE,
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

///设置打印等级
void BASE_API setprintloglevel(LOG_Level level);

//打印内容
void BASE_API printer(LOG_Level level,const char* filename,const char* func,int line,const char* fmt, ...);

/// 设置打印的输出回调函数
/// \param printer [in]  输出回调函数, 为空时设置打印输出到标准输出设备
/// \retval 0 成功
/// \retval -1 失败
int BASE_API setlogprinter(void* userflag,const LogPrinterProc& printer);

/// 清除打印的输出回调函数
int BASE_API cleanlogprinter(void* userflag);


/// 记录调试信息，级别6，不直接调用，用于实现debgf宏
/// \return 返回打印的字节数
#define logdebug(fmt,...) printer(LOG_Level_DEBUG,__FILE__,__FUNCTION__,__LINE__,fmt,##__VA_ARGS__)

/// 记录跟踪信息，级别5，不直接调用，用于实现tracef宏
/// \return 返回打印的字节数
#define logtrace(fmt, ...) printer(LOG_Level_TRACE,__FILE__,__FUNCTION__,__LINE__,fmt,##__VA_ARGS__)

/// 记录调试信息，级别4，不直接调用，用于实现infof宏
/// \return 返回打印的字节数
#define loginfo(fmt, ...) printer(LOG_Level_INFO,__FILE__,__FUNCTION__,__LINE__,fmt,##__VA_ARGS__)

/// 记录警告信息，级别3，不直接调用，用于实现warnf宏
/// \return 返回打印的字节数
#define logwarn(fmt, ...) printer(LOG_Level_WARN,__FILE__,__FUNCTION__,__LINE__,fmt,##__VA_ARGS__)

/// 记录错误信息，级别2，不直接调用，用于实现errorf宏
/// \return 返回打印的字节数
#define logerror(fmt, ...) printer(LOG_Level_ERROR,__FILE__,__FUNCTION__,__LINE__,fmt,##__VA_ARGS__)

/// 记录致命错误信息，级别1，不直接调用，用于实现fatalf宏
/// \return 返回打印的字节数
#define logfatal(fmt, ...) printer(LOG_Level_FATAL,__FILE__,__FUNCTION__,__LINE__,fmt,##__VA_ARGS__)


} // namespace Base
} // namespace Public

#endif //__BASE_PRINT_LOG_H__


