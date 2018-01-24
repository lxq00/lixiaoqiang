#include "Write_Side.h"
#include "xlslib.h"

using namespace Public;
using namespace Excel;

XM_Excel::Side::Side(int val, const shared_ptr<Color>& sideColor)
{
	internal = new LineInternal;
	internal->side = val;
	internal->color = sideColor;
}
XM_Excel::Side::Side(int val, const Color& sideColor)
{
	internal = new LineInternal;
	internal->side = val;
	internal->color = new Color(sideColor);
}
XM_Excel::Side::Side(const Side& side)
{
	internal = new LineInternal;
	*internal = *side.internal;
}
XM_Excel::Side& XM_Excel::Side::operator = (const Side& val)
{
	*internal = *val.internal;

	return *this;
}

XM_Excel::Side::~Side()
{
	SAFE_DELETE(internal);
}
