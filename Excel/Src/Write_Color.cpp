#include "Write_Color.h"

namespace Public {
namespace Excel {

WorkBook::Color::Color()
{
	internal = new ColorInternal();
}
WorkBook::Color::Color(const Color& color)
{
	internal = new ColorInternal(color.internal->c_r, color.internal->c_g, color.internal->c_b);
}
WorkBook::Color::Color(uint8_t _r, uint8_t _g, uint8_t _b)
{
	internal = new ColorInternal(_r, _g, _b);
}
WorkBook::Color& WorkBook::Color::operator = (const Color& val)
{
	internal->c_r = val.internal->c_r;
	internal->c_g = val.internal->c_g;
	internal->c_b = val.internal->c_b;

	return *this;
}
WorkBook::Color:: ~Color()
{
	SAFE_DELETE(internal);
}

}
}