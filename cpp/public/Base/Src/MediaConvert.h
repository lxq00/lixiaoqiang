//
//  Copyright (c)1998-2012, Chongqing Xunmei Technology
//  All Rights Reserved.
//
//	Description:
//	$Id: MediaConvertV1ToV2.h 248 2013-11-19 05:35:55Z lixiaoqiang $
#ifndef __MEDIACONVERTV1TOV2_H__
#define __MEDIACONVERTV1TOV2_H__
#include "Base/MediaPackage.h"
#include "Base/BaseTemplate.h"
#include "Base/Time.h"
namespace Xunmei{
namespace Base{

/// xunmeimedia版本转换接口
class IMediaConver
{
public:
	IMediaConver(MediaPackage* media):mediav2(media){}
	virtual ~IMediaConver(){}

	virtual bool parseByHeaderPtr(const char* buffer,void* outpaarm) = 0;
	virtual char* inputData(const char* buffer, uint32_t& dataSize) = 0;
	virtual int32_t getRemainSize() = 0;
protected:
	MediaPackage*			mediav2;
};

template<typename T>
T GetValueByBite(const char* buf,int pos)
{
	T val;
	memcpy(&val,buf + pos,sizeof(T));
	return val;
}

///讯美版本转换 V1 TO new
class MediaConverV1ToV2:public IMediaConver
{
public:
	struct XunmeiMediaV1Header
	{
		uint32_t frametype,codecID,subCodecID,total,frameSeq,timestamp;
		MediaExtTime	xmtime;
	};
public:
	MediaConverV1ToV2(MediaPackage* media):IMediaConver(media),copylen(0),totallen(0),exlength(0)
	{
		extDataBuffer = NULL;
		extDataLen = 0;
	}
	~MediaConverV1ToV2()
	{
		SAFE_DELETE(extDataBuffer);
	}
	static void ConverFrameType(int ver,uint32_t type,uint32_t& newtype)
	{
		if(ver != MEDIA_V1)
		{
			newtype = type;
			return;
		}
		newtype = FRAMETYPE_Header;
		switch(type)
		{
		case 0x00:
			newtype = FRAMETYPE_IFrame;
			break;
		case 0x01:
			newtype = FRAMETYPE_PFrame;
			break;
		case 0x02:
			newtype = FRAMETYPE_BFrame;
			break;
		case 0x03:
			newtype = FRAMETYPE_Video;
			break;
		case 0x20:
			newtype = FRAMETYPE_Audio;
			break;
		case 0x40:
			newtype = FRAMETYPE_OSD;
			break;
		case 0x80:
			newtype = FRAMETYPE_Header;
			break;
		default:
			newtype = FRAMETYPE_Manufacturer;
			break;	
		}
	}
	static void ConverCodeId(int ver,uint32_t type,uint32_t subcode,uint32_t& newtype,uint32_t& packetid)
	{
		if(ver != MEDIA_V1)
		{
			newtype = type;
			packetid = subcode;

			return;
		}
		packetid = subcode;
		switch(type)
		{
		case 0x00:
			newtype = 0x01;
			break;
		case 0x01:
			newtype = 0x03;
			break;
		case 0x02:
			newtype = 0x10;
			break;
		case 0x03:
			newtype = 0x29;
			break;
		case 0x20:
		case 0x21:
		case 0x22:
		case 0x23:
		case 0x24:
		case 0x25:
		case 0x26:
		case 0x27:
		case 0x28:
		case 0x40:
			newtype = type;
			break;
		case 0x41:
			packetid = 0x00;
		default:
			break;	
		}
	}
	bool parseByHeaderPtr(const char* buffer,void* outparam)
	{
		XunmeiMediaV1Header* header = (XunmeiMediaV1Header*)outparam;

		header->frametype	= *(uint8_t *)(buffer + 5);;
		header->codecID		= GetValueByBite<uint8_t>(buffer,6);
		header->subCodecID	= GetValueByBite<uint8_t>(buffer,7);
		header->total		= GetValueByBite<uint32_t>(buffer,8);
		header->frameSeq	= GetValueByBite<uint32_t>(buffer,12);
		header->timestamp	= GetValueByBite<uint32_t>(buffer,16);

		uint8_t tmp1 = GetValueByBite<uint8_t>(buffer,20);
		uint8_t tmp2 = GetValueByBite<uint8_t>(buffer,21);
		uint8_t tmp3 = GetValueByBite<uint8_t>(buffer,22);
		uint8_t tmp4 = GetValueByBite<uint8_t>(buffer,23);
		uint8_t tmp5 = GetValueByBite<uint8_t>(buffer,24);
		uint8_t tmp6 = GetValueByBite<uint8_t>(buffer,25);

		header->xmtime.year		= ((tmp1 << 4) +  ((tmp2 >> 4)&0xF)) + 2000;
		header->xmtime.month	= tmp2 & 0xF;
		header->xmtime.date		= tmp3 >> 3;
		header->xmtime.hour		= ((tmp3 & 0x7) << 2) + (tmp4 >> 6);
		header->xmtime.minute	= tmp4 & 0x3F;
		header->xmtime.second	= tmp5 >> 2;
		header->xmtime.millsec	= ((tmp5 & 0x03) << 8) + (tmp6 & 0xFF);

		Time utctime(header->xmtime.year,header->xmtime.month,header->xmtime.date,header->xmtime.hour,header->xmtime.minute,header->xmtime.second);
			
		header->timestamp = (uint32_t)(utctime.makeTime()*1000 + header->xmtime.millsec);

		ConverFrameType(MEDIA_V1,header->frametype,header->frametype);
		ConverCodeId(MEDIA_V1,header->codecID,header->subCodecID,header->codecID,header->subCodecID);

		exlength = GetValueByBite<uint8_t>(buffer,26) * 4;

		totallen = header->total - MEDIAHEADER_LENGTH_V1;
		header->total = header->total - exlength - MEDIAHEADER_LENGTH_V1 + MEDIAHEADER_LENGTH_V2;

		return true;
	}
	char* inputData(const char* buffer, uint32_t& dataSize)
	{
		char* copyaddr = (char*)buffer;
		if(copylen >= totallen)
		{
			return NULL;
		}
		if(copylen <  exlength)
		{
			int copyextlen = (exlength - copylen > (int)dataSize) ? (int)dataSize : (exlength - copylen);
			if(copyextlen < exlength - extDataLen || extDataLen > 0)
			{
				if(extDataBuffer == NULL)
				{
					extDataBuffer = new(std::nothrow) char[exlength];
				}
				memcpy(extDataBuffer + extDataLen,copyaddr,copyextlen);
				extDataLen += copyextlen;
			}
			if(extDataLen >= exlength)
			{
				mediav2->addCustomData(0x20,extDataBuffer,exlength);
				SAFE_DELETE(extDataBuffer);
				exlength = 0;
			}
			else if(copyextlen >= exlength)
			{
				mediav2->addCustomData(0x20,copyaddr,copyextlen);
			}
			copylen += copyextlen;
			dataSize -= copyextlen;
			copyaddr += copyextlen;
		}
		if(dataSize > 0)
		{
			copylen += dataSize;
			return copyaddr;
		}

		return NULL;
	}
	int32_t getRemainSize()
	{
		return totallen - copylen;
	}
private:
	char*					extDataBuffer;
	int						extDataLen;
	int						copylen;
	int						totallen;
	int						exlength;
};

class MediaConverV2ToV3:public IMediaConver
{
public:
	static void ConverFrameType(int ver,uint32_t type,uint32_t& newtype)
	{
		newtype = type;
	}

	static void ConverCodeId(int ver,uint32_t code,uint32_t packetid,uint32_t& newcode,uint32_t& newpacketid)
	{
		if(ver != MEDIA_V2)
		{
			newcode = code;
			newpacketid = packetid;

			return;
		}

		switch (code)
		{
		//case 0x01: newcode = 0x07;break;
		case 0x40: newcode = 0x50;break;
	    case 0x41: newcode = 0x51;break;
		case 0x42: newcode = 0x56;break;
		case 0x43: newcode = 0x57;break;
		case 0x44: newcode = 0x52;break;
		case 0x50: newcode = 0x2a;break;
		case 0x51: newcode = 0x2b;break;
		case 0x52: newcode = 0x2c;break;
		case 0x53: newcode = 0x2d;break;
		case 0x54: newcode = 0x2e;break;
		case 0x55: newcode = 0x2f;break;
		case 0x56: newcode = 0x30;break;
		case 0x57: newcode = 0x31;break;
		case 0x58: newcode = 0x32;break;
		case 0x59: newcode = 0x33;break;
		case 0x5a: newcode = 0x34;break;
		case 0x5b: newcode = 0x25;break;
		case 0x5c: newcode = 0x36;break;
		default:   newcode = code;break;
		}
		switch (packetid)
		{
		case 0x00: newpacketid = 0x01;break;
		case 0x01: newpacketid = 0x02;break;
		case 0x02: newpacketid = 0x03;break;
		case 0x03: newpacketid = 0x04;break;
		default:   newpacketid = packetid;break;
		}
	}
};

};
};
#endif //__MEDIACONVERTV1TOV2_H__
