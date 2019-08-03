#pragma once

#ifndef _I_RTP_UNPACK_H_
#define _I_RTP_UNPACK_H_

#include "RTSP/Defs.h"
#include "RTSP/RTSPStructs.h"
#include "Base/Base.h"
using namespace Public::Base;


namespace Public {
namespace RTSP {

//帧类型定义
typedef enum {
	FrameType_NONE = -1,
	FrameType_VIDEO_I_FRAME = 0 ,
	FrameType_VIDEO_P_FRAME = 1,
	FrameType_AUDIO_FRAME = 2,
	FrameType_H264_SPS_FRAME = 3,
	FrameType_H264_PPS_FRAME = 4,
	FrameType_MJPEG_FRAME = 5,
	FrameType_VIDEO_FRAME = 6,
}FrameType;


//将RTP包数据进行分解为相应的流数据
class RTSP_API RTPAnalyzer
{
public:
	//数据回调
//const unsigned char *pBuf 帧数据
//int nFreamSize			帧长度
//int nFreamType			帧类型
//long lTimestamp			时间戳,如果是分包以第一个RTP包时间戳为准
	typedef Function4<void, FrameType, const char*, uint32_t, long> FrameCallback;
public:
	RTPAnalyzer(const shared_ptr<STREAM_TRANS_INFO>& transportinfo,const FrameCallback& framcallback);
	~RTPAnalyzer();

	///输入RTP包
	///@[in] pBuf RTP 包数据
	///@[in] nBufSize RTP 包长度
	///@[in] nOffset 传入buffer的偏移量
	///@[return] 0:传入数据不够一帧，继续等待数据, -1:出现错误。
	bool inputRtpPacket(const RTPHEADER& rtpheader, const char* bufferaddr, uint32_t bufferlen);

	///清空缓存区
	bool reset();
private:
	struct RTPAnalyzerInternal;
	RTPAnalyzerInternal* internal;

};

}
}


#endif