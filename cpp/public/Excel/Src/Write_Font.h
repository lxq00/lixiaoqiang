#ifndef __WRITE_FONT_H__
#define __WRITE_FONT_H__
#include "Excel/Excel.h"

namespace Xunmei {
namespace Excel {

struct XM_Excel::Font::FontInternal
{
	FontInternal() :font_size(0), font_bold(false), font_underline(false) {}

	std::string					font_name;
	uint16_t					font_size;
	bool						font_bold;
	bool						font_underline;
	shared_ptr<XM_Excel::Color> font_color;

	static bool IsASCII(const std::string& str)
	{
		std::string::const_iterator cBegin, cEnd;

		cBegin = str.begin();
		cEnd = str.end();

		uint16_t c = 0;

		while (cBegin != cEnd) 
		{
			c |= *cBegin++;
		}

		return c <= 0x7F;
	}
};

}
}

#endif //__WRITE_FONT_H__