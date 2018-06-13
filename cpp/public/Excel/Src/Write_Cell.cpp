#include "Write_Cell.h"
#include "Write_Font.h"
#include "Write_Format.h"
#include "Write_Side.h"
#include "Write_WorkBook.h"
#include "Write_Sheet.h"

using namespace Public;
using namespace Excel;

Write_Cell::Write_Cell(const shared_ptr<WorkBook::Sheet>& _worksheet, uint32_t _rowNum, uint32_t _colNum, xlslib_core::cell_t* _cell)
	: m_cell(_cell),worksheet(_worksheet), m_rowNum(_rowNum), m_colNum(_colNum)
{
}
Write_Cell::~Write_Cell()
{
}
///行号
unsigned int Write_Cell::rowNum()
{
	return m_rowNum;
}
///列号
unsigned int Write_Cell::colNum()
{
	return m_colNum;
}

bool Write_Cell::font(const shared_ptr<WorkBook::Font>& font)
{
	if (font == NULL || m_cell == NULL)
	{
		return false;
	}

	//cell要设置属性，需要workbok对象，但他本事没有，找对象

	//this de father
	Write_Sheet* sheet = (Write_Sheet*)worksheet.get();
	//sheet de father
	Write_WorkBook* wb = (Write_WorkBook*)sheet->workbook.get();	
	shared_ptr<workbook> excelwb = wb->pWorkbook;

	font_t* _font = excelwb->font(font->internal->font_name);
	if (_font == NULL)
	{
		return false;
	}

	//需要设置才设置
	if (font->internal->font_size != 0)
	{
		_font->SetHeight(font->internal->font_size);
	}
	if (font->internal->font_bold)
	{
		_font->SetBoldStyle(BOLDNESS_BOLD);
	}
	if (font->internal->font_underline)
	{
		_font->SetUnderlineStyle(UNDERLINE_SINGLEACC);
	}
	if (font->internal->font_color != NULL)
	{
		uint8_t colorindex = wb->getColorIndex(*font->internal->font_color.get());
		_font->SetColor(colorindex);
	}
	m_cell->font(_font);

	return true;
}
//--------------填充颜色
bool Write_Cell::fill(const WorkBook::Color& _color)
{
	if (m_cell == NULL)
	{
		return false;
	}

	//this de father
	Write_Sheet* sheet = (Write_Sheet*)worksheet.get();
	//sheet de father
	Write_WorkBook* wb = (Write_WorkBook*)sheet->workbook.get();

	uint8_t colorindex = wb->getColorIndex(_color);
	m_cell->fillfgcolor(colorindex);
	m_cell->fillstyle(FILL_SOLID);

	return true; 
}
bool Write_Cell::fill(const shared_ptr<WorkBook::Color>& _color)
{ 
	if (_color == NULL)
	{
		return false;
	}

	return fill(*_color.get());
}

//--------------显示格式
bool Write_Cell::format(const WorkBook::Format& _fmt)
{ 
	//this de father
	Write_Sheet* sheet = (Write_Sheet*)worksheet.get();
	//sheet de father
	Write_WorkBook* wb = (Write_WorkBook*)sheet->workbook.get();
	shared_ptr<workbook> excelwb = wb->pWorkbook;

	xf_t* _format = excelwb->xformat();

	_format->SetHAlign((halign_option_t)_fmt.internal->xalign);
	_format->SetVAlign((valign_option_t)_fmt.internal->yalign);	
	_format->SetTxtOrientation((txtori_option_t)_fmt.internal->orient);
	_format->SetFormat((format_number_t)_fmt.internal->fomat);
	
	m_cell->SetXF(_format);

	return true;
}
bool Write_Cell::format(const shared_ptr<WorkBook::Format>& _fmt)
{ 
	if (_fmt == NULL)
	{
		return false;
	}

	return format(*_fmt.get());
}

//--------------单元格边框
bool Write_Cell::side(const WorkBook::Side& _sd)
{ 
	if (m_cell == NULL)
	{
		return false;
	}
	//this de father
	Write_Sheet* sheet = (Write_Sheet*)worksheet.get();
	//sheet de father
	Write_WorkBook* wb = (Write_WorkBook*)sheet->workbook.get();

	if (_sd.internal->side & SIDE_BOTTOM)
	{
		m_cell->borderstyle(BORDER_BOTTOM, BORDER_THIN);
		if (_sd.internal->color != NULL)
		{
			uint8_t colorindex = wb->getColorIndex(*_sd.internal->color.get());
			m_cell->bordercolor(BORDER_BOTTOM, colorindex);
		}
	}
	if (_sd.internal->side & SIDE_TOP)
	{
		m_cell->borderstyle(BORDER_TOP, BORDER_THIN);
		if (_sd.internal->color != NULL)
		{
			uint8_t colorindex = wb->getColorIndex(*_sd.internal->color.get());
			m_cell->bordercolor(BORDER_TOP, colorindex);
		}
	}
	if (_sd.internal->side & SIDE_LEFT)
	{
		m_cell->borderstyle(BORDER_LEFT, BORDER_THIN);
		if (_sd.internal->color != NULL)
		{
			uint8_t colorindex = wb->getColorIndex(*_sd.internal->color.get());
			m_cell->bordercolor(BORDER_LEFT, colorindex);
		}
	}
	if (_sd.internal->side & SIDE_RIGHT)
	{
		m_cell->borderstyle(BORDER_RIGHT, BORDER_THIN);
		if (_sd.internal->color != NULL)
		{
			uint8_t colorindex = wb->getColorIndex(*_sd.internal->color.get());
			m_cell->bordercolor(BORDER_RIGHT, colorindex);
		}
	}

	return true;
}
bool Write_Cell::side(const shared_ptr<WorkBook::Side>& _sd)
{	
	if (_sd == NULL)
	{
		return false;
	}

	return side(*_sd.get());
}
