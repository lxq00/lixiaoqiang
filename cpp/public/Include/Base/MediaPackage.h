//
//  Copyright (c)1998-2012, Xunmei Technology
//  All Rights Reserved.
//
//	Description:
//	$Id: MediaPackage.h 248 2013-11-19 05:35:55Z lixiaoqiang $

#ifndef __BASE_MEDIAPACKAGE_H__
#define __BASE_MEDIAPACKAGE_H__

#include "Base/Defs.h"
#include "Base/IntTypes.h"
#include "Base/Packet.h"
#include "Base/MediaDefine.h"
#include "Base/DynamicMemPool.h"

namespace Xunmei{
namespace Base{

#define MEDIA_V1	1
#define MEDIA_V2	2
#define MEDIA_V3	3

#define MEDIAHEADER_LENGTH_V1		28
#define MEDIAHEADER_LENGTH_V2		24
#define MEDIAHEADER_LENGTH_V3		24

#define MEDIA_VERSION			MEDIA_V3
#define MEDIAHEADER_LENGTH	MEDIAHEADER_LENGTH_V3

typedef struct
{
	uint32_t year	:16;	// 年
	uint32_t millsec:12;	// 毫秒
	uint32_t month	: 4;	//月
	uint32_t date	: 8;	// 日
	uint32_t hour	: 8;	// 小时
	uint32_t minute	: 8;	// 分
	uint32_t second	: 8;	// 秒
} MediaExtTime;//时间信息扩展数据


typedef struct
{
	uint8_t		AlgorithmType;		//加密类型
	uint8_t		Reserved;			//无用
	uint8_t		PasswordLength;		//password字段长度
	uint8_t		IVLength;			//IVData字段长度
	uint32_t	Summary;			//校验位
	uint32_t	FrameSize;			//原始帧数据大小
	uint8_t		Certificate[16];	//证书编码
	uint8_t		Password[256];		//加密密码信息
	uint8_t		IVData[256];		//加密向量信息
}PasswordInfo;

class BASE_API MediaPackage : public Packet
{
	struct MediaInternal;
public:
	/// 缺省构造函数 
	/// \note 当需要一个空包的时候使用
	MediaPackage(IMallcFreeMemObjcPtr* ptr = GDynamicMemObjcPtr);

	/// 构造函数
	/// \param mediaHdr [in] xunmei流的头, 保证是MEDIAHEADER_LENGTH个bytes
	/// \note 网络接收包时使用
	/// \note 构建完后，需要调用isValidPkt() 验证是否包头有效
	///  两种使用方式
	///   1. 直接使用 putbuffer(),构建包。如果putbuffer()返回的值< 实际的length时、
	///       表示,包已经满.
	///   2. 使用getCurDataBuffer getCurUnCopySize updatePlusDataSize getCurUnCopySize相结合取使用，参见测试代码
	MediaPackage(const char* packetHead,IMallcFreeMemObjcPtr* ptr = GDynamicMemObjcPtr);

	/// 构造函数
	/// \param frameSize [in] 流媒体数据包的长度
	/// \note 构建数据包时使用，创建包头和流媒体数据内存空间
	MediaPackage(uint32_t frameSize,IMallcFreeMemObjcPtr* ptr = GDynamicMemObjcPtr);

	/// 构造函数
	/// \param frametype [in]  
	/// \param codecID [in]
	/// \param packageID [in]
	/// \param frameSize [in]
	/// \param frameSeq [in]
	/// \param utcTimestamp [in]
	/// \param timestamp [in]
	/// \param extDataLen [in]  扩展数据的长度 字节长度(一定是4的倍数)
	//根据流格式协议V2.0定义的扩展数据长度值占用3个字节，但实际的扩展数据长度值应该给予适当控制，暂时将其设置为2个字节
	/// \param ver [in] 版本号
	/// \note 构建数据包时使用, 以上参数具体内容 参见讯美流媒体数据格式规范.docx
	MediaPackage(uint8_t frameType, uint8_t codecID, uint8_t packageID, uint32_t frameSize,
		uint32_t frameSeq, uint32_t timestamp,IMallcFreeMemObjcPtr* ptr = GDynamicMemObjcPtr);

	/// 析构函数
	~MediaPackage();

	/// 新旧版本编码转换
	/// \param	ver			旧数据版本	//当前旧版本指 版本为MEDIA_V1/MEDIA_V2
	/// \param	codeid		旧数据视频编码
	/// \param	subcode		旧数据子编码
	/// \param  newcodeid	新版本数据编码
	/// \param  newpacketid	旧版本封包类型
	/// \retval		无
	static void converCodeId(int ver,uint32_t codeid,uint32_t subcode,uint32_t& newcodeid,uint32_t& newpacketid);

	/// 新旧版本帧类型编码转换
	/// \param  ver			旧数据版本 //当前旧版本指 版本为MEDIA_V1/MEDIA_V2
	/// \param  frametype	旧数据帧类型
	/// \param  newframetype 新数据帧类型
	/// \retval		无
	static void converFrameType(int ver,uint32_t frametype,uint32_t& newframetype);
	///
	/// 用于 网络接收包的接口
	/// \note 以下接口的前提是 调用  validPkt() 返回的是true
	///

	/// 获得数据段还未拷贝的长度 (除去头)
	/// \retval 还需要拷贝的数据量
	/// \note 用于网络接收时,确定数据还有多少未接收到
	///       当返回为0 表示 这一个包已经完整
	uint32_t getRemainSize() const;

	/// 拷贝除头以外的扩展和/或流媒体数据
	/// \param remainData [in]更新数据
	/// \retval true 成功
	///         false 失败
	int32_t addRemainData(const char* remainData, uint32_t dataSize);

	///是否是一个有效的包
	/// \retval true	正确、false 错误
	virtual bool isValid(void) const;

	/// 获得版本
	/// \retval 版本号
	uint8_t getVersion() const;

	///获取Package头长度
	/// \retval uint32_t	返回Package头长度
	uint32_t getHeaderOffset() const;

	/// 设置帧类型
	/// \param frameType [in] 
	/// \param subFrameType [in] 
	/// \retval true 成功
	///         false 失败
	bool setFrameType(uint8_t frameType);

	/// 获得FrameType
	/// \retval FrameType
	uint8_t getFrameType() const;

	/// 设置编码器编码类型ID
	/// \param codecID [in]
	/// \retval true 成功
	///         false 失败
	bool setCodecID(uint8_t packageID);

	/// 获得编码器数据编码类型ID
	/// \retval CodecID 
	uint8_t getCodecID() const;

	/// 设置编码后文件打包格式ID
	/// \param codecID [in]
	/// \retval true 成功
	///         false 失败
	bool setPackageID(uint8_t codecID);

	/// 获得编码文件打包格式ID
	/// \retval PackageID 		
	uint8_t getPackageID() const;

	/// 设置帧序
	/// \param seq [in] 序号
	/// \retval true 成功
	///         false 失败
	bool setSequence(uint32_t seq);

	/// 获得 序号
	/// \retval Sequence
	uint32_t getSequence() const;
	
	/// 设置时间戳
	/// \param timeStamp [in] 时间戳
	/// \retval true 成功
	///         false 失败
	bool setTimeStamp(uint32_t timeStamp);
	
	/// 获得 时间戳
		/// \retval 时间戳
	uint32_t getTimeStamp() const;


	/// 获取ExtData的地址, 用于拷贝extData
	/// \retval ExtData的地址
	const char* getExtraData(void) const;

	/// 获得 ExtData长度 
	/// \retval ExtData长度  单位 bytes
	uint32_t getExtraDataSize() const;

	/// 放FrameData
	/// \param data [in] frameSize长度的数据地址
	/// \retval true 成功
	///         false 失败
	bool addFrameData(const char * data);
	
	/// 得到FrameData的地址, 用于拷贝FrameData
	/// \retval FrameData的地址
	const char *getFrameBuffer(void) const;

	/// 获得 Frame Size 
	/// \retval FrameSize 		
	uint32_t getFrameSize() const;

	/// MediaPackage包克隆
	/// \retval  返回一个新的MediaPackage 包、并且内容和当前一样
	/// 外部使用结束够需要对新的包进行delete删除
	MediaPackage* clone(IMallcFreeMemObjcPtr* ptr = GDynamicMemObjcPtr) const;
	
	/// 手动更行校验位
	///	以上的set相关操作都会自动更新校验位
	void updateVerify();

	/// 添加视频分辨率 额外数据
	/// \param width	视频宽度
	/// \param height	视频高度
	/// \retval	true 成功
	///         flase 失败
	/// 重复添加该字段，将自动覆盖已有的数据
	bool addExtraSolution(uint16_t width, uint16_t height);

	///获取视频分辨率 额外数据
	/// \param width	视频宽度
	/// \param height	视频高度
	/// \retval	true 成功
	///         flase 失败
	bool getExtraSolution(uint16_t& width, uint16_t& height) const;

	/// 添加视频帧率 额外数据
	/// \param frameRate	帧率
	/// \retval	true 成功
	///         flase 失败
	/// 重复添加该字段，将自动覆盖已有的数据
	bool addExtraFrameRate(uint8_t frameRate);

	/// 获取视频帧率 额外数据
	/// \param frameRate	帧率
	/// \retval	true 成功
	///         flase 失败
	bool getExtraFrameRate(uint8_t& frameRate) const;

	/// 添加视频码率 额外数据
	/// \param bitCtrlType	码率控制类型	0:CBR(固定码率) 1:VBR(可变码率)
	/// \param bitRate		码率大小
	/// \retval	true 成功
	///         flase 失败
	/// 重复添加该字段，将自动覆盖已有的数据
	bool addExtraBitrate(uint8_t bitCtrlType, uint32_t bitRate);

	/// 获取视频码率 额外数据
	/// \param bitCtrlType	码率控制类型	0:CBR(固定码率) 1:VBR(可变码率)
	/// \param bitRate		码率大小
	/// \retval	true 成功
	///         flase 失败
	bool getExtraBitrate(uint8_t& bitCtrlType, uint32_t& bitRate) const;

	/// 添加时间信息 额外数据
	/// \param time		 时间信息
	/// \retval	true 成功
	///         flase 失败
	/// 重复添加该字段，将自动覆盖已有的数据
	bool addExtraTimeStamp(const MediaExtTime& time);

	/// 获取时间信息 额外数据
	/// \param time		 时间信息
	/// \retval	true 成功
	///         flase 失败
	bool getExtraTimeStamp(MediaExtTime& exTime) const;

	/// 添加经纬度 额外数据
	/// \param longitude		 经度值
	/// \param latitude			 纬度值
	/// \retval	true 成功
	///         flase 失败
	/// 重复添加该字段，将自动覆盖已有的数据
	bool addExtraPostion(const uint32_t longitude, const uint32_t latitude);

	/// 获取经纬度 额外数据
	/// \param longitude		 经度值
	/// \param latitude			 纬度值
	/// \retval	true 成功
	///         flase 失败
	bool getExtraPostion(uint32_t& longitude, uint32_t& latitude) const;

	/// 添加视频流统计数据 额外数据
	/// \param gopSeq			GOP顺序索引
	/// \param downSpeed		下行速率
	/// \param frameRate		帧率
	/// \param ifCount			I帧总数
	/// \param pfCount			P帧总数
	/// \param bfCount			B帧总数
	/// \retval	true 成功
	///         flase 失败
	/// 重复添加该字段，将自动覆盖已有的数据
	bool addExtraStatistic(uint16_t gopSeq, uint16_t downSpeed, uint8_t frameRate, uint8_t ifCount, uint8_t pfCount, uint8_t bfCount);

	/// 获取视频流统计数据 额外数据
	/// \param gopSeq			GOP顺序索引
	/// \param downSpeed		下行速率
	/// \param frameRate		帧率
	/// \param ifCount			I帧总数
	/// \param pfCount			P帧总数
	/// \param bfCount			B帧总数
	/// \retval	true 成功
	///         flase 失败
	bool getExtraStatistic(uint16_t& gopSeq, uint16_t& downSpeed, uint8_t& frameRate, uint8_t& ifCount, uint8_t& pfCount, uint8_t& bfCount) const;


	/// 添加视频编解码信息
	/// \param vCodecId			视频编码ID
	/// \param aCodecId			音频编码ID
	/// \retval true 成功
	///			false 失败
	bool addExtraCodecInfo(MEDIA_CodecID vCodecId,MEDIA_CodecID aCodecId);

	/// 获取视频编解码信息
	/// \param vCodecId			视频编码ID
	/// \param aCodecId			音频编码ID
	/// \retval true 成功
	///			false 失败
	bool getExtraCodecInfo(MEDIA_CodecID& vCodecId,MEDIA_CodecID& aCodecId) const;

	/// 添加音频信息
	/// \param audioChannels			音频通道数,如单通道为1
	/// \param audioBitsPerSample		音频采样字节,如16位采样为16
	/// \param audioSamplesPerSecond	音频采样率,如采样频率为16000
	/// \retval true 成功
	///			false 失败
	bool addExtraAudioInfo(uint8_t audioChannels, uint8_t audioBitsPerSamples, uint16_t audioSamplesPerSecond);

	/// 获取音频信息
	/// \param audioChannels			音频通道数,如单通道为1
	/// \param audioBitsPerSample		音频采样字节,如16位采样为16
	/// \param audioSamplesPerSecond	音频采样率,如采样频率为16000
	/// \retval true 成功
	///			false 失败
	bool getExtraAudioInfo(uint8_t& audioChannels, uint8_t& audioBitsPerSamples, uint16_t& audioSamplesPerSecond) const;

	/// 添加加密信息
	/// \param encrypts					加密次数
	/// \param encryptData				加密数据指针
	/// \retval true 成功
	///			false 失败
	bool addExtraEncryptInfo(const PasswordInfo& encryptInfo);

	/// 获取加密信息
	/// \param encrypts					//加密次数
	/// \param encryptData				//加密数据指针
	/// \retval true 成功
	///			false 失败
	bool getExtraEncryptInfo(PasswordInfo& encryptInfo);

	/// 添加用户自定义数据 额外数据
	/// \param type				数据类型
	/// \param data				数据
	/// \param size				数据长度
	/// \retval	true 成功
	///         flase 失败
	/// 重复添加统一type数据，将自动覆盖已有的数据
	bool addCustomData(int type,const char* data,const uint16_t size);

	/// 获取用户自定义数据 额外数据
	/// \param type				数据类型
	/// \param size				数据长度
	/// \retval	NULL 表示无该数据
	const char* getCustomData(int type,uint16_t& size) const;
private:
	char* findExtraData(uint8_t flag,uint32_t& len) const;
	bool appendExtraData(int flag,void* data,int datalen);
private:
	MediaInternal*	internal;
};

} // namespace Base
} // namespace Xunmei

#endif //__BASE_MEDIAPACKAGE_H__


