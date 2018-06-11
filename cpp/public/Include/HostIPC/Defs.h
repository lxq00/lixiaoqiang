//
//  Copyright (c)1998-2012, Public Technology
//  All Rights Reserved.
//
//	Description:
//	$Id: Defs.h 216 2013-09-17 01:27:49Z $
//


#ifndef __HostIPC_DEFS_H__
#define __HostIPC_DEFS_H__

// WIN32 Dynamic Link Library
#ifdef _MSC_VER

#ifdef HOSTIPC_DLL_BUILD
#define  HostIPC_API _declspec(dllexport)
#else
#define  HostIPC_API _declspec(dllimport)
#endif

#else

#define HostIPC_API
#endif

#ifdef WIN32
#pragma warning(disable:4251)
#endif
#endif //__HostIPC_DEFS_H__

