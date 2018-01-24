//
//  Copyright (c)1998-2012, Xunmei Technology
//  All Rights Reserved.
//
//	Description:
//	$Id: Packet.h 255 2014-01-23 03:28:32Z lixiaoqiang $

#ifndef __BASE_PACKET_H__
#define __BASE_PACKET_H__
		
#include "Base/IntTypes.h"
#include "Base/Func.h"
#include "Defs.h"
#include "Base/DynamicMemPool.h"

namespace Xunmei {
namespace Base {

////////////////////////////////////////////////////////////////////////////////
/// \class Packet
/// \brief ���ݰ��������
class BASE_API Packet
{
	struct PacketInternal;
public:

	/// ȱʡ���캯��,����һ����Ч�İ�,��Ҫ��ֵ����ʹ��
	Packet(IMallcFreeMemObjcPtr* ptr);

	/// ���캯��,����һ������ʹ�õİ�
	/// \param dataSize 	[in]  	����İ���������Ч���ݻ�����ֽ���
	Packet(IMallcFreeMemObjcPtr* ptr,uint32_t size);

	/// ��������,��ɶ԰��������ü����ݼ�
	virtual ~Packet();

	/// �ж��Ƿ�Ϊ��Ч��
	virtual bool isValid(void) const;

	/// �õ����������ʼλ�õ�ָ��
	/// \retval �����׵�ַ
	uint8_t* getBuffer(void) const;

	uint32_t size(void) const;
protected:
	void reinit(uint32_t size);
	void removePacket(void);
	char* remalloc(uint32_t& size);
	bool resetBuffer(uint8_t* buffer,uint32_t size,uint32_t maxSize);
	bool addDataLength(uint32_t len);
	int getDataLen() const;
private:
	PacketInternal*			internal;
	IMallcFreeMemObjcPtr*	memptr;
};

} // namespace Base
} // namespace Xunmei

#endif	// __BASE_PACKET_H__

