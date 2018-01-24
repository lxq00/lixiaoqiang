//
//  Copyright (c)1998-2014, Xunmei Technology
//  All Rights Reserved.
//
//	Description:
//	$Id: Callback.h 3 2013-01-21 06:57:38Z jiangwei $

#ifndef __BASE_CALLBACK_H__
#define __BASE_CALLBACK_H__

#if defined(_MSC_VER)
	#pragma warning (push)
	#pragma warning (disable : 4786)
#endif
#include <stdio.h>
#include "Func.h"
#include "Mutex.h"
#include "Guard.h"
#include "Thread.h"
#include "Time.h"
#include "PrintLog.h"

namespace Xunmei{
namespace Base{

//Callback0
#define CALLBACK_NUMBER 0
#define CALLBACK_CLASS_TYPES void
#define CALLBACK_TYPES_COMMA
#define CALLBACK_TYPE_ARGS void
#define CALLBACK_ARGS
#include "CallbackTempl.h"
#undef  CALLBACK_NUMBER
#undef  CALLBACK_CLASS_TYPES
#undef	CALLBACK_TYPES_COMMA
#undef	CALLBACK_TYPE_ARGS
#undef	CALLBACK_ARGS

//Callback1
#define CALLBACK_NUMBER 1
#define CALLBACK_CLASS_TYPES typename T1
#define CALLBACK_TYPES_COMMA , T1
#define CALLBACK_TYPE_ARGS T1 a1
#define CALLBACK_ARGS a1
#include "CallbackTempl.h"
#undef  CALLBACK_NUMBER
#undef  CALLBACK_CLASS_TYPES
#undef	CALLBACK_TYPES_COMMA
#undef	CALLBACK_TYPE_ARGS
#undef	CALLBACK_ARGS

//Callback2
#define CALLBACK_NUMBER 2
#define CALLBACK_CLASS_TYPES typename T1, typename T2
#define CALLBACK_TYPES_COMMA , T1, T2
#define CALLBACK_TYPE_ARGS T1 a1, T2 a2
#define CALLBACK_ARGS a1, a2
#include "CallbackTempl.h"
#undef  CALLBACK_NUMBER
#undef  CALLBACK_CLASS_TYPES
#undef	CALLBACK_TYPES_COMMA
#undef	CALLBACK_TYPE_ARGS
#undef	CALLBACK_ARGS

//Callback3
#define CALLBACK_NUMBER 3
#define CALLBACK_CLASS_TYPES typename T1, typename T2, typename T3
#define CALLBACK_TYPES_COMMA , T1, T2, T3
#define CALLBACK_TYPE_ARGS T1 a1, T2 a2, T3 a3
#define CALLBACK_ARGS a1, a2, a3
#include "CallbackTempl.h"
#undef  CALLBACK_NUMBER
#undef  CALLBACK_CLASS_TYPES
#undef	CALLBACK_TYPES_COMMA
#undef	CALLBACK_TYPE_ARGS
#undef	CALLBACK_ARGS

//Callback4
#define CALLBACK_NUMBER 4
#define CALLBACK_CLASS_TYPES typename T1, typename T2, typename T3, typename T4
#define CALLBACK_TYPES_COMMA , T1, T2, T3, T4
#define CALLBACK_TYPE_ARGS T1 a1, T2 a2, T3 a3, T4 a4
#define CALLBACK_ARGS a1, a2, a3, a4
#include "CallbackTempl.h"
#undef  CALLBACK_NUMBER
#undef  CALLBACK_CLASS_TYPES
#undef	CALLBACK_TYPES_COMMA
#undef	CALLBACK_TYPE_ARGS
#undef	CALLBACK_ARGS

//Callback5
#define CALLBACK_NUMBER 5
#define CALLBACK_CLASS_TYPES typename T1, typename T2, typename T3, typename T4, typename T5
#define CALLBACK_TYPES_COMMA , T1, T2, T3, T4, T5
#define CALLBACK_TYPE_ARGS T1 a1, T2 a2, T3 a3, T4 a4, T5 a5
#define CALLBACK_ARGS a1, a2, a3, a4, a5
#include "CallbackTempl.h"
#undef  CALLBACK_NUMBER
#undef  CALLBACK_CLASS_TYPES
#undef	CALLBACK_TYPES_COMMA
#undef	CALLBACK_TYPE_ARGS
#undef	CALLBACK_ARGS

//Callback6
#define CALLBACK_NUMBER 6
#define CALLBACK_CLASS_TYPES typename T1, typename T2, typename T3, typename T4, typename T5, typename T6
#define CALLBACK_TYPES_COMMA , T1, T2, T3, T4, T5, T6
#define CALLBACK_TYPE_ARGS T1 a1, T2 a2, T3 a3, T4 a4, T5 a5, T6 a6
#define CALLBACK_ARGS a1, a2, a3, a4, a5, a6
#include "CallbackTempl.h"
#undef  CALLBACK_NUMBER
#undef  CALLBACK_CLASS_TYPES
#undef	CALLBACK_TYPES_COMMA
#undef	CALLBACK_TYPE_ARGS
#undef	CALLBACK_ARGS

} // namespace Base
} // namespace Xunmei

#if defined(_MSC_VER)
	#pragma warning (pop)
#endif

#endif //__BASE_CALLBACK_H__

