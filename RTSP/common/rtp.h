#pragma  once
#include "Base/Base.h"
#include "RTSP/RTSPStructs.h"
using namespace Public::Base;

typedef struct  _RTPHEADER
{
	unsigned short   cc : 4;
	unsigned short   x : 1;
	unsigned short   p : 1;
	unsigned short   v : 2;
	unsigned short   pt : 7;
	unsigned short   m : 1;

	unsigned short   seq;
	unsigned long    ts;
	unsigned long    ssrc;

}RTPHEADER, * LPRTPHEADER;

#define MAXRTPPACKETLEN		1240

class rtp
{
public:
	//Fucntion4<bool isvideo,uint32_t timestmap,const char* buffer,uint32_t bufferlen>RTPDataCallback
	typedef Function4<void, bool,uint32_t, const char*, uint32_t> RTPDataCallback;
public:
	rtp(bool _isserver,const RTSP_MEDIA_INFO& _rtspmedia, const RTPDataCallback& _datacallback)
		:isserver(_isserver), rtspmedia(_rtspmedia),datacallback(_datacallback) {}
	virtual ~rtp() {}

	virtual void sendData(bool isvideo, uint32_t timesmap, const char* buffer, uint32_t bufferlen) = 0;
protected:
	bool			isserver;
	RTPDataCallback	datacallback;
	RTSP_MEDIA_INFO	rtspmedia;
};