#include "Write_Format.h"

using namespace Xunmei;
using namespace Excel;

XM_Excel::Format::Format()
{
	internal = new FormatInternal();
}
XM_Excel::Format::Format(const Format& format)
{
	internal = new FormatInternal();
	*internal = *format.internal;
}
XM_Excel::Format& XM_Excel::Format::operator = (const Format& val)
{
	*internal = *val.internal;

	return *this;
}

XM_Excel::Format::~Format()
{
	SAFE_DELETE(internal);
}

//��ʾ��ʾ��ʽ
bool XM_Excel::Format::setFormat(FomatType type)
{
	internal->fomat = type;
	return true;
}

//���ö��뷽ʽ
bool XM_Excel::Format::setAlign(ALIGN_X_Type xalign, ALIGN_Y_Type yalign)
{ 
	internal->xalign = xalign;
	internal->yalign = yalign;
	
	return true;
}

//������ʾ��ʽ
bool XM_Excel::Format::setTxtori(TxtoriType type)
{ 
	internal->orient = type;
	return false; 
}