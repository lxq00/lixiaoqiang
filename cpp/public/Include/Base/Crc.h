//
//  Copyright (c)1998-2012, Public Technology
//  All Rights Reserved.
//
//	Description:
//	$Id: Crc.h 3 2013-01-21 06:57:38Z jiangwei $
//

#ifndef __BASE_CRC_ORDER_H__
#define __BASE_CRC_ORDER_H__

#include <stddef.h>
#include "Base/IntTypes.h"
#include "Defs.h"


namespace Public {
namespace Base {


/// \class Crc 
/// \brief Crc 的方法类

class BASE_API Crc
{
public:
	///	crc16 - compute the CRC-16 for the data buffer
	///	\param [in] crc: previous CRC value
	///	\param [in] buffer: data pointer
	///	\param [in] len: number of bytes in the buffer
	///	\retval  The updated CRC value.
	static uint16_t to16bit(uint16_t crc, uint8_t const* buffer, size_t length);

	///	crc32 - compute the CRC-32 for the data buffer
	///	\param [in] buffer: data pointer
	///	\param [in] len: number of bytes in the buffer
	///	\retval The updated CRC value.
	static uint32_t to32bit (uint8_t const* buffer, size_t length);
};

} // namespace Base
} // namespace Public

#endif// __Public_CRC_ORDER_H__


