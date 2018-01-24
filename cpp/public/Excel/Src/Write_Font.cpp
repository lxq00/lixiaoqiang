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

//���������С
bool XM_Excel::Font::setSize(uint32_t size)
{
	if (size < 0)
	{
		return false;
	}
	//��Ҫ������size����20������������ʾ
	internal->font_size = size*20;
	return true;
}

//����Ӵ�
bool XM_Excel::Font::setBold()
{
	internal->font_bold = true;
	return true;
}

//�����»���
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