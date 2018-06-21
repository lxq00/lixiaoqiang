#pragma once

#ifdef WIN32
	#ifdef RTMP_EXPORTS
		#define RTMP_API __declspec(dllexport)
	#else
		#define RTMP_API __declspec(dllimport)
	#endif
#else
	#define RTMP_API
#endif

typedef enum RTMPType
{
	FlvTagAudio,
	FlvTagVideo,
	FlvTagScript,
};