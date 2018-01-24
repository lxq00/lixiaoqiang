//
//  Copyright (c)1998-2012, Xunmei Technology
//  All Rights Reserved.
//
//	Description:
//	$Id: MediaDefind.h 248 2015-02-26 17:25:56Z lixiaoqiang $

#ifndef __BASE_MEDIADEFINE_H__
#define __BASE_MEDIADEFINE_H__

//2 帧类型，1 Byte 无符号整型
enum MEDIA_FrameType
{
	FRAMETYPE_IFrame = 0x00,
	FRAMETYPE_PFrame = 0x01,
	FRAMETYPE_BFrame = 0x02,
	FRAMETYPE_Header = 0x03,
	FRAMETYPE_Video = 0x04,
	FRAMETYPE_CustomData = 0x0e,
	FRAMETYPE_CustomProtocol = 0x0f,
	FRAMETYPE_Statistic = 0x10,
	FRAMETYPE_Audio = 0x20,
	FRAMETYPE_Picture = 0x30,
	FRAMETYPE_BMP = 0x31,
	FRAMETYPE_JPEG = 0x32,
	FRAMETYPE_OSD = 0x40,
	FRAMETYPE_MotionDetetion = 0x50,
	FRAMETYPE_AreaMotion = 0x51,
	FRAMETYPE_ScenceChange = 0x52,
	FRAMETYPE_RegionFound = 0x53,
	FRAMETYPE_Cruise = 0x54,
	FRAMETYPE_Alarm = 0x60,
	FRAMETYPE_VideoOcclusion = 0x61,
	FRAMETYPE_LOSSignal = 0x62,
	FRAMETYPE_AlarmHost = 0x63,
	FRAMETYPE_IllegalOperation = 0x64,
	FRAMETYPE_IntelligentAnalysis = 0x70,
	FRAMETYPE_LegacyDetection = 0x71,
	FRAMETYPE_FaceCapture = 0x72,
	FRAMETYPE_LPR = 0x73,
	FRAMETYPE_BoundaryCrossing = 0x74,
	FRAMETYPE_BodyDetection = 0x75,
	FRAMETYPE_Talk = 0x80,
	FRAMETYPE_Geographic = 0x90,
	FRAMETYPE_Manufacturer = 0xff,
};

//2 编解码ID: 1 Byte,主要用于解码时,选择适当的解码器.具体值在如下
enum MEDIA_CodecID
{
	//4 视频定义
	CodecID_H263 = 0x00,
	CodecID_H264 = 0x01,
	CodecID_H265 = 0x02,
	CodecID_Mpeg4 = 0x03,
	CodecID_H261 = 0x04,
	CodecID_MP2T = 0x05,
	CodecID_MPV = 0x06,
	CodecID_JEPG = 0x07,
	//4  音频定义	
	CodecID_G711A = 0x20,
	CodecID_G711Mu = 0x21,
	CodecID_ADPCM = 0x22,
	CodecID_PCM = 0x23,
	CodecID_G726 = 0x24,
	CodecID_MPEG2 = 0x25,
	CodecID_AMR = 0x26,
	CodecID_AAC = 0x27,
	CodecID_MP3 = 0x28,
	CodecID_OGG = 0x29,
	CodecID_GSM = 0X2a,
	CodecID_DVI4 = 0x2b,
	CodecID_LPC = 0x2c,
	CodecID_L16 = 0x2d,
	CodecID_QCELP = 0x2e,
	CodecID_CN = 0x2f,
	CodecID_MPA = 0x30,
	CodecID_G728 = 0x31,
	CodecID_G729 = 0x32,
	CodecID_G729D = 0x33,
	CodecID_G729E = 0x34,
	CodecID_GSMESR = 0x35,
	CodecID_L8 = 0x36,
	//4 图片定义
	CodecID_MJEPG = 0x40,

	//以下为设备私有流流定义
	CodecID_Hik = 0x50,			//海康
	CodecID_DaHua = 0x51,		//大华
	CodecID_Bosch_H263 = 0x52,  //博士D1非标准H263、需要组场处理
	CodecID_XunMeiD = 0x53,		//讯美D设备
	CodecID_XunMeiE = 0x54,		//讯美E设备
	CodecID_XunMeiHY = 0x55,	//讯美HY设备
	CodecID_XunMei = 0x56,		//其他讯美设备统一的私有流
	CodecID_BlueStar7K = 0x57,	//蓝星7系设备
	CodecID_BlueStar6K = 0x58,	//蓝星6系设备
	CodecID_BlueStar = 0x59,	//其他蓝星设备统一的私有流
	CodecID_HaoYun = 0x5a,		//浩云设备
	CodecID_HanBang = 0x5b,		//汉邦设备
	CodecID_TDWY = 0x5c,		//天地伟业设备
	CodecID_CinSTon = 0x5d,		//新视电设备
	CodecID_XMH5IPCFRG = 0x5e,   //讯美H5IPCFRG系列设备
	CodecID_GosuncnARIPC = 0x60, //高薪兴增强现实

	CodecID_GB28181PS = 0x100,	//国标PS流
};
//2 Package ID: 1 Byte 无符号整型 压缩编码数据打包格式类型ID值，具体值在如下
enum MEDIA_PackageId
{
	PackageID_PES = 0x00,
	PackageID_PS = 0x01,
	PackageID_TS = 0x02,
	PackageID_AESPS = 0x03,
	PackageID_AESTS = 0x04,
	PackageID_GB21818PS = 0x80,
	PackageID_GB21818PES = 0x81,
	PackageID_UnknowFrame = 0x82,	//未知帧数据的数据包，通常用于无法帧分析的原始数据
};

#endif //__BASE_MEDIAPACKAGE_H__


