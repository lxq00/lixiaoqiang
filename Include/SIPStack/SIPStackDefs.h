#ifndef __SIPSTACK_DEFS_H__
#define __SIPSTACK_DEFS_H__
#include "Base/Base.h"
using namespace Public::Base;

#ifdef _MSC_VER
#ifdef SIPSTACK_BUILD
#define  SIP_API _declspec(dllexport)
#else
#define  SIP_API _declspec(dllimport)
#endif

#else

#define SIP_API
#endif

namespace Public{
namespace SIPStack {

typedef enum {
	INVITETYPE_REALPLAY,
	INVITETYPE_PLAYBACK,
	INVITETYPE_DOWNLOAD,
	INVITETYPE_PROXY,
}INVITETYPE_E;

enum StreamSessionType
{
	StreamSessionType_Realplay = INVITETYPE_REALPLAY,
	StreamSessionType_VOD = INVITETYPE_PLAYBACK,
	StreamSessionType_Download = INVITETYPE_DOWNLOAD,
	StreamSessionType_ThirdCall,
};

struct SDPMediaInfo
{
	int 			Platload;
	std::string		MediaType;
};

enum SIPSocketType
{
	SocketType_UDP = 0,
	SocketType_TCP,
};

class StreamSessionSDP
{
public:
	StreamSessionSDP()
	{
		vodStartTime = 0;
		VodEndTime = 0;
		downSpeed = 1;
		streamType = 0;
	}
public:
	std::string 				proxyId;
	StreamSessionType			type;
	std::string 				deviceId;
	std::string 				ipAddr;
	std::string					ssrc;
	int							port;
	int							streamType;
	long 						vodStartTime;
	long 						VodEndTime;
	int							downSpeed;
	std::list<SDPMediaInfo>		MediaList;
	SIPSocketType				SocketType;
};

};
};



#endif //__SIPSTACK_DEFS_H__
