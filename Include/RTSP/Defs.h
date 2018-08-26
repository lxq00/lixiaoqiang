#ifndef __PUBLIC_RTSP_H__
#define __PUBLIC_RTSP_H__
#include "RTSPStructs.h"

#ifdef WIN32
	#ifdef RTSP_EXPORTS
		#define RTSP_API __declspec(dllexport)
	#else
		#define RTSP_API __declspec(dllimport)
	#endif
#else
	#define RTSP_API
#endif


#endif