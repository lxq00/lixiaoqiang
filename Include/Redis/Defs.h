#pragma once

#ifdef WIN32
	#ifdef REDIS_EXPORTS
		#define REDIS_API __declspec(dllexport)
	#else
		#define REDIS_API __declspec(dllimport)
	#endif
#else
	#define REDIS_API
#endif

