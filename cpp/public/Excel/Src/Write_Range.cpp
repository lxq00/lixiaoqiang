#include "Write_Range.h"
#include "Write_Sheet.h"
#include "Write_Cell.h"
#include "Write_Color.h"
#include "Write_Format.h"
#include "Write_Side.h"
#include "Write_Font.h"
#include "Write_WorkBook.h"


using namespace Public;
using namespace Excel;



Write_Range::Write_Range(const shared_ptr<WorkBook::Sheet>& _worksheet, uint32_t _startRowNum, uint32_t _startColNum, uint32_t _stopRowNum, uint32_t _stopColNum, xlslib_core::range* __range)
	:  worksheet(_worksheet), _range(__range), startRowNum(_startRowNum), startColNum(_startColNum), stopRowNum(_stopRowNum), stopColNum(_stopColNum)
{
}
Write_Range::~Write_Range() 
{
}

//--------------字体相关		
bool Write_Range::font(const shared_ptr<WorkBook::Font>& _font)
{
	if (worksheet == NULL || _range == NULL)
	{
		return false;
	}
	//this de father
	Write_Sheet* sheet = (Write_Sheet*)worksheet.get();
	//sheet de father
	Write_WorkBook* wb = (Write_WorkBook*)sheet->workbook.get();
	_range->fontname(_font->internal->font_name);
	//需要设置才设置
	if (_font->internal->font_size != 0)
	{
		_range->fontheight(_font->internal->font_size);
	}
	if (_font->internal->font_bold)
	{
		_range->fontbold(BOLDNESS_BOLD);
	}
	if (_font->internal->font_underline)
	{
		_range->fontunderline(UNDERLINE_SINGLEACC);
	}
	if (_font->internal->font_color != NULL)
	{
		uint8_t colorindex = wb->getColorIndex(*_font->internal->font_color.get());
		_range->fontcolor(colorindex);
	}
	
	return true;
}

//--------------颜色相关,填充颜色
bool Write_Range::fill(const WorkBook::Color& _color)
{ 
	//this de father
	Write_Sheet* sheet = (Write_Sheet*)worksheet.get();
	//sheet de father
	Write_WorkBook* wb = (Write_WorkBook*)sheet->workbook.get();
	uint8_t colorindex = wb->getColorIndex(_color);
	_range->fillfgcolor(colorindex);
	_range->fillstyle(FILL_SOLID);

	return true;
}
bool Write_Range::fill(const shared_ptr<WorkBook::Color>& _color)
{ 
	fill(*_color.get());
	return true;
}

//--------------显示格式
bool Write_Range::format(const WorkBook::Format& _fmt)
{ 
	_range->format((format_number_t)_fmt.internal->fomat);
	_range->orientation((txtori_option_t)_fmt.internal->orient);
	_range->valign((valign_option_t)_fmt.internal->yalign);
	_range->halign((halign_option_t)_fmt.internal->xalign);
	return true;
}
bool Write_Range::format(const shared_ptr<WorkBook::Format>& _fmt)
{ 
	format(*_fmt.get());
	return true;
}

//--------------单元格边框
bool Write_Range::side(const WorkBook::Side& _sd)
{ 
	//this de father
	Write_Sheet* sheet = (Write_Sheet*)worksheet.get();
	//sheet de father
	Write_WorkBook* wb = (Write_WorkBook*)sheet->workbook.get();
	
	if (_sd.internal->side & SIDE_BOTTOM)
	{
		_range->borderstyle(BORDER_BOTTOM, BORDER_THIN);
		if (_sd.internal->color != NULL)
		{
			uint8_t colorindex = wb->getColorIndex(*_sd.internal->color.get());
			_range->bordercolor(BORDER_BOTTOM, colorindex);
		}
	}
	if (_sd.internal->side & SIDE_TOP)
	{
		_range->borderstyle(BORDER_TOP, BORDER_THIN);
		if (_sd.internal->color != NULL)
		{
			uint8_t colorindex = wb->getColorIndex(*_sd.internal->color.get());
			_range->bordercolor(BORDER_TOP, colorindex);
		}
	}
	if (_sd.internal->side & SIDE_LEFT)
	{
		_range->borderstyle(BORDER_LEFT, BORDER_THIN);
		if (_sd.internal->color != NULL)
		{
			uint8_t colorindex = wb->getColorIndex(*_sd.internal->color.get());
			_range->bordercolor(BORDER_LEFT, colorindex);
		}
	}
	if (_sd.internal->side & SIDE_RIGHT)
	{
		_range->borderstyle(BORDER_RIGHT, BORDER_THIN);
		if (_sd.internal->color != NULL)
		{
			uint8_t colorindex = wb->getColorIndex(*_sd.internal->color.get());
			_range->bordercolor(BORDER_RIGHT, colorindex);
		}
	}
	return true;
}
bool Write_Range::side(const shared_ptr<WorkBook::Side>& _sd)
{ 
	side(*_sd.get());
	return true;
}

//--------------显示，隐藏,整区域的隐藏显示
void Write_Range::hidden(bool hidden_opt)
{
	worksheet->row(startRowNum);

	_range->hidden(hidden_opt);
	return;
}

//--------------合并,指定单元格
bool Write_Range::merge()
{
	//this de father
	Write_Sheet* sheet = (Write_Sheet*)worksheet.get();
	//sheet de father
	xlslib_core::worksheet* sh = sheet->writesheet;
	
	sh->merge(startRowNum, startColNum, stopRowNum, stopColNum);
	
	return	true;
}