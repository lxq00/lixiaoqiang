//
//  Copyright (c)1998-2014, Public Technology
//  All Rights Reserved.
//
//	Description:
//	$Id: Defs.h 216 2013-09-17 01:27:49Z  $
//


#ifndef __SQLITE_DEFS_H__
#define __SQLITE_DEFS_H__

// WIN32 Dynamic Link Library
#ifdef _MSC_VER

#ifdef SQLITE_DLL_BUILD
#define  SQLITE_API _declspec(dllexport)
#define  SQLITE_CALLBACK CALLBACK
#else
#define  SQLITE_API _declspec(dllimport)
#define	 SQLITE_CALLBACK CALLBACK 
#endif

#else

#define SQLITE_API
#define SQLITE_CALLBACK 
#endif

#ifndef NULL
#define NULL 0
#endif

#endif //__BASE_DEFS_H__

