#include "Write_Font.h"

namespace Public {
namespace Excel {

shared_ptr<WorkBook::Font> WorkBook::Font::create(const std::string& name)
{
	if (name == "" || !FontInternal::IsASCII(name))
	{
		return shared_ptr<WorkBook::Font>();
	}
	shared_ptr<WorkBook::Font> Ffont(new WorkBook::Font);
	Ffont->internal->font_name = name;
	return Ffont;
}
WorkBook::Font::Font()
{
	internal = new FontInternal;
}
WorkBook::Font::Font(const Font& font)
{
	internal = new FontInternal;
	*internal = *font.internal;
}
WorkBook::Font& WorkBook::Font::operator = (const Font& val)
{
	*internal = *val.internal;
	return *this;
}
WorkBook::Font::~Font()
{
	SAFE_DELETE(internal);
}

//设置字体大小
bool WorkBook::Font::setSize(uint32_t size)
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
bool WorkBook::Font::setBold()
{
	internal->font_bold = true;
	return true;
}

//字体下划线
bool WorkBook::Font::setUnderline()
{
	internal->font_underline = true;
	return true;
}

bool WorkBook::Font::setColor(const Color& color)
{
	internal->font_color = make_shared<Color>(color);
	return true;
}
bool WorkBook::Font::setColor(const shared_ptr<Color>& color)
{
	if (color == NULL)
	{
		return false;
	}

	return setColor(*color.get());
}

}
}