#include "Write_Side.h"
#include "xlslib.h"

using namespace Public;
using namespace Excel;

WorkBook::Side::Side(int val, const shared_ptr<Color>& sideColor)
{
	internal = new LineInternal;
	internal->side = val;
	internal->color = sideColor;
}
WorkBook::Side::Side(int val, const Color& sideColor)
{
	internal = new LineInternal;
	internal->side = val;
	internal->color = make_shared<Color>(sideColor);
}
WorkBook::Side::Side(const Side& side)
{
	internal = new LineInternal;
	*internal = *side.internal;
}
WorkBook::Side& WorkBook::Side::operator = (const Side& val)
{
	*internal = *val.internal;

	return *this;
}

WorkBook::Side::~Side()
{
	SAFE_DELETE(internal);
}
