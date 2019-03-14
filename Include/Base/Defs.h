//
//  Copyright (c)1998-2014, Public Technology
//  All Rights Reserved.
//
//	Description:
//	$Id: Defs.h 216 2013-09-17 01:27:49Z  $
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
#pragma warning(disable:4786)
#pragma warning(disable:4091)
#endif

#ifdef WIN32

/*
MS VC++ 14.1 _MSC_VER = 1917 (Visual Studio 2017)
MS VC++ 14.0 _MSC_VER = 1900 (Visual Studio 2015)
MS VC++ 12.0 _MSC_VER = 1800 (VisualStudio 2013)
MS VC++ 11.0 _MSC_VER = 1700 (VisualStudio 2012)
MS VC++ 10.0 _MSC_VER = 1600(VisualStudio 2010)
MS VC++ 9.0 _MSC_VER = 1500(VisualStudio 2008)
MS VC++ 8.0 _MSC_VER = 1400(VisualStudio 2005)
MS VC++ 7.1 _MSC_VER = 1310(VisualStudio 2003)
MS VC++ 7.0 _MSC_VER = 1300(VisualStudio .NET)
MS VC++ 6.0 _MSC_VER = 1200(VisualStudio 98)
MS VC++ 5.0 _MSC_VER = 1100(VisualStudio 97)
*/
#if _MSC_VER > 1500 

#define GCCSUPORTC11	//是否支持C++标准
#define STDSUPORTMOVE	//是否支持std::move接口

#endif

#else

#define GCC_VERSION (__GNUC__ * 10000 \
                     + __GNUC_MINOR__ * 100 \
                     + __GNUC_PATCHLEVEL__)

/* Test for GCC > 3.2.0 */
//#if GCC_VERSION > 30200

#if GCC_VERSION   >= 40800
#define GCCSUPORTC11
#endif

#if GCC_VERSION >= 50000
#define STDSUPORTMOVE
#endif

#endif



#endif //__BASE_DEFS_H__

