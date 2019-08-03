#pragma once

#ifndef _I_UNPACK_H_
#define _I_UNPACK_H_

#include "rtpAnalyzerStructs.h"
#include "RTSP/RTPAnalyzer.h"
using namespace Public::Base;
using namespace Public::RTSP;

#define VIEOD_FRAME_LEN		1024*1024
#define AUDIO_FRAME_LEN		1024*10
#define SHORT_BUFFER_LEN    128
#define MIDDLE_BUFFER_LEN   512
#define BIG_BUFFER_LEN		1024
#define LARG_BUFFER_LEN		2048


typedef RTPAnalyzer::FrameCallback CBFreamCallBack;

class IRtpAnalyzer
{

public:
	IRtpAnalyzer() {}
	virtual ~IRtpAnalyzer() {}

	///输入RTP包
	///@[in] pBuf RTP 包数据
	///@[in] nBufSize RTP 包长度
	///@[in] nOffset 传入buffer的偏移量
	///@[return] 0:传入数据不够一帧，继续等待数据, -1:出现错误。
	virtual int InputData(const RTPHEADER& rtpheader,const char *pBuf, unsigned short nBufSize) = 0;
};

#endif