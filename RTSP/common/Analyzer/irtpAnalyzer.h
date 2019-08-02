#pragma once

#include "rtpAnalyzerStructs.h"

#ifndef _I_UNPACK_H_
#define _I_UNPACK_H_

#define VIEOD_FRAME_LEN		1024*1024
#define AUDIO_FRAME_LEN		1024*10
#define SHORT_BUFFER_LEN    128
#define MIDDLE_BUFFER_LEN   512
#define BIG_BUFFER_LEN		1024
#define LARG_BUFFER_LEN		2048

#define RTP_VERSION			2
#define RTP_HEADER_LEN		12

//帧类型定义
#define VIDEO_I_FRAME_TYPE	0
#define VIDEO_P_FRAME_TYPE	1
#define AUDIO_FRAME_TYPE	2
#define H264_SPS_FRAME_TYPE	3
#define H264_PPS_FRAME_TYPE	4
#define MJPEG_FRAME_TYPE	5


//数据回调
//const unsigned char *pBuf 帧数据
//int nFreamSize			帧长度
//int nFreamType			帧类型
//long lTimestamp			时间戳,如果是分包以第一个RTP包时间戳为准
typedef void (*CBFreamCallBack)(char *pFrameBuf, int nFrameSize, int nFreamType, long lTimestamp, uint64_t dwUser, void* lpUser);

class IRtpAnalyzer
{

public:
	///初始化分析器
	///@[in] nBufferSize 初始设置缓存去大小，设置为0 表示不是有缓存机制
	///@[in] nVideoType  视频荷载类型
	///@[in] nAudioType  音频荷载类型
	///@[return] -1 表示失败; 0 表示成功
	virtual int Init(int nBufferSize) = 0;

	///输入RTP包
	///@[in] pBuf RTP 包数据
	///@[in] nBufSize RTP 包长度
	///@[in] nOffset 传入buffer的偏移量
	///@[return] 0:传入数据不够一帧，继续等待数据, -1:出现错误。
	virtual int InputData(char *pBuf, unsigned short nBufSize, int nOffset) = 0;

	///设置帧数据回调
	///@[in] pBuf RTP 包数据
	///@[in] nBufSize RTP 包长度
	///@[in] nOffset 传入buffer的偏移量
	///@[out] pOutFream 完整的帧数据
	///@[out] nFreamSize 传出帧长度
	///@[return] 0:传入数据不够一帧，继续等待数据; 1:有完整的帧产生，通过 pOutFream和nFreamSize传出; -1:出现错误。
	virtual int SetFreamDataCallBack(CBFreamCallBack pCallBack, uint64_t dwUserData1, void* lpUserData2) = 0;

	///清空缓存区
	virtual int ResetBuffer() = 0;

	virtual void Free() = 0;

};

#endif