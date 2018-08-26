#ifndef __INTERPROTOCOL_DEFS_H__
#define __INTERPROTOCOL_DEFS_H__


#ifdef INTERPROTOCOL_BUILD
#define INTERPROTOCOL_API __declspec(dllexport)
#else 
#ifdef INTERPROTOCOL_USED
#define INTERPROTOCOL_API __declspec(dllimport)
#else
#define INTERPROTOCOL_API
#endif
#endif



#endif

