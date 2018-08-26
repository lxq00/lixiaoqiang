//
//  Copyright (c)1998-2012, Chongqing Technology
//  All Rights Reserved.
//
//	Description:
//	$Id: Defs.h 216 2013-09-17 01:27:49Z $
//


#ifndef __LOG_DEFS_H__
#define __LOG_DEFS_H__

// WIN32 Dynamic Link Library
#ifdef _MSC_VER

#ifdef LOG_DLL_BUILD
#define  LOG_API _declspec(dllexport)
#define  LOG_CALLBACK CALLBACK
#else
#define  LOG_API _declspec(dllimport)
#define	 LOG_CALLBACK CALLBACK
#endif

#else

#define LOG_API
#define LOG_CALLBACK 
#endif

#ifdef WIN32

#pragma warning(disable:4251)
#endif

#endif //__BASE_DEFS_H__

