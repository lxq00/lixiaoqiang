#include "RtpPackage.h"
#include <WinSock2.h>

typedef struct
{
	unsigned char csrc_len:4;
	unsigned char extension:1;
	unsigned char padding:1;
	unsigned char version:2;
	unsigned char payload:7;
	unsigned char marker:1;
	unsigned short seq_no;
	unsigned  long timestamp;
	unsigned long ssrc;
} RTP_FIXED_HEADER, *LPRTP_FIXED_HEADER;

typedef struct {
	unsigned char TYPE:5;
	unsigned char NRI:2;
	unsigned char F:1;
} NALU_HEADER, *LPNALU_HEADER;

typedef struct {
	unsigned char TYPE:5;
	unsigned char NRI:2;
	unsigned char F:1;
} FU_INDICATOR, *LPFU_INDICATOR;

typedef struct {
	unsigned char TYPE:5;
	unsigned char R:1;
	unsigned char E:1;
	unsigned char S:1;
} FU_HEADER, *LPFU_HEADER;

static int FindStartCode2 (unsigned char *Buf)
{
	if(Buf[0]!=0 || Buf[1]!=0 || Buf[2] !=1) return 0; //判断是否为0x000001,如果是返回1
	else return 3;
}

static int FindStartCode3 (unsigned char *Buf)
{
	if(Buf[0]!=0 || Buf[1]!=0 || Buf[2] !=0 || Buf[3] !=1) return 0;//判断是否为0x00000001,如果是返回1
	else return 4;
}


RtpPackage::RtpPackage(const RtpPackageCallback &callback)
	: rtpcallback(callback)
	, pVideoBuffer(NULL)
	, nVideobufSize(0)
	, audio_seq_num(0)
	, video_seq_num(0)
	, nFrameRate(25)
	, curtimestamp(0)
	, nAudiobufSize(0)
	, pAudioBuffer(NULL)
	, nSamplingRate(8000)
{

}


RtpPackage::~RtpPackage()
{
	if (pVideoBuffer != NULL)
	{
		delete[] pVideoBuffer;
		pVideoBuffer = NULL;
	}
}

void RtpPackage::SetFrameRate(int framerate)
{
	nFrameRate = framerate;
}

int RtpPackage::GetFrameRate()
{
	return nFrameRate;
}

void RtpPackage::SetSamplingRate(int samplingRate)
{
	nSamplingRate = samplingRate;
}

int RtpPackage::GetSamplingRate()
{
	return nSamplingRate;
}

bool RtpPackage::startOfFile(const std::string &filename)
{
	return false;
}

#define DEFUALT_PACKAGE_SIZE		1400
bool RtpPackage::InputVideoData(unsigned char* data, int size)
{
	if (size <= 4)
	{
		return false;
	}

	int naulstartPos = FindStartCode2(data);
	if (naulstartPos == 0)
	{
		naulstartPos = FindStartCode3(data);
		if (naulstartPos == 0)
		{
			return false;
		}
	}

	int maxSize = size + sizeof(RTP_FIXED_HEADER) + sizeof(FU_INDICATOR) + sizeof(FU_HEADER);
	if (nVideobufSize < maxSize)
	{
		if (pVideoBuffer != NULL)
		{
			delete[] pVideoBuffer;
			pVideoBuffer = NULL;
		}
	}
	
	if (pVideoBuffer == NULL)
	{
		nVideobufSize = maxSize;
		pVideoBuffer = new char[nVideobufSize];
		memset(pVideoBuffer, 0, nVideobufSize);
	}

	int packSize = 0;
	LPRTP_FIXED_HEADER lpRtp_hdr = (LPRTP_FIXED_HEADER)pVideoBuffer;
	packSize += sizeof(RTP_FIXED_HEADER);
	lpRtp_hdr->payload = 96;		// H264
	lpRtp_hdr->version = 2;
	lpRtp_hdr->marker = 0;
	lpRtp_hdr->ssrc = htonl(10);
	int dataSize = size - naulstartPos;
	char nalu = data[naulstartPos];
	curtimestamp += 90000 / nFrameRate;
	lpRtp_hdr->timestamp = htonl(curtimestamp);
	if (dataSize <= DEFUALT_PACKAGE_SIZE)			// 分片
	{
		lpRtp_hdr->seq_no = htons(video_seq_num++);
		lpRtp_hdr->marker = 1;
		LPNALU_HEADER lpNalu_hdr = (LPNALU_HEADER)(pVideoBuffer + sizeof(RTP_FIXED_HEADER));
		packSize += sizeof(LPNALU_HEADER);
		lpNalu_hdr->F = nalu >> 7 & 1;
		lpNalu_hdr->NRI = nalu >> 5 & 3;
		lpNalu_hdr->TYPE = nalu & 0x1F;

		char *payload = (char*)(lpNalu_hdr + 1);
		int leng = naulstartPos + 1;
		packSize += size - leng;
		memcpy(payload, data + leng, size - leng);
		if (!rtpcallback.empty())
		{
			rtpcallback(pVideoBuffer, packSize, NULL, true);
		}
		return true;
	}

	int k = dataSize / DEFUALT_PACKAGE_SIZE;
	int l = dataSize % DEFUALT_PACKAGE_SIZE;
	int n = 0;
	while (k >= n)
	{
		packSize = sizeof(RTP_FIXED_HEADER);
		lpRtp_hdr->marker = 0;
		lpRtp_hdr->seq_no = htons(video_seq_num++);
		FU_INDICATOR *lpFu_ind = (FU_INDICATOR*)(pVideoBuffer + sizeof(RTP_FIXED_HEADER));
		packSize += sizeof(FU_INDICATOR);
		lpFu_ind->F = nalu >> 7 & 1;
		lpFu_ind->NRI = nalu >> 5 & 3;
		lpFu_ind->TYPE = 28;

		FU_HEADER *lpFu_hdr = (FU_HEADER*)(lpFu_ind + 1);
		packSize += sizeof(FU_HEADER);
		lpFu_hdr->R = 0;
		lpFu_hdr->E = 0;
		lpFu_hdr->S = 0;
		int payloadSize = DEFUALT_PACKAGE_SIZE;
		if (n == 0)
		{
			lpFu_hdr->S = 1;
		}

		if (n == k)
		{
			lpRtp_hdr->marker = 1;
			lpFu_hdr->E = 1;
			if (l == 0)
			{
				break;
			}
			payloadSize = l - 1;
		}
		packSize += payloadSize;
		lpFu_hdr->TYPE = nalu & 0x1F;

		char *plyload = (char*)(lpFu_hdr + 1);
		int leng = naulstartPos + 1;
		memcpy(plyload, data + leng + n * DEFUALT_PACKAGE_SIZE, payloadSize);
		rtpcallback(pVideoBuffer, packSize, NULL, true);
		n++;
	}
	return true;
}

bool RtpPackage::InputAudioData(unsigned char* data, int size)
{
	int maxSize = size + sizeof(RTP_FIXED_HEADER) + sizeof(FU_INDICATOR) + sizeof(FU_HEADER);
	if (nAudiobufSize < maxSize)
	{
		if (pAudioBuffer != NULL)
		{
			delete[] pAudioBuffer;
			pAudioBuffer = NULL;
		}
	}

	if (pAudioBuffer == NULL)
	{
		nAudiobufSize = maxSize;
		pAudioBuffer = new char[nAudiobufSize];
		memset(pAudioBuffer, 0, nAudiobufSize);
	}

	int packSize = 0;
	LPRTP_FIXED_HEADER lpRtp_hdr = (LPRTP_FIXED_HEADER)pAudioBuffer;
	packSize += sizeof(RTP_FIXED_HEADER);
	lpRtp_hdr->payload = 8;		// G711A
	lpRtp_hdr->version = 2;
	lpRtp_hdr->marker = 1;
	lpRtp_hdr->ssrc = htonl(10);
	int time = (int)(((float)curtimestamp / 3600 / nFrameRate) * nSamplingRate);
	lpRtp_hdr->timestamp = htonl(time);
	lpRtp_hdr->seq_no = htons(audio_seq_num++);

	char *payload = (char*)(lpRtp_hdr + 1);
	packSize += size;
	memcpy(payload, data, size);
	if (!rtpcallback.empty())
	{
		rtpcallback(pAudioBuffer, packSize, NULL, false);
	}
	return true;

}