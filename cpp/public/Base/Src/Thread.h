//
//  Copyright (c)1998-2012,  Public Technology
//  All Rights Reserved.
//
//	Description:
//	$Id: Thread.h 60 2013-03-23 08:07:40Z  $
//

#ifndef __INTERNAL_THREAD_H_
#define __INTERNAL_THREAD_H_

#include <string.h>
#include <string>
#include <map>


#include "Base/Thread.h"
#include "Base/Time.h"
#include "Base/Defs.h"
#include "Base/PrintLog.h"
#include "Base/Semaphore.h"
#include "Base/Mutex.h"
#include "Base/Guard.h"
#include "Base/IntTypes.h"

namespace Public{
namespace Base{

#define ErrorInfo_MaxLength 256

/// \class CThreadManager
/// \brief 线程管理类
/// \see CThread
struct ThreadErrorManager
{
public:

	/// 单建模式
	ThreadErrorManager();
	~ThreadErrorManager();

	bool cleanThreadErrorInfo(uint32_t threadId);

	bool setThreadErrorInfo(uint32_t threadId,const XM_ErrorInfo& info);

	bool getThreadErrorInfo(uint32_t threadId,XM_ErrorInfo& info);	
private:
	Mutex							  mutex; 
	std::map<uint32_t, XM_ErrorInfo>  threadErrorMap;		///< 线程错误信息
};

} // namespace Base
} // namespace Public


#endif //__INTERNAL_THREAD_H_

