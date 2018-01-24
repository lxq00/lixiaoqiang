//
//  Copyright (c)1998-2012, Chongqing Xunmei Technology
//  All Rights Reserved.
//
//	Description:
//	$Id: Defs.h 216 2013-09-17 01:27:49Z linyang $
//


#ifndef __NETWORK_DEFS_H__
#define __NETWORK_DEFS_H__

// WIN32 Dynamic Link Library
#ifdef _MSC_VER

#ifdef NETWORK_DLL_BUILD
#define  NETWORK_API _declspec(dllexport)
#define  NETWORK_CALLBACK CALLBACK
#else
#define  NETWORK_API _declspec(dllimport)
#define	 NETWORK_CALLBACK CALLBACK
#endif

#else

#define NETWORK_API
#define NETWORK_CALLBACK 
#endif

#ifdef WIN32
#pragma warning(disable:4251)
#endif
#endif //__BASE_DEFS_H__

