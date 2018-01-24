//
//  Copyright (c)1998-2014, Xunmei Technology
//  All Rights Reserved.
//
//	Description:
//	$Id: Defs.h 216 2013-09-17 01:27:49Z linyang $
//


#ifndef __BASE_DEFS_H__
#define __BASE_DEFS_H__

// WIN32 Dynamic Link Library
#ifdef _MSC_VER

#ifdef BASE_DLL_BUILD
#define  BASE_API _declspec(dllexport)
#define  BASE_CALLBACK CALLBACK
#else
#define  BASE_API _declspec(dllimport)
#define	 BASE_CALLBACK CALLBACK 
#endif

#else

#define BASE_API
#define BASE_CALLBACK 
#endif

#ifndef NULL
#define NULL 0
#endif

//////////////////////////////////////////////////////////////////////////
// useful definition

#define BITMSK(bit)				(int)(1 << (bit))

//////////////////////////////////////////////////////////////////////////
// Join two variables
#define MACRO_JOIN( X, Y ) MACRO_DO_JOIN( X, Y )
#define MACRO_DO_JOIN( X, Y ) MACRO_DO_JOIN2(X,Y)
#define MACRO_DO_JOIN2( X, Y ) X##Y


//////////////////////////////////////////////////////////////////////////
// use the unified 'DEBUG' macro
#if (!defined(NDEBUG)) && !defined(DEBUG)
#	define DEBUG
#endif

#ifdef WIN32
#pragma warning(disable:4251)
#endif

#endif //__BASE_DEFS_H__

