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
	uint32_t year	:16;	// ��
	uint32_t millsec:12;	// ����
	uint32_t month	: 4;	//��
	uint32_t date	: 8;	// ��
	uint32_t hour	: 8;	// Сʱ
	uint32_t minute	: 8;	// ��
	uint32_t second	: 8;	// ��
} MediaExtTime;//ʱ����Ϣ��չ����


typedef struct
{
	uint8_t		AlgorithmType;		//��������
	uint8_t		Reserved;			//����
	uint8_t		PasswordLength;		//password�ֶγ���
	uint8_t		IVLength;			//IVData�ֶγ���
	uint32_t	Summary;			//У��λ
	uint32_t	FrameSize;			//ԭʼ֡���ݴ�С
	uint8_t		Certificate[16];	//֤�����
	uint8_t		Password[256];		//����������Ϣ
	uint8_t		IVData[256];		//����������Ϣ
}PasswordInfo;

class BASE_API MediaPackage : public Packet
{
	struct MediaInternal;
public:
	/// ȱʡ���캯�� 
	/// \note ����Ҫһ���հ���ʱ��ʹ��
	MediaPackage(IMallcFreeMemObjcPtr* ptr = GDynamicMemObjcPtr);

	/// ���캯��
	/// \param mediaHdr [in] xunmei����ͷ, ��֤��MEDIAHEADER_LENGTH��bytes
	/// \note ������հ�ʱʹ��
	/// \note ���������Ҫ����isValidPkt() ��֤�Ƿ��ͷ��Ч
	///  ����ʹ�÷�ʽ
	///   1. ֱ��ʹ�� putbuffer(),�����������putbuffer()���ص�ֵ< ʵ�ʵ�lengthʱ��
	///       ��ʾ,���Ѿ���.
	///   2. ʹ��getCurDataBuffer getCurUnCopySize updatePlusDataSize getCurUnCopySize����ȡʹ�ã��μ����Դ���
	MediaPackage(const char* packetHead,IMallcFreeMemObjcPtr* ptr = GDynamicMemObjcPtr);

	/// ���캯��
	/// \param frameSize [in] ��ý�����ݰ��ĳ���
	/// \note �������ݰ�ʱʹ�ã�������ͷ����ý�������ڴ�ռ�
	MediaPackage(uint32_t frameSize,IMallcFreeMemObjcPtr* ptr = GDynamicMemObjcPtr);

	/// ���캯��
	/// \param frametype [in]  
	/// \param codecID [in]
	/// \param packageID [in]
	/// \param frameSize [in]
	/// \param frameSeq [in]
	/// \param utcTimestamp [in]
	/// \param timestamp [in]
	/// \param extDataLen [in]  ��չ���ݵĳ��� �ֽڳ���(һ����4�ı���)
	//��������ʽЭ��V2.0�������չ���ݳ���ֵռ��3���ֽڣ���ʵ�ʵ���չ���ݳ���ֵӦ�ø����ʵ����ƣ���ʱ��������Ϊ2���ֽ�
	/// \param ver [in] �汾��
	/// \note �������ݰ�ʱʹ��, ���ϲ����������� �μ�Ѷ����ý�����ݸ�ʽ�淶.docx
	MediaPackage(uint8_t frameType, uint8_t codecID, uint8_t packageID, uint32_t frameSize,
		uint32_t frameSeq, uint32_t timestamp,IMallcFreeMemObjcPtr* ptr = GDynamicMemObjcPtr);

	/// ��������
	~MediaPackage();

	/// �¾ɰ汾����ת��
	/// \param	ver			�����ݰ汾	//��ǰ�ɰ汾ָ �汾ΪMEDIA_V1/MEDIA_V2
	/// \param	codeid		��������Ƶ����
	/// \param	subcode		�������ӱ���
	/// \param  newcodeid	�°汾���ݱ���
	/// \param  newpacketid	�ɰ汾�������
	/// \retval		��
	static void converCodeId(int ver,uint32_t codeid,uint32_t subcode,uint32_t& newcodeid,uint32_t& newpacketid);

	/// �¾ɰ汾֡���ͱ���ת��
	/// \param  ver			�����ݰ汾 //��ǰ�ɰ汾ָ �汾ΪMEDIA_V1/MEDIA_V2
	/// \param  frametype	������֡����
	/// \param  newframetype ������֡����
	/// \retval		��
	static void converFrameType(int ver,uint32_t frametype,uint32_t& newframetype);
	///
	/// ���� ������հ��Ľӿ�
	/// \note ���½ӿڵ�ǰ���� ����  validPkt() ���ص���true
	///

	/// ������ݶλ�δ�����ĳ��� (��ȥͷ)
	/// \retval ����Ҫ������������
	/// \note �����������ʱ,ȷ�����ݻ��ж���δ���յ�
	///       ������Ϊ0 ��ʾ ��һ�����Ѿ�����
	uint32_t getRemainSize() const;

	/// ������ͷ�������չ��/����ý������
	/// \param remainData [in]��������
	/// \retval true �ɹ�
	///         false ʧ��
	int32_t addRemainData(const char* remainData, uint32_t dataSize);

	///�Ƿ���һ����Ч�İ�
	/// \retval true	��ȷ��false ����
	virtual bool isValid(void) const;

	/// ��ð汾
	/// \retval �汾��
	uint8_t getVersion() const;

	///��ȡPackageͷ����
	/// \retval uint32_t	����Packageͷ����
	uint32_t getHeaderOffset() const;

	/// ����֡����
	/// \param frameType [in] 
	/// \param subFrameType [in] 
	/// \retval true �ɹ�
	///         false ʧ��
	bool setFrameType(uint8_t frameType);

	/// ���FrameType
	/// \retval FrameType
	uint8_t getFrameType() const;

	/// ���ñ�������������ID
	/// \param codecID [in]
	/// \retval true �ɹ�
	///         false ʧ��
	bool setCodecID(uint8_t packageID);

	/// ��ñ��������ݱ�������ID
	/// \retval CodecID 
	uint8_t getCodecID() const;

	/// ���ñ�����ļ������ʽID
	/// \param codecID [in]
	/// \retval true �ɹ�
	///         false ʧ��
	bool setPackageID(uint8_t codecID);

	/// ��ñ����ļ������ʽID
	/// \retval PackageID 		
	uint8_t getPackageID() const;

	/// ����֡��
	/// \param seq [in] ���
	/// \retval true �ɹ�
	///         false ʧ��
	bool setSequence(uint32_t seq);

	/// ��� ���
	/// \retval Sequence
	uint32_t getSequence() const;
	
	/// ����ʱ���
	/// \param timeStamp [in] ʱ���
	/// \retval true �ɹ�
	///         false ʧ��
	bool setTimeStamp(uint32_t timeStamp);
	
	/// ��� ʱ���
		/// \retval ʱ���
	uint32_t getTimeStamp() const;


	/// ��ȡExtData�ĵ�ַ, ���ڿ���extData
	/// \retval ExtData�ĵ�ַ
	const char* getExtraData(void) const;

	/// ��� ExtData���� 
	/// \retval ExtData����  ��λ bytes
	uint32_t getExtraDataSize() const;

	/// ��FrameData
	/// \param data [in] frameSize���ȵ����ݵ�ַ
	/// \retval true �ɹ�
	///         false ʧ��
	bool addFrameData(const char * data);
	
	/// �õ�FrameData�ĵ�ַ, ���ڿ���FrameData
	/// \retval FrameData�ĵ�ַ
	const char *getFrameBuffer(void) const;

	/// ��� Frame Size 
	/// \retval FrameSize 		
	uint32_t getFrameSize() const;

	/// MediaPackage����¡
	/// \retval  ����һ���µ�MediaPackage �����������ݺ͵�ǰһ��
	/// �ⲿʹ�ý�������Ҫ���µİ�����deleteɾ��
	MediaPackage* clone(IMallcFreeMemObjcPtr* ptr = GDynamicMemObjcPtr) const;
	
	/// �ֶ�����У��λ
	///	���ϵ�set��ز��������Զ�����У��λ
	void updateVerify();

	/// �����Ƶ�ֱ��� ��������
	/// \param width	��Ƶ���
	/// \param height	��Ƶ�߶�
	/// \retval	true �ɹ�
	///         flase ʧ��
	/// �ظ���Ӹ��ֶΣ����Զ��������е�����
	bool addExtraSolution(uint16_t width, uint16_t height);

	///��ȡ��Ƶ�ֱ��� ��������
	/// \param width	��Ƶ���
	/// \param height	��Ƶ�߶�
	/// \retval	true �ɹ�
	///         flase ʧ��
	bool getExtraSolution(uint16_t& width, uint16_t& height) const;

	/// �����Ƶ֡�� ��������
	/// \param frameRate	֡��
	/// \retval	true �ɹ�
	///         flase ʧ��
	/// �ظ���Ӹ��ֶΣ����Զ��������е�����
	bool addExtraFrameRate(uint8_t frameRate);

	/// ��ȡ��Ƶ֡�� ��������
	/// \param frameRate	֡��
	/// \retval	true �ɹ�
	///         flase ʧ��
	bool getExtraFrameRate(uint8_t& frameRate) const;

	/// �����Ƶ���� ��������
	/// \param bitCtrlType	���ʿ�������	0:CBR(�̶�����) 1:VBR(�ɱ�����)
	/// \param bitRate		���ʴ�С
	/// \retval	true �ɹ�
	///         flase ʧ��
	/// �ظ���Ӹ��ֶΣ����Զ��������е�����
	bool addExtraBitrate(uint8_t bitCtrlType, uint32_t bitRate);

	/// ��ȡ��Ƶ���� ��������
	/// \param bitCtrlType	���ʿ�������	0:CBR(�̶�����) 1:VBR(�ɱ�����)
	/// \param bitRate		���ʴ�С
	/// \retval	true �ɹ�
	///         flase ʧ��
	bool getExtraBitrate(uint8_t& bitCtrlType, uint32_t& bitRate) const;

	/// ���ʱ����Ϣ ��������
	/// \param time		 ʱ����Ϣ
	/// \retval	true �ɹ�
	///         flase ʧ��
	/// �ظ���Ӹ��ֶΣ����Զ��������е�����
	bool addExtraTimeStamp(const MediaExtTime& time);

	/// ��ȡʱ����Ϣ ��������
	/// \param time		 ʱ����Ϣ
	/// \retval	true �ɹ�
	///         flase ʧ��
	bool getExtraTimeStamp(MediaExtTime& exTime) const;

	/// ��Ӿ�γ�� ��������
	/// \param longitude		 ����ֵ
	/// \param latitude			 γ��ֵ
	/// \retval	true �ɹ�
	///         flase ʧ��
	/// �ظ���Ӹ��ֶΣ����Զ��������е�����
	bool addExtraPostion(const uint32_t longitude, const uint32_t latitude);

	/// ��ȡ��γ�� ��������
	/// \param longitude		 ����ֵ
	/// \param latitude			 γ��ֵ
	/// \retval	true �ɹ�
	///         flase ʧ��
	bool getExtraPostion(uint32_t& longitude, uint32_t& latitude) const;

	/// �����Ƶ��ͳ������ ��������
	/// \param gopSeq			GOP˳������
	/// \param downSpeed		��������
	/// \param frameRate		֡��
	/// \param ifCount			I֡����
	/// \param pfCount			P֡����
	/// \param bfCount			B֡����
	/// \retval	true �ɹ�
	///         flase ʧ��
	/// �ظ���Ӹ��ֶΣ����Զ��������е�����
	bool addExtraStatistic(uint16_t gopSeq, uint16_t downSpeed, uint8_t frameRate, uint8_t ifCount, uint8_t pfCount, uint8_t bfCount);

	/// ��ȡ��Ƶ��ͳ������ ��������
	/// \param gopSeq			GOP˳������
	/// \param downSpeed		��������
	/// \param frameRate		֡��
	/// \param ifCount			I֡����
	/// \param pfCount			P֡����
	/// \param bfCount			B֡����
	/// \retval	true �ɹ�
	///         flase ʧ��
	bool getExtraStatistic(uint16_t& gopSeq, uint16_t& downSpeed, uint8_t& frameRate, uint8_t& ifCount, uint8_t& pfCount, uint8_t& bfCount) const;


	/// �����Ƶ�������Ϣ
	/// \param vCodecId			��Ƶ����ID
	/// \param aCodecId			��Ƶ����ID
	/// \retval true �ɹ�
	///			false ʧ��
	bool addExtraCodecInfo(MEDIA_CodecID vCodecId,MEDIA_CodecID aCodecId);

	/// ��ȡ��Ƶ�������Ϣ
	/// \param vCodecId			��Ƶ����ID
	/// \param aCodecId			��Ƶ����ID
	/// \retval true �ɹ�
	///			false ʧ��
	bool getExtraCodecInfo(MEDIA_CodecID& vCodecId,MEDIA_CodecID& aCodecId) const;

	/// �����Ƶ��Ϣ
	/// \param audioChannels			��Ƶͨ����,�絥ͨ��Ϊ1
	/// \param audioBitsPerSample		��Ƶ�����ֽ�,��16λ����Ϊ16
	/// \param audioSamplesPerSecond	��Ƶ������,�����Ƶ��Ϊ16000
	/// \retval true �ɹ�
	///			false ʧ��
	bool addExtraAudioInfo(uint8_t audioChannels, uint8_t audioBitsPerSamples, uint16_t audioSamplesPerSecond);

	/// ��ȡ��Ƶ��Ϣ
	/// \param audioChannels			��Ƶͨ����,�絥ͨ��Ϊ1
	/// \param audioBitsPerSample		��Ƶ�����ֽ�,��16λ����Ϊ16
	/// \param audioSamplesPerSecond	��Ƶ������,�����Ƶ��Ϊ16000
	/// \retval true �ɹ�
	///			false ʧ��
	bool getExtraAudioInfo(uint8_t& audioChannels, uint8_t& audioBitsPerSamples, uint16_t& audioSamplesPerSecond) const;

	/// ��Ӽ�����Ϣ
	/// \param encrypts					���ܴ���
	/// \param encryptData				��������ָ��
	/// \retval true �ɹ�
	///			false ʧ��
	bool addExtraEncryptInfo(const PasswordInfo& encryptInfo);

	/// ��ȡ������Ϣ
	/// \param encrypts					//���ܴ���
	/// \param encryptData				//��������ָ��
	/// \retval true �ɹ�
	///			false ʧ��
	bool getExtraEncryptInfo(PasswordInfo& encryptInfo);

	/// ����û��Զ������� ��������
	/// \param type				��������
	/// \param data				����
	/// \param size				���ݳ���
	/// \retval	true �ɹ�
	///         flase ʧ��
	/// �ظ����ͳһtype���ݣ����Զ��������е�����
	bool addCustomData(int type,const char* data,const uint16_t size);

	/// ��ȡ�û��Զ������� ��������
	/// \param type				��������
	/// \param size				���ݳ���
	/// \retval	NULL ��ʾ�޸�����
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


