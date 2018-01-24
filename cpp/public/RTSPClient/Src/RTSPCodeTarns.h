#ifndef __RTSPCODETRANS_H__
#define __RTSPCODETRANS_H__
#include "RTSPClient/RTSPStructs.h"
#include "Base/Base.h"


static int GetAudioCodec(int nPayLoad, char* pCodc, int &nAudioFormat, int &nChannel, int &nSampRate)
{
	if (nPayLoad < 0 && NULL == pCodc)
	{
		return -1;
	}
	switch (nPayLoad)
	{

	case AUDIO_PCMU_8000_1:
		nAudioFormat = XM_PLAYER_CODEID_G711Mu;
		nChannel = 1;
		nSampRate = 8000;
		break;
	case AUDIO_1016_8000_1:
		nAudioFormat = XM_PLAYER_CODEID_1016;
		nChannel = 1;
		nSampRate = 8000;
		break;
	case AUDIO_G721_8000_1:
		nAudioFormat = XM_PLAYER_CODEID_G721;
		nChannel = 1;
		break;
	case AUDIO_GSM_8000_1:
		nAudioFormat = XM_PLAYER_CODEID_GSM;
		nChannel = 1;
		nSampRate = 8000;
		break;
	case AUDIO_G723_8000_1:
		nAudioFormat = XM_PLAYER_CODEID_G723;
		nChannel = 1;
		nSampRate = 8000;
		break;
	case AUDIO_DVI4_8000_1:
		nSampRate = 8000;
		nAudioFormat = XM_PLAYER_CODEID_DVI4;
		nChannel = 1;
		break;
	case AUDIO_DVI4_16000_1:
		nSampRate = 1600;
		nAudioFormat = XM_PLAYER_CODEID_DVI4;
		nChannel = 1;
		break;
	case AUDIO_DVI4_11025_1:
		nSampRate = 11025;
		nAudioFormat = XM_PLAYER_CODEID_DVI4;
		nChannel = 1;
		break;
	case AUDIO_DVI4_22050_1:
		nSampRate = 22050;
		nAudioFormat = XM_PLAYER_CODEID_DVI4;
		nChannel = 1;
		break;
	case AUDIO_LPC_8000_1:
		nAudioFormat = XM_PLAYER_CODEID_LPC;
		nChannel = 1;
		nSampRate = 8000;
		break;
	case AUDIO_PCMA_8000_1:
		nAudioFormat = XM_PLAYER_CODEID_G711A;
		nChannel = 1;
		nSampRate = 8000;
		break;
	case AUDIO_G722_8000_1:
		nAudioFormat = XM_PLAYER_CODEID_G722;
		nChannel = 1;
		nSampRate = 8000;
		break;
	case AUDIO_L16_44100_2:
		nAudioFormat = XM_PLAYER_CODEID_PCM;
		nChannel = 2;
		nSampRate = 44100;
		break;
	case AUDIO_L16_44100_1:
		nAudioFormat = XM_PLAYER_CODEID_PCM;
		nChannel = 1;
		nSampRate = 44100;
		break;
	case AUDIO_QCELP_8000_1:
		nAudioFormat = XM_PLAYER_CODEID_QCELP;
		nChannel = 1;
		nSampRate = 8000;
		break;
	case AUDIO_CN_8000_1:
		nAudioFormat = XM_PLAYER_CODEID_CN;
		nChannel = 1;
		nSampRate = 8000;
		break;
	case AUDIO_MPA_90000_1:
		nAudioFormat = XM_PLAYER_CODEID_MPA;
		nChannel = 1;
		nSampRate = 90000;
		break;
	case AUDIO_G728_8000_1:
		nAudioFormat = XM_PLAYER_CODEID_G728;
		nChannel = 1;
		break;
	case AUDIO_G729_8000_1:
		nChannel = 1;
		nSampRate = 8000;
		break;
	default://动态,需要使用字符串进行判断
	{	//比较13个字节字符串是否为"mpeg4-generic"
		if (0 == strncasecmp(pCodc, AUDIO_MPEG4_GENERIC, 13))
		{
			nAudioFormat = XM_PLAYER_CODEID_AAC;
			nChannel = 1;
			nSampRate = 8000;
		}
		else if (0 == strncasecmp(pCodc, AUDIO_G726, 4))
		{
			nAudioFormat = XM_PLAYER_CODEID_G726;
			nChannel = 1;
			nSampRate = 8000;
		}
		else if (0 == strncasecmp(pCodc, AUDIO_L16, 3))
		{
			nAudioFormat = XM_PLAYER_CODEID_L16;
			nChannel = 1;
			nSampRate = 8000;
		}
		else
		{
			nAudioFormat = -1;
		}
	}
	break;
	}

	return 0;
}

static int GetVideoCodec(int nPayLoad, char* pCodc, int &nVideoFormat)
{
	if (nPayLoad < 0 && NULL == pCodc)
	{
		return -1;
	}
	switch (nPayLoad)
	{
	case VIDEO_JPEG_90000:
		nVideoFormat = XM_PLAYER_CODEID_MJEPG;
		break;
	case VIDEO_MPV_90000:
		nVideoFormat = XM_PLAYER_CODEID_MJEPG;
		break;
	case VIDEO_MP2T_90000:
		nVideoFormat = XM_PLAYER_CODEID_MJEPG;
		break;
	case VIDEO_H261_90000:
		nVideoFormat = XM_PLAYER_CODEID_MJEPG;
		break;
	case VIDEO_H263_90000:
		nVideoFormat = XM_PLAYER_CODEID_MJEPG;
		break;
	default:
	{
		//比较4个字节字符串是否为"H264"
		if (0 == strncasecmp(pCodc, VIDEO_H264, 4))
		{
			nVideoFormat = XM_PLAYER_CODEID_H264;
		}
	}
	break;
	}

	return 0;
}

#endif //__RTSPCODETRANS_H__
