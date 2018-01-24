#include "Write_Color.h"

namespace Public {
namespace Excel {

XM_Excel::Color::Color()
{
	internal = new ColorInternal();
}
XM_Excel::Color::Color(const Color& color)
{
	internal = new ColorInternal(color.internal->c_r, color.internal->c_g, color.internal->c_b);
}
XM_Excel::Color::Color(uint8_t _r, uint8_t _g, uint8_t _b)
{
	internal = new ColorInternal(_r, _g, _b);
}
XM_Excel::Color& XM_Excel::Color::operator = (const Color& val)
{
	internal->c_r = val.internal->c_r;
	internal->c_g = val.internal->c_g;
	internal->c_b = val.internal->c_b;

	return *this;
}
XM_Excel::Color:: ~Color()
{
	SAFE_DELETE(internal);
}

}
}