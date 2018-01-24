//
//  Copyright (c)1998-2012, Xunmei Technology
//  All Rights Reserved.
//
//
//	Description:
//	$Id: System.h 3 2013-01-21 06:57:38Z jiangwei $


#ifndef __BASE_SYSTEM_H__
#define __BASE_SYSTEM_H__

#include "Defs.h"


namespace Xunmei {
namespace Base {

/// \function SystemCall
/// \brief 系统调用
/// \param cmd [in] 系统call的命令
/// \param 执行的返回值
int BASE_API SystemCall(const char* cmd);



} // namespace Base
} // namespace Xunmei

#endif //__BASE_SYSTEM_H__


