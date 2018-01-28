//
//  Copyright (c)1998-2012, Public Technology
//  All Rights Reserved.
//
//
//	Description:
//	$Id: ThreadEx.h 58 2013-03-20 08:44:05Z  $


#ifndef __BASE_THREADEX_H_
#define __BASE_THREADEX_H_

#include <string>
#include "Base/Defs.h"
#include "Thread.h"
#include "Func.h"

namespace Public{
namespace Base{


class BASE_API ThreadEx
{
	public:
		typedef Function2<void, Thread*, void*>	Proc;

		static shared_ptr<Thread> creatThreadEx(const std::string& name,const Proc& proc, void* param,
			int priority = Thread::priorDefault, int policy = Thread::policyNormal, int stackSize = 0);
};


} // namespace Base
} // namespace Public

#endif //__BASE_THREADEX_H_


