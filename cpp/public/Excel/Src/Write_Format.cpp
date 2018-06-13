#include "Write_Format.h"

using namespace Public;
using namespace Excel;

WorkBook::Format::Format()
{
	internal = new FormatInternal();
}
WorkBook::Format::Format(const Format& format)
{
	internal = new FormatInternal();
	*internal = *format.internal;
}
WorkBook::Format& WorkBook::Format::operator = (const Format& val)
{
	*internal = *val.internal;

	return *this;
}

WorkBook::Format::~Format()
{
	SAFE_DELETE(internal);
}

//显示显示格式
bool WorkBook::Format::setFormat(FomatType type)
{
	internal->fomat = type;
	return true;
}

//设置对齐方式
bool WorkBook::Format::setAlign(ALIGN_X_Type xalign, ALIGN_Y_Type yalign)
{ 
	internal->xalign = xalign;
	internal->yalign = yalign;
	
	return true;
}

//设置显示方式
bool WorkBook::Format::setTxtori(TxtoriType type)
{ 
	internal->orient = type;
	return false; 
}