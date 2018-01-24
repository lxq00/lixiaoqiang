//
//  Copyright (c)1998-2012, Chongqing Xunmei Technology
//  All Rights Reserved.
//
//	Description:
//	$Id: MediaPackage.cpp 139 2013-06-19 06:36:37Z jiangwei $
//

#include <string.h>

#include "Base/MediaPackage.h"
#include "Base/PrintLog.h"
#include "Base/BaseTemplate.h"
#include "Base/StaticMemPool.h"
#include "MediaConvert.h"

namespace Xunmei{
namespace Base{
typedef enum
{
	EXTRATYPE_Solution = 0x01,
	EXTRATYPE_Framerate = 0x02,
	EXTRATYPE_Bitrate = 0x03,
	EXTRATYPE_Timestamp = 0x04,
	EXTRATYPE_Position = 0x05,
	EXTRATYPE_CodecInfo = 0x06,
	EXTRATYPE_AudioInfo = 0x07,
	EXTRATYPE_Statistic = 0x0f,
	EXTRATYPE_EncryptInfo = 0x10,
	EXTRATYPE_UserDefine = 0x20,
}XUNMEIMEDIA_ExtraType;


////////////////////////////////////////////////////////////////////////////////
struct extDataBuffer
{
	uint32_t	exFlag : 8;
	uint32_t 	exLen : 24;
};
// 扩展数据结构体
typedef struct
{
	uint32_t width	: 16;
	uint32_t height : 16;
}solution;//分辨率扩展数据

typedef struct
{
	uint32_t frate : 32;
}framerate;//帧率扩展数据

typedef struct
{
	uint32_t bitctrl: 1;
	uint32_t bitval : 31;
}bitrate;//码率控制扩展数据


typedef MediaExtTime extTime;//时间信息扩展数据

typedef struct
{
	uint32_t longitude : 32;
	uint32_t latitude  : 32;
}position;//经纬度坐标扩展数据

typedef struct
{
	uint32_t gopsequence 	: 16;
	uint32_t framerate 		: 8;
	uint32_t ifcount 		: 8;
	uint32_t bfcount 		: 8;
	uint32_t pfcount 		: 8;
	uint32_t downspeed 		: 16;
}statistic;//帧统计信息扩展数据

typedef struct
{
	uint16_t				vCodecId;
	uint16_t				aCodecId;
}codecInfo;

typedef struct
{
	uint32_t audioChannels		: 8; //音频通道数
	uint32_t audioBitsPerSample	: 8; //音频采样字节
	uint32_t audioSamplesPerSecond	: 16; //音频采样率
}audioInfo;

typedef struct
{
	uint32_t		AlgorithmType:8;		//加密类型
	uint32_t		Reserved:8;				//无用
	uint32_t		PasswordLength:8;		//password字段长度
	uint32_t		IVLength:8;				//IVData字段长度	
	uint32_t		Summary;				//校验位
	uint32_t		FrameSize;				//原始帧数据大小
}PasswordHeader;

struct MediaPackage::MediaInternal
{
	int					currVersion;
	int					currOffset;
	IMediaConver*	conver;
};


uint32_t getExtDatalen(uint32_t len)
{
	if(len == 0)
	{
		return 0;
	}
	if((len % 4) != 0)
	{
		len = ((len/4) + 1)*4;
	}
	len += sizeof(extDataBuffer);

	return len;
}

uint32_t getDatalen(uint32_t len)
{
	if(len == 0)
	{
		return 0;
	}
	if((len % 4) != 0)
	{
		len = ((len/4) + 1)*4;
	}

	return len;
}

//取4的整数倍
#define ExtLenLog4(len)	getExtDatalen(len)
#define DataLenLog4(len) getDatalen(len)

void SAFE_MEMCPY(void* dst,int dstlen,const void* src,int srclen)
{
	if(dst != NULL && src != NULL && dstlen != 0 && dstlen >= srclen) 
	{
		memcpy(dst,src,srclen);
	}
}

MediaPackage::MediaPackage(IMallcFreeMemObjcPtr* ptr) : Packet(ptr),internal(NULL)
{
}

MediaPackage::MediaPackage(const char* packetHead,IMallcFreeMemObjcPtr* ptr) : Packet(ptr),internal(NULL)
{
	internal = new(std::nothrow) MediaInternal;
	if(internal == NULL || packetHead == NULL)
	{
		return;
	}
	internal->currVersion = MEDIA_VERSION;
	internal->currOffset = MEDIAHEADER_LENGTH;
	internal->conver = NULL;
	if (::memcmp(packetHead, "XMMD", 4) != 0)
	{
		return;
	}
	uint8_t version = MEDIA_VERSION;
	memcpy(&version,packetHead + 4,1);
	if(version == MEDIA_V1)
	{
		internal->conver = new(std::nothrow) MediaConverV1ToV2(this);
		internal->currVersion = MEDIA_V1;
		internal->currOffset = MEDIAHEADER_LENGTH_V1;

		MediaConverV1ToV2::XunmeiMediaV1Header headerv1;

		internal->conver->parseByHeaderPtr(packetHead,&headerv1);

		reinit(headerv1.total);

		uint8_t ver = MEDIA_VERSION;
		uint8_t *p = getBuffer();
		SAFE_MEMCPY(p, 4, "XMMD", 4);
		SAFE_MEMCPY(p + 4, 1, &ver, 1);
		SAFE_MEMCPY(p + 8, 4, &headerv1.total, 4);

		MediaConverV2ToV3::ConverFrameType(MEDIA_V2,headerv1.frametype,headerv1.frametype);
		MediaConverV2ToV3::ConverCodeId(MEDIA_V2,headerv1.codecID,headerv1.subCodecID,headerv1.codecID,headerv1.subCodecID);

		setFrameType((uint8_t)headerv1.frametype);
		setCodecID((uint8_t)headerv1.codecID);
		setPackageID((uint8_t)headerv1.subCodecID);
		setSequence(headerv1.frameSeq);
		setTimeStamp(headerv1.timestamp);
		updateVerify();
		addDataLength(MEDIAHEADER_LENGTH);
		addExtraTimeStamp(headerv1.xmtime);

		return;
	}
	
	uint8_t verify = 0;
	for (int i = 0; i != 23; i++)
	{
		verify += (uint8_t)packetHead[i];
	}
	if (verify != (uint8_t)packetHead[23])
	{
		return;
	}

	reinit(*((uint32_t *)(packetHead + 8)));
	if(!Packet::isValid())
	{
		return;
	}
	SAFE_MEMCPY(getBuffer(), MEDIAHEADER_LENGTH, packetHead, MEDIAHEADER_LENGTH);
	addDataLength(MEDIAHEADER_LENGTH);

	if(version == MEDIA_V2)
	{
		uint32_t freamtype = getFrameType();
		uint32_t codeid = getCodecID();
		uint32_t packetid = getPackageID();

		MediaConverV2ToV3::ConverFrameType(MEDIA_V2,freamtype,freamtype);
		MediaConverV2ToV3::ConverCodeId(MEDIA_V2,codeid,packetid,codeid,packetid);

		uint8_t ver = MEDIA_VERSION;
		uint8_t *p = getBuffer();
		SAFE_MEMCPY(p + 4, 1, &ver, 1);

		setFrameType(freamtype);
		setCodecID(codeid);
		setPackageID(packetid);
	}
}

MediaPackage::MediaPackage(uint32_t frameSize,IMallcFreeMemObjcPtr* ptr) : Packet(ptr,frameSize + MEDIAHEADER_LENGTH),internal(NULL)
{
	if(!Packet::isValid())
	{
		return;
	}
	internal = new(std::nothrow) MediaInternal;
	if(internal == NULL)
	{
		return;
	}
	internal->currVersion = MEDIA_VERSION;
	internal->currOffset = MEDIAHEADER_LENGTH;
	internal->conver = NULL;

	memset(getBuffer(),0,MEDIAHEADER_LENGTH);
	int packetTotalSize = frameSize + MEDIAHEADER_LENGTH;
	uint8_t ver = MEDIA_VERSION;
	uint8_t *p = getBuffer();
	SAFE_MEMCPY(p, 4, "XMMD", 4);
	SAFE_MEMCPY(p + 4, 1, &ver, 1);
	SAFE_MEMCPY(p + 8, 4, &packetTotalSize, 4);
	updateVerify();
	addDataLength(MEDIAHEADER_LENGTH);
}

MediaPackage::MediaPackage(uint8_t frameType, uint8_t codecID, uint8_t packageID, uint32_t frameSize,
	uint32_t frameSeq, uint32_t timestamp,IMallcFreeMemObjcPtr* ptr) : Packet(ptr,frameSize + MEDIAHEADER_LENGTH),internal(NULL)
{
	if(!Packet::isValid())
	{
		return;
	}
	internal = new(std::nothrow) MediaInternal;
	if(internal == NULL)
	{
		return;
	}
	internal->currVersion = MEDIA_VERSION;
	internal->currOffset = MEDIAHEADER_LENGTH;
	internal->conver = NULL;

	int packetTotalSize = frameSize + MEDIAHEADER_LENGTH;
	uint8_t ver = MEDIA_VERSION;
	uint8_t *p = getBuffer();
	SAFE_MEMCPY(p, 4, "XMMD", 4);
	SAFE_MEMCPY(p + 4, 1, &ver, 1);
	SAFE_MEMCPY(p + 8, 4, &packetTotalSize, 4);
	setFrameType(frameType);
	setCodecID(codecID);
	setPackageID(packageID);
	setSequence(frameSeq);
	setTimeStamp(timestamp);
	updateVerify();
	addDataLength(MEDIAHEADER_LENGTH);
}
MediaPackage::~MediaPackage()
{
	if(internal != NULL)
	{
		SAFE_DELETE(internal->conver);
		SAFE_DELETE(internal);
	}
}

bool MediaPackage::isValid(void) const
{
	if(!Packet::isValid() || internal == NULL)
	{
		return false;
	}

	return true;
}
uint32_t MediaPackage::getHeaderOffset() const
{
	if(!isValid())
	{
		return 0;
	}

	return internal->currOffset;
}
void MediaPackage::updateVerify()
{
	if(!isValid())
	{
		return;
	}
	uint8_t* packet = getBuffer();
	if (NULL != packet)
	{
		uint8_t verify = 0;
		for (int i = 0; i != 23; i++)
		{
			verify += packet[i];
		}
		SAFE_MEMCPY(packet + 23, 1, &verify, 1);
	}
}
uint32_t MediaPackage::getRemainSize() const
{
	if(!isValid())
	{
		return 0;
	}
	if(internal != NULL && internal->conver != NULL)
	{
		return internal->conver->getRemainSize();
	}
	char* packet = (char*)getBuffer();
	int packetTotalSize = *((int *)(packet + 8));

	return packetTotalSize - getDataLen();
}

int32_t MediaPackage::addRemainData(const char* remainData, uint32_t dataSize)
{
	if(!isValid())
	{
		return -1;
	}
	if(internal != NULL && internal->conver != NULL)
	{
		char* addr = (char*)remainData;
		uint32_t copysize = dataSize;
		while(addr != NULL && dataSize > 0)
		{
			addr = internal->conver->inputData(addr,dataSize);
			if(addr != NULL)
			{
				SAFE_MEMCPY(getBuffer() + getDataLen(), dataSize, addr, dataSize);
				addDataLength(dataSize);
				dataSize -= dataSize;
			}
		}
		return copysize;
	}
	uint32_t remainSize = getRemainSize();

	if(dataSize > remainSize)
	{
		return -1;
	}
	SAFE_MEMCPY(getBuffer() + getDataLen(), dataSize, remainData, dataSize);
	addDataLength(dataSize);

	return dataSize;
}

bool MediaPackage::setFrameType(uint8_t frameType)
{
	if(!isValid())
	{
		return false;
	}

	uint8_t* packet = getBuffer();
	if (NULL != packet)
	{
		SAFE_MEMCPY(packet + 5, 1, &frameType, 1);
		updateVerify();
		return true;
	}

	return false;
}

bool MediaPackage::setCodecID(uint8_t codecID)
{
	if(!isValid())
	{
		return false;
	}
	uint8_t* packet = getBuffer();
	if (NULL != packet)
	{
		SAFE_MEMCPY(packet + 6, 1, &codecID, 1);
		updateVerify();
		return true;
	}

	return false;
}

bool MediaPackage::setPackageID(uint8_t packageID)
{
	if(!isValid())
	{
		return false;
	}
	uint8_t* packet = getBuffer();
	if (NULL != packet)
	{
		SAFE_MEMCPY(packet + 7, 1, &packageID, 1);
		updateVerify();
		return true;
	}

	return false;
}

bool MediaPackage::setSequence(uint32_t seq)
{
	if(!isValid())
	{
		return false;
	}
	uint8_t* packet = getBuffer();
	if (NULL != packet)
	{
		SAFE_MEMCPY(packet + 12, 4, &seq, 4);
		updateVerify();
		return true;
	}

	return false;
}

bool MediaPackage::setTimeStamp(uint32_t timeStamp)
{
	if(!isValid())
	{
		return false;
	}
	uint8_t* packet = getBuffer();
	if (NULL != packet)
	{
		SAFE_MEMCPY(packet + 16, 4, &timeStamp, 4);
		updateVerify();
		return true;
	}

	return false;
}

bool MediaPackage::addExtraSolution(uint16_t solutionWidth, uint16_t solutionHeight)
{
	solution sol;
	sol.width = solutionWidth;
	sol.height = solutionHeight;

	return appendExtraData(EXTRATYPE_Solution,&sol,sizeof(solution));
}

bool MediaPackage::getExtraSolution(uint16_t& solutionWidth, uint16_t& solutionHeight) const
{
	uint32_t extbufferlen = 0;
	char* extbuffer = NULL;

	extbuffer = findExtraData(EXTRATYPE_Solution,extbufferlen);
	if(extbuffer == NULL || extbufferlen != sizeof(solution))
	{
		return false;
	}
	solution* psol = (solution*)extbuffer;
	solutionWidth = psol->width;
	solutionHeight = psol->height;

	return true;
}

bool MediaPackage::addExtraFrameRate(const uint8_t frameRate)
{
	framerate rate;
	rate.frate = frameRate;
	
	return appendExtraData(EXTRATYPE_Framerate,&rate,sizeof(framerate));
}

bool MediaPackage::getExtraFrameRate(uint8_t& frameRate) const
{
	uint32_t extbufferlen = 0;
	char* extbuffer = NULL;

	extbuffer = findExtraData(EXTRATYPE_Framerate,extbufferlen);
	if(extbuffer == NULL || extbufferlen != sizeof(framerate))
	{
		return false;
	}
	framerate* ptmp = (framerate*)extbuffer;
	frameRate = ptmp->frate;

	return true;
}

bool MediaPackage::addExtraBitrate(const uint8_t bitCtrlType, const uint32_t bitRateVal)
{
	bitrate brate;

	brate.bitctrl = bitCtrlType;
	brate.bitval = bitRateVal;
	
	return appendExtraData(EXTRATYPE_Bitrate,&brate,sizeof(bitrate));
}

bool MediaPackage::getExtraBitrate(uint8_t& bitCtrlType, uint32_t& bitRateVal) const
{
	uint32_t extbufferlen = 0;
	char* extbuffer = NULL;

	extbuffer = findExtraData(EXTRATYPE_Bitrate,extbufferlen);
	if(extbuffer == NULL || extbufferlen != sizeof(bitrate))
	{
		return false;
	}
	bitrate* ptmp = (bitrate*)extbuffer;
	bitCtrlType = ptmp->bitctrl;
	bitRateVal = ptmp->bitval;

	return true;
}

bool MediaPackage::addExtraTimeStamp(const MediaExtTime& xmTime)
{
	extTime etime;
	etime.year = xmTime.year;
	etime.month = xmTime.month;
	etime.date = xmTime.date;
	etime.hour = xmTime.hour;
	etime.minute = xmTime.minute;
	etime.second = xmTime.second;
	etime.millsec = xmTime.millsec;
		
	return appendExtraData(EXTRATYPE_Timestamp,&etime,sizeof(extTime));
}

bool MediaPackage::getExtraTimeStamp(MediaExtTime& xmTime) const
{
	uint32_t extbufferlen = 0;
	char* extbuffer = NULL;

	extbuffer = findExtraData(EXTRATYPE_Timestamp,extbufferlen);
	if(extbuffer == NULL || extbufferlen != sizeof(extTime))
	{
		return false;
	}
	extTime* ptmp = (extTime*)extbuffer;
	xmTime.year = ptmp->year;
	xmTime.month = ptmp->month;
	xmTime.date = ptmp->date;
	xmTime.hour = ptmp->hour;
	xmTime.minute = ptmp->minute;
	xmTime.second = ptmp->second;
	xmTime.millsec = ptmp->millsec;
	
	return true;
}

bool MediaPackage::addExtraPostion(const uint32_t longitude, const uint32_t latitude)
{
	position epos;
	epos.longitude = longitude;
	epos.latitude = latitude;
	
	return appendExtraData(EXTRATYPE_Position,&epos,sizeof(position));
}

bool MediaPackage::getExtraPostion(uint32_t& longitude, uint32_t& latitude) const
{
	uint32_t extbufferlen = 0;
	char* extbuffer = NULL;

	extbuffer = findExtraData(EXTRATYPE_Position,extbufferlen);
	if(extbuffer == NULL || extbufferlen != sizeof(position))
	{
		return false;
	}
	position* ptmp = (position*)extbuffer;
	longitude = ptmp->longitude;
	latitude = ptmp->latitude;

	return true;
}

bool MediaPackage::addExtraStatistic(const uint16_t gopSeq, const uint16_t downSpeed, const uint8_t frameRate, const uint8_t ifCount, 
	const uint8_t pfCount, const uint8_t bfCount)
{
	statistic exstic;
	exstic.gopsequence = gopSeq;
	exstic.downspeed = downSpeed;
	exstic.framerate = frameRate;
	exstic.ifcount = ifCount;
	exstic.pfcount = pfCount;
	exstic.bfcount = bfCount;
		
	return appendExtraData(EXTRATYPE_Statistic,&exstic,sizeof(statistic));
}

bool MediaPackage::getExtraStatistic(uint16_t& gopSeq, uint16_t& downSpeed, uint8_t& frameRate, uint8_t& ifCount, 
	uint8_t& pfCount, uint8_t& bfCount) const
{
	uint32_t extbufferlen = 0;
	char* extbuffer = NULL;

	extbuffer = findExtraData(EXTRATYPE_Statistic,extbufferlen);
	if(extbuffer == NULL || extbufferlen != sizeof(statistic))
	{
		return false;
	}
	statistic* ptmp = (statistic*)extbuffer;
	gopSeq = ptmp->gopsequence;
	downSpeed = ptmp->downspeed;
	frameRate = ptmp->framerate;
	ifCount = ptmp->ifcount;
	bfCount = ptmp->bfcount;
	pfCount = ptmp->pfcount;

	return true;
}

bool MediaPackage::addExtraCodecInfo(MEDIA_CodecID vCodecId,MEDIA_CodecID aCodecId)
{
	codecInfo info;
	info.vCodecId = (uint8_t)vCodecId;
	info.aCodecId = (uint8_t)aCodecId;

	return appendExtraData(EXTRATYPE_CodecInfo,&info,sizeof(codecInfo));
}
bool MediaPackage::getExtraCodecInfo(MEDIA_CodecID& vCodecId,MEDIA_CodecID& aCodecId) const
{
	uint32_t extbufferlen = 0;
	char* extbuffer = NULL;

	extbuffer = findExtraData(EXTRATYPE_CodecInfo,extbufferlen);
	if(extbuffer == NULL || extbufferlen != sizeof(codecInfo))
	{
		return false;
	}
	codecInfo* info = (codecInfo*)extbuffer;
	vCodecId = (MEDIA_CodecID)info->vCodecId;
	aCodecId = (MEDIA_CodecID)info->aCodecId;

	return true;
}

bool MediaPackage::addExtraAudioInfo(uint8_t audioChannels, uint8_t audioBitsPerSamples, uint16_t audioSamplesPerSecond)
{
	audioInfo exAudioInfo;
	exAudioInfo.audioChannels = audioChannels;
	exAudioInfo.audioBitsPerSample = audioBitsPerSamples;
	exAudioInfo.audioSamplesPerSecond = audioSamplesPerSecond;

	return appendExtraData(EXTRATYPE_AudioInfo, &exAudioInfo, sizeof(audioInfo));
}

bool MediaPackage::getExtraAudioInfo(uint8_t& audioChannels, uint8_t& audioBitsPerSamples, uint16_t& audioSamplesPerSecond) const
{
	uint32_t extbufferlen = 0;
	char* extbuffer = NULL;

	extbuffer = findExtraData(EXTRATYPE_AudioInfo,extbufferlen);
	if(extbuffer == NULL || extbufferlen != sizeof(audioInfo))
	{
		return false;
	}

	audioInfo* exInfo = (audioInfo*) extbuffer;
	audioChannels = exInfo->audioChannels;
	audioBitsPerSamples = exInfo->audioBitsPerSample;
	audioSamplesPerSecond = exInfo->audioSamplesPerSecond;

	return true;
}

bool MediaPackage::addExtraEncryptInfo(const PasswordInfo& encryptInfo)
{
	uint32_t encrybufferlen = sizeof(PasswordHeader);
	encrybufferlen += 16; //证书编码长度
	encrybufferlen += encryptInfo.PasswordLength;
	encrybufferlen += encryptInfo.IVLength;

	uint32_t writeBufferOffset = 0;
	char *extraBuffer = new char[encrybufferlen];

	PasswordHeader passwordHeader = {0};
	passwordHeader.AlgorithmType = encryptInfo.AlgorithmType;
	passwordHeader.FrameSize = encryptInfo.FrameSize;
	passwordHeader.IVLength = encryptInfo.IVLength;
	passwordHeader.PasswordLength = encryptInfo.PasswordLength;
	passwordHeader.Reserved = encryptInfo.Reserved;
	passwordHeader.Summary = encryptInfo.Summary;
	memcpy(extraBuffer + writeBufferOffset, &passwordHeader, sizeof(PasswordHeader));
	writeBufferOffset += sizeof(PasswordHeader);
	memcpy(extraBuffer + writeBufferOffset, encryptInfo.Certificate, 16);
	writeBufferOffset += DataLenLog4(16);
	memcpy(extraBuffer + writeBufferOffset, encryptInfo.Password, encryptInfo.PasswordLength);
	writeBufferOffset += DataLenLog4(encryptInfo.PasswordLength);
	memcpy(extraBuffer + writeBufferOffset, encryptInfo.IVData, encryptInfo.IVLength);

	return appendExtraData(EXTRATYPE_EncryptInfo, extraBuffer, encrybufferlen);
}

bool MediaPackage::getExtraEncryptInfo(PasswordInfo& encryptInfo)
{
	uint32_t extbufferlen = 0;
	char* extbuffer = NULL;

	uint32_t readBufferOffset = 0;
	extbuffer = findExtraData(EXTRATYPE_EncryptInfo, extbufferlen);
	if (extbuffer == NULL)
	{
		return false;
	}
	PasswordHeader *sPasswordHeader = (PasswordHeader*)extbuffer;
	uint32_t size = sizeof(PasswordHeader) + 16 + sPasswordHeader->IVLength + sPasswordHeader->PasswordLength;
	if(size != extbufferlen)
	{
		return false;
	}
	encryptInfo.AlgorithmType = sPasswordHeader->AlgorithmType;
	encryptInfo.FrameSize = sPasswordHeader->FrameSize;
	encryptInfo.IVLength = sPasswordHeader->IVLength;
	encryptInfo.PasswordLength = sPasswordHeader->PasswordLength;
	encryptInfo.Reserved = sPasswordHeader->Reserved;
	encryptInfo.Summary = sPasswordHeader->Summary;

	readBufferOffset += sizeof(PasswordHeader);
	memcpy(encryptInfo.Certificate, extbuffer + readBufferOffset, 16);
	readBufferOffset += DataLenLog4(16);
	memcpy(encryptInfo.Password, extbuffer + readBufferOffset, encryptInfo.PasswordLength);
	readBufferOffset += DataLenLog4(encryptInfo.PasswordLength);
	memcpy(encryptInfo.IVData, extbuffer + readBufferOffset, encryptInfo.IVLength);

	return true;
}

bool MediaPackage::addCustomData(int type,const char* pCustomData,uint16_t CustomSize)
{
	if(type < EXTRATYPE_UserDefine)
	{
		return false;
	}
	return appendExtraData(type,(void*)pCustomData,CustomSize);
}

const char* MediaPackage::getCustomData(int type,uint16_t& CustomSize) const
{
	if(type < EXTRATYPE_UserDefine)
	{
		return NULL;
	}
	
	uint32_t extbufferlen = 0;
	char* extbuffer = NULL;

	extbuffer = findExtraData(type,extbufferlen);
	if(extbuffer == NULL)
	{
		return NULL;
	}
	CustomSize = (uint16_t)extbufferlen;

	return extbuffer;
}

const char* MediaPackage::getExtraData() const
{
	if(!isValid())
	{
		return NULL;
	}
	
	uint8_t* packet = getBuffer();
	if (NULL != packet)
	{
		uint32_t extraTotalSize = getExtraDataSize();

		if (0 < extraTotalSize)
		{
			return (char*)(packet + MEDIAHEADER_LENGTH);
		}
	}

	return NULL;
}

bool MediaPackage::addFrameData(const char * data)
{
	if(!isValid())
	{
		return false;
	}
	uint8_t* packet = getBuffer();
	if (NULL != packet)
	{
		uint32_t packetTotalSize = *((uint32_t *)(getBuffer() + 8));
		uint32_t extraTotalSize = getExtraDataSize();
		uint32_t frameSize = packetTotalSize - MEDIAHEADER_LENGTH - extraTotalSize;

		if (0 < frameSize)
		{
			SAFE_MEMCPY(packet + MEDIAHEADER_LENGTH + extraTotalSize, frameSize, data, frameSize);
			addDataLength(frameSize);
			return true;
		}
	}

	return false;
}

const char *MediaPackage::getFrameBuffer() const
{
	if(!isValid())
	{
		return NULL;
	}
	uint8_t* packet = getBuffer();
	if (NULL != packet)
	{
		uint32_t extraTotalSize = getExtraDataSize();
		return (char *)(packet + MEDIAHEADER_LENGTH + extraTotalSize);
	}

	return NULL;
}

uint8_t MediaPackage::getVersion() const
{
	if(!isValid())
	{
		return 0;
	}
	if(internal != NULL)
	{
		return internal->currVersion;
	}

	return *(uint8_t *)(getBuffer() + 4);
}

uint8_t MediaPackage::getFrameType() const
{
	if(!isValid())
	{
		return 0;
	}
	return *(uint8_t *)(getBuffer() + 5);
}

uint8_t MediaPackage::getCodecID() const
{
	if(!isValid())
	{
		return 0;
	}
	return *(uint8_t *)(getBuffer() + 6);
}

uint8_t MediaPackage::getPackageID() const
{
	if(!isValid())
	{
		return 0;
	}
	return *(uint8_t *)(getBuffer() + 7);
}

uint32_t MediaPackage::getFrameSize() const
{
	if(!isValid())
	{
		return 0;
	}
	uint32_t packetTotalSize = *((uint32_t *)(getBuffer() + 8));
	uint32_t extraTotalSize = getExtraDataSize();
	return packetTotalSize - MEDIAHEADER_LENGTH - extraTotalSize;
}

uint32_t MediaPackage::getSequence() const
{
	if(!isValid())
	{
		return 0;
	}
	return *(uint32_t *)(getBuffer() + 12);
}

uint32_t MediaPackage::getTimeStamp() const
{
	if(!isValid())
	{
		return 0;
	}
	return *(uint32_t *)(getBuffer() + 16);
}

uint32_t MediaPackage::getExtraDataSize() const
{
	if(!isValid())
	{
		return 0;
	}
	
	char* buffer = (char*)getBuffer();
	uint32_t extraTotalSize = 0;
	memcpy(&extraTotalSize,buffer + 20,3);

	return extraTotalSize;
}

bool MediaPackage::appendExtraData(int flag,void* data,int datalen)
{
	if(!isValid())
	{
		return false;
	}
	
	uint32_t exlen = 0;
	char* extbuffer = findExtraData(flag,exlen);

	if(extbuffer != NULL && (int)exlen == datalen)
	{
		//已经存在并且长度一致、直接覆盖
		memcpy(extbuffer,data,datalen);
		return true;
	}

	extDataBuffer newextbufer;
	newextbufer.exFlag = flag;
	newextbufer.exLen = datalen;

	uint8_t* packet = getBuffer();
	uint32_t oldPacketSize = *((uint32_t*)(packet + 8));
	uint32_t packetExtraSize = getExtraDataSize();
	uint32_t newExtSize = packetExtraSize - ExtLenLog4(exlen) + ExtLenLog4(newextbufer.exLen);
	uint32_t newPacketSize = oldPacketSize - ExtLenLog4(exlen) + ExtLenLog4(newextbufer.exLen);
	uint8_t* needcopyTailAddr = NULL;
	uint32_t needcopyheaderSize = 0,needcopyTailSIZE = 0;
	if(extbuffer == NULL)
	{
		needcopyheaderSize = MEDIAHEADER_LENGTH + packetExtraSize;
		needcopyTailAddr = packet + MEDIAHEADER_LENGTH + packetExtraSize;
		needcopyTailSIZE = oldPacketSize - needcopyheaderSize;
	}
	else
	{
		needcopyheaderSize = extbuffer - (char*)packet;
		needcopyTailAddr = (uint8_t*)extbuffer + ExtLenLog4(exlen);
		needcopyheaderSize = oldPacketSize - needcopyheaderSize - ExtLenLog4(exlen);
	}
	uint32_t newspacesize = newPacketSize;
	uint8_t* newPacketData = (uint8_t*)remalloc(newspacesize);
	if(newPacketData == NULL)
	{
		return false;
	}
	if(newPacketData != packet)
	{
		memmove(newPacketData,packet,needcopyheaderSize);
	}
	//先拷贝后面的数据、避免同一buffer的被覆盖
	memmove(newPacketData + needcopyheaderSize + ExtLenLog4(datalen),needcopyTailAddr,needcopyTailSIZE);
	//加入新数据
	memcpy(newPacketData + needcopyheaderSize,&newextbufer,sizeof(extDataBuffer));
	memcpy(newPacketData + needcopyheaderSize + sizeof(extDataBuffer),data,datalen);
	

	SAFE_MEMCPY(newPacketData + 8, 4, &newPacketSize, 4);
	SAFE_MEMCPY(newPacketData + 20, 3, &newExtSize, 3);

	resetBuffer(newPacketData,newPacketSize,newspacesize);
	updateVerify();
	
	addDataLength(ExtLenLog4(newextbufer.exLen) - ExtLenLog4(exlen));

	return true;
}

char* MediaPackage::findExtraData(uint8_t flag,uint32_t& len) const
{
	if (! isValid())
	{
		return NULL;
	}
	uint32_t extraTotalSize = getExtraDataSize(), readSize = 0;
	if(0 >= extraTotalSize)
	{
		return NULL;
	}
	while(readSize < extraTotalSize)
	{
		extDataBuffer* exbuf = (extDataBuffer*)(getBuffer() + MEDIAHEADER_LENGTH + readSize);
		if(exbuf->exLen > extraTotalSize - readSize || exbuf->exLen == 0)
		{
			break;
		}
		if(exbuf->exFlag != flag)
		{
			readSize += ExtLenLog4(exbuf->exLen);
			continue;
		}
		len = exbuf->exLen;
		return ((char*)getBuffer() + MEDIAHEADER_LENGTH + readSize + sizeof(extDataBuffer));
	}	
	
	return NULL;
}

MediaPackage* MediaPackage::clone(IMallcFreeMemObjcPtr* ptr) const
{
	MediaPackage* media = new(std::nothrow) MediaPackage((char*)getBuffer(),ptr);
	if(media != NULL)
	{
		media->addRemainData((char*)getBuffer() + media->getHeaderOffset(),media->getRemainSize());
	}
		
	return (MediaPackage*)media;
}

void MediaPackage::converCodeId(int ver,uint32_t codeid,uint32_t subcode,uint32_t& newtype,uint32_t& packetid)
{
	if(ver == MEDIA_V1)
	{
		MediaConverV1ToV2::ConverCodeId(ver,codeid,subcode,newtype,packetid);
		ver = MEDIA_V2;
	}
 	if(ver == MEDIA_V2)
 	{
 		MediaConverV2ToV3::ConverCodeId(ver,codeid,subcode,newtype,packetid);
 		ver = MEDIA_V3;
 	}
}
void MediaPackage::converFrameType(int ver,uint32_t type,uint32_t& newtype)
{
	if(ver == MEDIA_V1)
	{
		MediaConverV1ToV2::ConverFrameType(ver,type,newtype);
		ver = MEDIA_V2;
	}
 	if(ver == MEDIA_V2)
 	{
 		MediaConverV2ToV3::ConverFrameType(ver,type,newtype);
 		ver = MEDIA_V3;
 	}
}
} // namespace Base
} // namespace Xunmei



