//
//  Copyright (c)1998-2012, Public Technology
//  All Rights Reserved.
//
//
//	Description:
//	$Id: ThreadEx.h 58 2013-03-20 08:44:05Z jiangwei $


#ifndef __BASE_THREADEX_H_
#define __BASE_THREADEX_H_

#include <string>
#include "Base/Defs.h"
#include "Thread.h"
#include "Func.h"

namespace Public{
namespace Base{


typedef Function1<void, void*>	ThreadExProc;

class BASE_API ThreadEx
{
	public:
		static Thread* creatThreadEx(const char* name,const ThreadExProc& proc, void* param,
			int priority = Thread::priorDefault, int policy = Thread::policyNormal, int stackSize = 0);
};

#define CreatBaseThreadEx ThreadEx::creatThreadEx



} // namespace Base
} // namespace Public

#endif //__BASE_THREADEX_H_


