#ifndef __WRITE_COLOR_H__
#define __WRITE_COLOR_H__
#include "Write_WorkBook.h"

namespace Public {
namespace Excel {

struct WorkBook::Color::ColorInternal
{
	uint8_t c_r;
	uint8_t c_g;
	uint8_t c_b;

	ColorInternal() {}
	ColorInternal(uint8_t _r, uint8_t _g, uint8_t _b) :c_r(_r), c_g(_g), c_b(_b) {}
};
}
}
#endif //__WRITE_COLOR_H__