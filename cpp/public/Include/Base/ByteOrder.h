//
//  Copyright (c)1998-2012, Public Technology
//  All Rights Reserved.
//
//	Description:
//	$Id: ByteOrder.h 3 2013-01-21 06:57:38Z jiangwei $
//

#ifndef __BASE_BYTEORDER_H__
#define __BASE_BYTEORDER_H__

#include "Base/IntTypes.h"

namespace Public {
namespace Base {

/// 交换16bit的整型结构
///  \param x [in] 原值
///  \retval 交换后的值
inline uint16_t swap_word(uint16_t x)
{
	return ((((uint16_t)(x)) & 0xff00) >> 8 | (((uint16_t)(x)) & 0x00ff) << 8);
}

/// 交换32bit的整型结构
///  \param x [in] 原值
///  \retval 交换后的值
inline uint32_t swap_long(uint32_t x)
{
	return (((((uint32_t)(x)) & 0xff000000) >> 24) |
	((((uint32_t)(x)) & 0x00ff0000) >> 8) |
	((((uint32_t)(x)) & 0x0000ff00) << 8) |
	((((uint32_t)(x)) & 0x000000ff) << 24)
	);
}

/// 交换64bit的整型结构
///  \param x [in] 原值
///  \retval 交换后的值
inline uint64_t swap_longlong(uint64_t x)
{
#ifdef __GNUC__
	return (((((uint64_t)(x)) & 0xff00000000000000llu) >> 56) |
	((((uint64_t)(x)) & 0x00ff000000000000llu) >> 40) |
	((((uint64_t)(x)) & 0x0000ff0000000000llu) >> 24) |
	((((uint64_t)(x)) & 0x000000ff00000000llu) >> 8) |
	((((uint64_t)(x)) & 0x00000000ff000000llu) << 8) |
	((((uint64_t)(x)) & 0x0000000000ff0000llu) << 24) |
	((((uint64_t)(x)) & 0x000000000000ff00llu) << 40) |
	((((uint64_t)(x)) & 0x00000000000000ffllu) << 56)
	);
#else
	return (((((uint64_t)(x)) & 0xff00000000000000) >> 56) |
	((((uint64_t)(x)) & 0x00ff000000000000) >> 40) |
	((((uint64_t)(x)) & 0x0000ff0000000000) >> 24) |
	((((uint64_t)(x)) & 0x000000ff00000000) >> 8) |
	((((uint64_t)(x)) & 0x00000000ff000000) << 8) |
	((((uint64_t)(x)) & 0x0000000000ff0000) << 24) |
	((((uint64_t)(x)) & 0x000000000000ff00) << 40) |
	((((uint64_t)(x)) & 0x00000000000000ff) << 56)
	);
#endif
}

/// 判断系统的是否是大端
/// \reval true  大端系统
/// \reval false 小端系统
inline bool is_big_endian(void)
{
    union 
   	{
       uint16_t data;
       char str[2];
    } tmp = {0x1234};
    
    return (tmp.str[0] == 0x12);
}

template <int v>
struct Int2Type
{
	enum {value = v};
};

/// \Template Function flipToLittle
/// \brief  本地数据转换为小端数据
template <typename T>
inline void flipSelfToLittle(T& x)
{
	x = flipToLittle(x);
}

template <typename T>
inline T flipToLittle(const T x)
{
	return flipToLittle(x, Int2Type<sizeof(T)>());
}

template <typename T>
inline T flipToLittle(const T x, Int2Type<1>)
{
	return x;
}

template <typename T>
inline T flipToLittle(const T x, Int2Type<2>)
{
	return is_big_endian() ? swap_word(x) : x;
}

template <typename T>
inline T flipToLittle(const T x, Int2Type<4>)
{
	return is_big_endian() ? swap_long(x) : x;
}

template <typename T>
inline T flipToLittle(const T x, Int2Type<8>)
{
	return is_big_endian() ? swap_longlong(x) : x;
}

/// \Template Function flipFromLittle
/// \brief  小端数据转换为本地数据
template <typename T>
inline void flipSelfFromLittle(T& x)
{
	x = flipFromLittle(x);
}

template <typename T>
inline T flipFromLittle(const T x)
{
	return flipFromLittle(x, Int2Type<sizeof(T)>());
}

template <typename T>
inline T flipFromLittle(const T x, Int2Type<1>)
{
	return x;
}

template <typename T>
inline T flipFromLittle(const T x, Int2Type<2>)
{
	return is_big_endian() ? swap_word(x) : x;
}

template <typename T>
inline T flipFromLittle(const T x, Int2Type<4>)
{
	return is_big_endian() ? swap_long(x) : x;
}

template <typename T>
inline T flipFromLittle(const T x, Int2Type<8>)
{
	return is_big_endian() ? swap_longlong(x) : x;
}

/// \Template Function flipToBig
/// \brief  本地数据转换为大端数据
template <typename T>
inline void flipSelfToBig(T& x)
{
	x = flipToBig(x);
}

template <typename T>
inline T flipToBig(const T x)
{
	return flipToBig(x, Int2Type<sizeof(T)>());
}

template <typename T>
inline T flipToBig(const T x, Int2Type<1>)
{
	return x;
}

template <typename T>
inline T flipToBig(const T x, Int2Type<2>)
{
	return is_big_endian() ? x : swap_word(x);
}

template <typename T>
inline T flipToBig(const T x, Int2Type<4>)
{
	return is_big_endian() ? x : swap_long(x);
}

template <typename T>
inline T flipToBig(const T x, Int2Type<8>)
{
	return is_big_endian() ? x : swap_longlong(x);
}

///////////////////////////////////////////////
//////////  flipFromBig
/// \Template Function flipFromLittle
/// \brief  大端数据转换为本地数据
template <typename T>
inline void flipSelfFromBig(T& x)
{
	x = flipFromBig(x);
}

template <typename T>
inline T flipFromBig(const T x)
{
	return flipFromBig(x, Int2Type<sizeof(T)>());
}

template <typename T>
inline T flipFromBig(const T x, Int2Type<1>)
{
	return x;
}

template <typename T>
inline T flipFromBig(const T x, Int2Type<2>)
{
	return is_big_endian() ? x : swap_word(x);
}

template <typename T>
inline T flipFromBig(const T x, Int2Type<4>)
{
	return is_big_endian() ? x : swap_long(x);
}

template <typename T>
inline T flipFromBig(const T x, Int2Type<8>)
{
	return is_big_endian() ? x : swap_longlong(x);
}

} // namespace Base
} // namespace Public

#endif// __BASE_BYTEORDER_H__

