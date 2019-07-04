#ifndef __XML_DEFS_H__

//#ifdef WIN32
//#ifdef XML_DLL_BUILD
//#define  XML_API _declspec(dllexport)
//#else 
//#define  XML_API _declspec(dllimport)
//#endif
//#else
#define XML_API
//#endif

extern "C" XML_API void  printXMLLibVersion();

#ifdef WIN32
#pragma warning(disable:4251)
#endif

#endif 