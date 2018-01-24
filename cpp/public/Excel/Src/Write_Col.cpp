#include "Write_Col.h"
#include "Write_Sheet.h"

using namespace Xunmei;
using namespace Excel;

Write_Col::Write_Col(const shared_ptr<XM_Excel::Sheet>& _worksheet, xlslib_core::worksheet* _writesheet, uint32_t _colNum)
	: worksheet(_worksheet), writesheet(_writesheet), m_colNum(_colNum)
{

}
Write_Col::~Write_Col()
{

}
//设置数据，内容
shared_ptr<XM_Excel::Cell> Write_Col::setData(uint32_t rowNum, const shared_ptr<XM_Excel::Value>& val)
{
	if (val == NULL)
		return shared_ptr<XM_Excel::Cell>();
	return Write_Col::setData(rowNum, *val.get());
}

shared_ptr<XM_Excel::Cell> Write_Col::setData(uint32_t rowNum, const XM_Excel::Value& val)
{
	if (worksheet == NULL)
		return shared_ptr<XM_Excel::Cell>();
	return worksheet->setData(rowNum, m_colNum, val);
}
//--------------字体相关		
bool Write_Col::font(const shared_ptr<XM_Excel::Font>& font)
{
	if (worksheet == NULL)
	{
		return false;
	}
	return worksheet->font(font);
}
//--------------填充颜色
bool Write_Col::fill(const XM_Excel::Color& color)
{
	if (worksheet == NULL)
	{
		return false;
	}
	return worksheet->fill(color);
}
bool Write_Col::fill(const shared_ptr<XM_Excel::Color>& color)
{
	return fill(*color.get());
}
//--------------显示格式
bool Write_Col::format(const XM_Excel::Format& _fmt)
{
	if (worksheet == NULL)
	{
		return false;
	}
	return worksheet->format(_fmt);
}
bool Write_Col::format(const shared_ptr<XM_Excel::Format>& _fmt)
{
	return format(*_fmt.get());
}
//--------------单元格边框
bool Write_Col::side(const XM_Excel::Side& _sd)
{
	if (worksheet == NULL)
	{
		return false;
	}
	shared_ptr<XM_Excel::Range> ran = range(0, maxRowNum());
	if (ran == NULL)
	{
		return false;
	}
	
	return ran->side(_sd);
}
bool Write_Col::side(const shared_ptr<XM_Excel::Side>& _sd)
{
	return side(*_sd.get());
}

//--------------区域，单行的指定行合并, 开始的行号，结束的行号
shared_ptr<XM_Excel::Range>  Write_Col::range(uint32_t startRowNum, uint32_t stopRowNum)
{ 
	return worksheet->range(startRowNum, m_colNum, stopRowNum, m_colNum);
}

//--------------显示，隐藏,整行的隐藏显示
bool  Write_Col::show(bool showflag)
{
	shared_ptr<XM_Excel::Col> c_col = worksheet->col(m_colNum);
	shared_ptr<xlslib_core::range> c_range = worksheet->range(m_colNum, 0, m_colNum, c_col->maxRowNum());
	c_range->hidden(showflag);

	return true;
}

//--------------当前列的列号
uint32_t Write_Col::colNum()
{
	return m_colNum;
}
//--------------当前列的最大的行数
uint32_t Write_Col::maxRowNum()
{
	Write_Sheet* _writesheet = (Write_Sheet*)worksheet.get();

	return _writesheet->getColMaxRowNum(m_colNum);
}
//列宽
//uint32_t Write_Col::width()
//{ 
//	return m_width;
//}
//bool Write_Col::setWidth(uint32_t width)
//{ 
//	if (width < 0)
//	{
//		return false;
//	}
//	m_width = width;
//	writesheet->label(m_colNum, 1, " ");
//	writesheet->colwidth(m_colNum, m_width);
//	return true;
//}
