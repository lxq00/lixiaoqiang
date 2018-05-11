//
//  Copyright (c)1998-2014, Public Technology
//  All Rights Reserved.
//
//	Description:
//	$Id: Defs.h 216 2013-09-17 01:27:49Z  $
//


#ifndef __PUBLIC_JSON_DEFS_H__
#define __PUBLIC_JSON_DEFS_H__

// WIN32 Dynamic Link Library
#ifdef _MSC_VER

#ifdef JSON_DLL_BUILD
#define  JSON_API _declspec(dllexport)
#else
#define  JSON_API _declspec(dllimport)
#endif

#else

#define JSON_API
#endif

#ifndef NULL
#define NULL 0
#endif

#ifdef WIN32
#pragma warning(disable:4251)
#endif

#endif //__BASE_DEFS_H__

