#include "Write_Font.h"

namespace Xunmei {
namespace Excel {

shared_ptr<XM_Excel::Font> XM_Excel::Font::create(const std::string& name)
{
	if (name == "" || !FontInternal::IsASCII(name))
	{
		return shared_ptr<XM_Excel::Font>();
	}
	shared_ptr<XM_Excel::Font> Ffont(new XM_Excel::Font);
	Ffont->internal->font_name = name;
	return Ffont;
}
XM_Excel::Font::Font()
{
	internal = new FontInternal;
}
XM_Excel::Font::Font(const Font& font)
{
	internal = new FontInternal;
	*internal = *font.internal;
}
XM_Excel::Font& XM_Excel::Font::operator = (const Font& val)
{
	*internal = *val.internal;
	return *this;
}
XM_Excel::Font::~Font()
{
	SAFE_DELETE(internal);
}

//设置字体大小
bool XM_Excel::Font::setSize(uint32_t size)
{
	if (size < 0)
	{
		return false;
	}
	//需要将输入size扩大20倍才能正常显示
	internal->font_size = size*20;
	return true;
}

//字体加粗
bool XM_Excel::Font::setBold()
{
	internal->font_bold = true;
	return true;
}

//字体下划线
bool XM_Excel::Font::setUnderline()
{
	internal->font_underline = true;
	return true;
}

bool XM_Excel::Font::setColor(const Color& color)
{
	internal->font_color = new Color(color);
	return true;
}
bool XM_Excel::Font::setColor(const shared_ptr<Color>& color)
{
	if (color == NULL)
	{
		return false;
	}

	return setColor(*color.get());
}

}
}