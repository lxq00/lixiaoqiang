//
//  Copyright (c)1998-2014, Xunmei Technology
//  All Rights Reserved.
//
//	Description:
//	$Id: Defs.h 216 2013-09-17 01:27:49Z linyang $
//


#ifndef __HTTP_DEFS_H__
#define __HTTP_DEFS_H__

// WIN32 Dynamic Link Library
#ifdef _MSC_VER

#ifdef HTTP_DLL_BUILD
#define  HTTP_API _declspec(dllexport)
#else
#define  HTTP_API _declspec(dllimport)
#endif

#else

#define HTTP_API
#define HTTP_CALLBACK 
#endif


#endif //__BASE_DEFS_H__

