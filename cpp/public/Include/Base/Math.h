//
//  Copyright (c)1998-2012, Public Technology
//  All Rights Reserved.
//
//	Description:
//	$Id: Math.h 3 2013-01-21 06:57:38Z jiangwei $
//

#ifndef __BASE_MATH_H__
#define __BASE_MATH_H__

#include "Base/IntTypes.h"
#include "Defs.h"

namespace Public {
namespace Base {

/// \param x 无符号整形变量值
/// \return 对数值整数
inline int log2i(uint32_t x)
{
	int r = 31;

	if (!x)
		return 0;
	if (!(x & 0xffff0000u)) {
		x <<= 16;
		r -= 16;
	}
	if (!(x & 0xff000000u)) {
		x <<= 8;
		r -= 8;
	}
	if (!(x & 0xf0000000u)) {
		x <<= 4;
		r -= 4;
	}
	if (!(x & 0xc0000000u)) {
		x <<= 2;
		r -= 2;
	}
	if (!(x & 0x80000000u)) {
		x <<= 1;
		r -= 1;
	}
	return r;
}

} // namespace Base
} // namespace Public

#endif// __BASE_MATH_H__

