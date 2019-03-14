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

#ifdef MYSQL_DLL_BUILD
#define  MYSQL_API _declspec(dllexport)
#define  MYSQL_CALLBACK CALLBACK
#else
#define  MYSQL_API _declspec(dllimport)
#define	 MYSQL_CALLBACK CALLBACK 
#endif

#else

#define MYSQL_API
#define MYSQL_CALLBACK 
#endif

#ifndef NULL
#define NULL 0
#endif

#endif //__BASE_DEFS_H__

