#pragma  once
#include "Base/Base.h"
#include "RTSP/RTSPStructs.h"
using namespace Public::Base;
using namespace Public::RTSP;

class RTPSession
{
public:
	//Fucntion4<bool isvideo,uint32_t timestmap,const char* buffer,uint32_t bufferlen,bool mark>RTPDataCallback
	typedef Function5<void, const shared_ptr<STREAM_TRANS_INFO>&,uint32_t,const char*, uint32_t, bool> MediaDataCallback;

	//控制数据回调
	typedef Function3<void, const shared_ptr<STREAM_TRANS_INFO>&,const char*, uint32_t> ContorlDataCallback;
public:
	RTPSession(const shared_ptr<STREAM_TRANS_INFO>& _transport,const MediaDataCallback& _datacallback,const ContorlDataCallback& _contorlcallback)
		:transportinfo(_transport),datacallback(_datacallback), contorlcallback(_contorlcallback){}
	virtual ~RTPSession() {}

	virtual void rtpovertcpContorlCallback(const shared_ptr<STREAM_TRANS_INFO>& mediainfo, const char*  buffer, uint32_t bufferlen){}
	virtual void rtpovertcpMediaCallback(const shared_ptr<STREAM_TRANS_INFO>& mediainfo, const RTPHEADER& rtpheader, const char*  buffer, uint32_t bufferlen) {}

	virtual void sendContorlData(const shared_ptr<STREAM_TRANS_INFO>& transportinfo, const char*  buffer, uint32_t bufferlen) = 0;
	virtual void sendMediaData(const shared_ptr<STREAM_TRANS_INFO>& transportinfo, uint32_t timestmap, const char*  buffer, uint32_t bufferlen, bool mark) = 0;
	virtual bool onPoolHeartbeat() { return false; }
protected:
	shared_ptr<STREAM_TRANS_INFO>	transportinfo;
	MediaDataCallback				datacallback;
	ContorlDataCallback				contorlcallback;
};