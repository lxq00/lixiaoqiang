//
//  Copyright (c)1998-2014, Public Technology
//  All Rights Reserved.
//
//	Description:
//	$Id: Defs.h 216 2013-09-17 01:27:49Z $
//


#ifndef __ABSTRUCT_DEFS_H__
#define __ABSTRUCT_DEFS_H__

// WIN32 Dynamic Link Library
#ifdef _MSC_VER

#ifdef ABSTRUCT_DLL_BUILD
#define  ABSTRUCT_API _declspec(dllexport)
#else
#define  ABSTRUCT_API 
#endif

#else

#define ABSTRUCT_API
#endif


#endif //__ABSTRUCT_DEFS_H__

