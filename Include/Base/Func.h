//
//  Copyright (c)1998-2012, Public Technology
//  All Rights Reserved.
//
//	Description:
//	$Id: Func.h 3 2013-01-21 06:57:38Z  $
//


#ifndef __BASE_FUNCTION_H__
#define __BASE_FUNCTION_H__

#include <assert.h>
#include <typeinfo>
#include "Defs.h"
#include "Shared_ptr.h"

#ifdef GCCSUPORTC11
#include <functional>
#endif

namespace Public {
namespace Base {
	
// from http://www.codeproject.com/cpp/FastDelegate.asp by Don Clugston
//		horrible_cast< >
// This is truly evil. It completely subverts C++'s type system, allowing you
// to cast from any class to any other class. Technically, using a union
// to perform the cast is undefined behaviour (even in C). But we can see if
// it is OK by checking that the union is the same size as each of its members.
// horrible_cast<> should only be used for compiler-specific workarounds.
// Usage is identical to reinterpret_cast<>.

// This union is declared outside the horrible_cast because BCC 5.5.1
// can't inline a function with a nested class, and gives a warning.
template <class OutputClass, class InputClass>
union horrible_union{
	OutputClass out;
	InputClass in;
};

template <class OutputClass, class InputClass>
inline OutputClass horrible_cast(const InputClass input){
	horrible_union<OutputClass, InputClass> u;
	// Cause a compile-time error if in, out and u are not the same size.
	// If the compile fails here, it means the compiler has peculiar
	// unions which would prevent the cast from working.
	typedef int ERROR_CantUseHorrible_cast[sizeof(InputClass)==sizeof(u)
		/*&&  sizeof(InputClass)==sizeof(OutputClass)*/ ? 1 : -1];
	ERROR_CantUseHorrible_cast a;
	(void) a;
	u.in = input;
	return u.out;
}


template<class R>
struct FUNCTION_RETURN
{
	static R getDefaultValue()
	{
		R* ptr = new(std::nothrow) R();
		R tmp(*ptr);
		delete ptr;
		return tmp;
	}
};

//Function0
#define FUNCTION_NUMBER 0
#define FUNCTION_CLASS_TYPES typename R
#define FUNCTION_TYPES
#define FUNCTION_TYPE_ARGS
#define FUNCTION_ARGS
#define FUNCTION_STDFUNCTIONPTR	R(FUNCTION_TYPES) 
#define FUNCTION_STDPLACEHOLDERS 
#include "FuncTempl.h"
#undef  FUNCTION_NUMBER
#undef  FUNCTION_CLASS_TYPES
#undef	FUNCTION_TYPES
#undef	FUNCTION_TYPE_ARGS
#undef	FUNCTION_ARGS
#undef  FUNCTION_STDFUNCTIONPTR
#undef  FUNCTION_STDPLACEHOLDERS

//Function1
#define FUNCTION_NUMBER 1
#define FUNCTION_CLASS_TYPES typename R, typename T1
#define FUNCTION_TYPES T1
#define FUNCTION_TYPE_ARGS T1 a1
#define FUNCTION_ARGS a1
#define FUNCTION_STDFUNCTIONPTR	R(FUNCTION_TYPES) 
#define FUNCTION_STDPLACEHOLDERS std::placeholders::_1
#include "FuncTempl.h"
#undef  FUNCTION_NUMBER
#undef  FUNCTION_CLASS_TYPES
#undef	FUNCTION_TYPES
#undef	FUNCTION_TYPE_ARGS
#undef	FUNCTION_ARGS
#undef  FUNCTION_STDFUNCTIONPTR
#undef  FUNCTION_STDPLACEHOLDERS

//Function2
#define FUNCTION_NUMBER 2
#define FUNCTION_CLASS_TYPES typename R, typename T1, typename T2
#define FUNCTION_TYPES T1, T2
#define FUNCTION_TYPE_ARGS T1 a1, T2 a2
#define FUNCTION_ARGS a1, a2
#define FUNCTION_STDFUNCTIONPTR	R(FUNCTION_TYPES) 
#define FUNCTION_STDPLACEHOLDERS std::placeholders::_1, std::placeholders::_2
#include "FuncTempl.h"
#undef  FUNCTION_NUMBER
#undef  FUNCTION_CLASS_TYPES
#undef	FUNCTION_TYPES
#undef	FUNCTION_TYPE_ARGS
#undef	FUNCTION_ARGS
#undef  FUNCTION_STDFUNCTIONPTR
#undef  FUNCTION_STDPLACEHOLDERS

//Function3
#define FUNCTION_NUMBER 3
#define FUNCTION_CLASS_TYPES typename R, typename T1, typename T2, typename T3
#define FUNCTION_TYPES T1, T2, T3
#define FUNCTION_TYPE_ARGS T1 a1, T2 a2, T3 a3
#define FUNCTION_ARGS a1, a2, a3
#define FUNCTION_STDFUNCTIONPTR	R(FUNCTION_TYPES) 
#define FUNCTION_STDPLACEHOLDERS std::placeholders::_1, std::placeholders::_2, std::placeholders::_3
#include "FuncTempl.h"
#undef  FUNCTION_NUMBER
#undef  FUNCTION_CLASS_TYPES
#undef	FUNCTION_TYPES
#undef	FUNCTION_TYPE_ARGS
#undef	FUNCTION_ARGS
#undef  FUNCTION_STDFUNCTIONPTR
#undef  FUNCTION_STDPLACEHOLDERS

//Function4
#define FUNCTION_NUMBER 4
#define FUNCTION_CLASS_TYPES typename R, typename T1, typename T2, typename T3, typename T4
#define FUNCTION_TYPES T1, T2, T3, T4
#define FUNCTION_TYPE_ARGS T1 a1, T2 a2, T3 a3, T4 a4
#define FUNCTION_ARGS a1, a2, a3, a4
#define FUNCTION_STDFUNCTIONPTR	R(FUNCTION_TYPES) 
#define FUNCTION_STDPLACEHOLDERS std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4
#include "FuncTempl.h"
#undef  FUNCTION_NUMBER
#undef  FUNCTION_CLASS_TYPES
#undef	FUNCTION_TYPES
#undef	FUNCTION_TYPE_ARGS
#undef	FUNCTION_ARGS
#undef  FUNCTION_STDFUNCTIONPTR
#undef  FUNCTION_STDPLACEHOLDERS

//Function5
#define FUNCTION_NUMBER 5
#define FUNCTION_CLASS_TYPES typename R, typename T1, typename T2, typename T3, typename T4, typename T5
#define FUNCTION_TYPES T1, T2, T3, T4, T5
#define FUNCTION_TYPE_ARGS T1 a1, T2 a2, T3 a3, T4 a4, T5 a5
#define FUNCTION_ARGS a1, a2, a3, a4, a5
#define FUNCTION_STDFUNCTIONPTR	R(FUNCTION_TYPES) 
#define FUNCTION_STDPLACEHOLDERS std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4, std::placeholders::_5
#include "FuncTempl.h"
#undef  FUNCTION_NUMBER
#undef  FUNCTION_CLASS_TYPES
#undef	FUNCTION_TYPES
#undef	FUNCTION_TYPE_ARGS
#undef	FUNCTION_ARGS
#undef  FUNCTION_STDFUNCTIONPTR
#undef  FUNCTION_STDPLACEHOLDERS

//Function6
#define FUNCTION_NUMBER 6
#define FUNCTION_CLASS_TYPES typename R, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6
#define FUNCTION_TYPES T1, T2, T3, T4, T5, T6
#define FUNCTION_TYPE_ARGS T1 a1, T2 a2, T3 a3, T4 a4, T5 a5, T6 a6
#define FUNCTION_ARGS a1, a2, a3, a4, a5, a6
#define FUNCTION_STDFUNCTIONPTR	R(FUNCTION_TYPES) 
#define FUNCTION_STDPLACEHOLDERS std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4, std::placeholders::_5, std::placeholders::_6
#include "FuncTempl.h"
#undef  FUNCTION_NUMBER
#undef  FUNCTION_CLASS_TYPES
#undef	FUNCTION_TYPES
#undef	FUNCTION_TYPE_ARGS
#undef	FUNCTION_ARGS
#undef  FUNCTION_STDFUNCTIONPTR
#undef  FUNCTION_STDPLACEHOLDERS


} // namespace Base
} // namespace Public


#endif //__BASE_FUNCTION_H__

