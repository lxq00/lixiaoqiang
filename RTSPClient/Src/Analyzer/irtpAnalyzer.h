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

//֡���Ͷ���
#define VIDEO_I_FRAME_TYPE	0
#define VIDEO_P_FRAME_TYPE	1
#define AUDIO_FRAME_TYPE	2
#define H264_SPS_FRAME_TYPE	3
#define H264_PPS_FRAME_TYPE	4
#define MJPEG_FRAME_TYPE	5


//���ݻص�
//const unsigned char *pBuf ֡����
//int nFreamSize			֡����
//int nFreamType			֡����
//long lTimestamp			ʱ���,����Ƿְ��Ե�һ��RTP��ʱ���Ϊ׼
typedef void (*CBFreamCallBack)(char *pFrameBuf, int nFrameSize, int nFreamType, long lTimestamp, uint64_t dwUser, void* lpUser);

class IRtpAnalyzer
{

public:
	///��ʼ��������
	///@[in] nBufferSize ��ʼ���û���ȥ��С������Ϊ0 ��ʾ�����л������
	///@[in] nVideoType  ��Ƶ��������
	///@[in] nAudioType  ��Ƶ��������
	///@[return] -1 ��ʾʧ��; 0 ��ʾ�ɹ�
	virtual int Init(int nBufferSize) = 0;

	///����RTP��
	///@[in] pBuf RTP ������
	///@[in] nBufSize RTP ������
	///@[in] nOffset ����buffer��ƫ����
	///@[return] 0:�������ݲ���һ֡�������ȴ�����, -1:���ִ���
	virtual int InputData(char *pBuf, unsigned short nBufSize, int nOffset) = 0;

	///����֡���ݻص�
	///@[in] pBuf RTP ������
	///@[in] nBufSize RTP ������
	///@[in] nOffset ����buffer��ƫ����
	///@[out] pOutFream ������֡����
	///@[out] nFreamSize ����֡����
	///@[return] 0:�������ݲ���һ֡�������ȴ�����; 1:��������֡������ͨ�� pOutFream��nFreamSize����; -1:���ִ���
	virtual int SetFreamDataCallBack(CBFreamCallBack pCallBack, uint64_t dwUserData1, void* lpUserData2) = 0;

	///��ջ�����
	virtual int ResetBuffer() = 0;

	virtual void Free() = 0;

};

#endif