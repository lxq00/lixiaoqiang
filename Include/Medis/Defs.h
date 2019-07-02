//
//  Copyright (c)1998-2014, Public Technology
//  All Rights Reserved.
//
//	Description:
//	$Id: Defs.h 216 2013-09-17 01:27:49Z  $
//


#ifndef __MEDIS_DEFS_H__
#define __MEDIS_DEFS_H__

// WIN32 Dynamic Link Library
#ifdef _MSC_VER

#ifdef MEDIS_DLL_BUILD
#define  MEDIS_API _declspec(dllexport)
#define  MEDIS_CALLBACK CALLBACK
#else
#define  MEDIS_API _declspec(dllimport)
#define	 MEDIS_CALLBACK CALLBACK 
#endif

#else

#define MEDIS_API
#define MEDIS_CALLBACK 
#endif


#endif //

