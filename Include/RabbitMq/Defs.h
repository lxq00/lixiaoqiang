//
//  Copyright (c)1998-2014, Public Technology
//  All Rights Reserved.
//
//	Description:
//	$Id: Defs.h 216 2013-09-17 01:27:49Z  $
//


#ifndef __RABBITMQ_DEFS_H__
#define __RABBITMQ_DEFS_H__

// WIN32 Dynamic Link Library
#ifdef _MSC_VER

#ifdef RABBITMQ_DLL_BUILD
#define  RABBITMQ_API _declspec(dllexport)
#define  RABBITMQ_CALLBACK CALLBACK
#else
#define  RABBITMQ_API _declspec(dllimport)
#define	 RABBITMQ_CALLBACK CALLBACK 
#endif

#else

#define RABBITMQ_API
#define RABBITMQ_CALLBACK 
#endif

#ifndef NULL
#define NULL 0
#endif

#endif //__BASE_DEFS_H__

