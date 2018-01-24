#include "Write_Sheet.h"
#include "Write_Row.h"
#include "Write_Col.h"
#include "Write_Font.h"
#include "Write_Cell.h"
#include "Write_Range.h"
#include "Write_WorkBook.h"

using namespace Public;
using namespace Excel;


wstring Write_Sheet::c2w(const string& str)
{
	wstring wstr;
	{
		uint32_t maxlen = str.length() + 100;

		wchar_t* ptr = new wchar_t[maxlen];

		if (setlocale(LC_CTYPE, "chs") == NULL)
		{
			setlocale(LC_CTYPE, "");
		}
		int len = mbstowcs(ptr, str.c_str(), maxlen);

		wstr = wstring(ptr, len);
		SAFE_DELETEARRAY(ptr);
	
	}

	return wstr;
}

Write_Sheet::Write_Sheet(const shared_ptr<XM_Excel::WorkBook>& _workbook, xlslib_core::worksheet* _writesheet, const std::string& _sheetname)
	:sheetname(_sheetname), writesheet(_writesheet), workbook(_workbook)
{

}
Write_Sheet::~Write_Sheet()
{

}

std::string Write_Sheet::name()
{
	 return sheetname; 
}
///�������ݣ�����
shared_ptr<XM_Excel::Cell> Write_Sheet::setData(uint32_t rowNum, uint32_t colNum, const shared_ptr<XM_Excel::Value>& val)
{
	if (val == NULL) return shared_ptr<XM_Excel::Cell>();

	return Write_Sheet::setData(rowNum,colNum,*val.get());
}

shared_ptr<XM_Excel::Cell> Write_Sheet::setData(uint32_t rowNum, uint32_t colNum, const XM_Excel::Value& val)
{
	//cell_t* m_cell;
	if (writesheet == NULL)
	{
		return shared_ptr<XM_Excel::Cell>();
	}

	cell_t* m_cell = NULL;
	if (val.type() == XM_Excel::Value::Type_Bool)
	{
		m_cell = writesheet->boolean(rowNum, colNum, val.toBool());
	}
	else if (val.type() == XM_Excel::Value::Type_Number)
	{
		m_cell = writesheet->number(rowNum, colNum, val.toInt32());
	}
	else if (val.type() == XM_Excel::Value::Type_Double)
	{
		m_cell = writesheet->number(rowNum, colNum, val.toDouble());
	}
	else if (val.type() == XM_Excel::Value::Type_Double)
	{
		m_cell = writesheet->number(rowNum, colNum, val.toFloat());
	}
	else if (val.type() == XM_Excel::Value::Type_Number)
	{
		m_cell = writesheet->number(rowNum, colNum, val.toUint32());
	}
	else if (val.type() == XM_Excel::Value::Type_String)
	{
		m_cell = writesheet->label(rowNum, colNum, c2w(val.toString()));
	}
	return shared_ptr<XM_Excel::Cell>(new Write_Cell(shared_from_this(), rowNum, colNum, m_cell));
}

shared_ptr<XM_Excel::Cell> Write_Sheet::cell(uint32_t rowNum, uint32_t colNum)
{
	cell_t* m_cell = writesheet->label(rowNum, colNum, (workbook->addSheet(sheetname)->data(rowNum, colNum))->toString());
	if (writesheet == NULL)
	{
		return shared_ptr<XM_Excel::Cell>();
	}

	return shared_ptr<XM_Excel::Cell>(new Write_Cell(shared_from_this(), rowNum, colNum, m_cell));
}

bool Write_Sheet::font(const shared_ptr<XM_Excel::Font>& font, const std::string& name)
{ 
	if (&font == NULL || name == "")
	{
		return false;
	}
	//shared_ptr<XM_Excel::Font> font(new Write_Font(name));
	font->create(name);

	return true;
}
//--------------�����ɫ
bool Write_Sheet::fill(const XM_Excel::Color& color)
{ 
	if (&color == NULL)
	{
		return false;
	}
	XM_Excel::Color _color = color;
	return true;
}
bool Write_Sheet::fill(const shared_ptr<XM_Excel::Color>& color)
{ 
	fill(*color.get());
	return true; 
}

//--------------��ʾ��ʽ
bool Write_Sheet::format(const XM_Excel::Format& fmt)
{ 
	if (&fmt == NULL)
	{
		return false;
	}
	XM_Excel::Format _fmt = fmt;
	return true;
}
bool Write_Sheet::format(const shared_ptr<XM_Excel::Format>& fmt)
{ 
	format(*fmt.get());
	return true;
}
//--------------�ϲ�
shared_ptr<XM_Excel::Range> Write_Sheet::range(uint32_t startRowNum, uint32_t startColNum, uint32_t stopRowNum, uint32_t stopColNum)
{ 
	xlslib_core::range* _range;	//�������������
	_range = writesheet->rangegroup(startRowNum, startColNum, stopRowNum, stopColNum);
	if (_range == NULL)
	{
		return shared_ptr<XM_Excel::Range>();
	}

	return shared_ptr<XM_Excel::Range>(new Write_Range(shared_from_this(), startRowNum, startColNum, stopRowNum, stopColNum, _range));
}

//--------------Ĭ�Ͽ���
bool Write_Sheet::defaultRowHeight(uint16_t height)
{
	if (height < 0)
	{
		return false;
	}
	m_height = height;
	writesheet->defaultRowHeight(height);
	return true;
}

bool Write_Sheet::defaultColwidth(uint16_t width)
{
	if (width < 0)
	{
		return false;
	}
	m_width = width;
	writesheet->defaultColwidth(width);
	return true;
}

//--------------��������
uint32_t Write_Sheet::maxRowNum()
{ 
	if (writesheet == NULL)
	{
		return 0;
	}
	//ʵ��excel������Ϊlastrow+1
	return writesheet->maxRow;
}
//--------------��������
uint32_t Write_Sheet::maxColNum()
{ 
	if (writesheet == NULL)
	{
		return 0;
	}
	//ʵ��excel������Ϊlastrow+1
	return writesheet->maxCol;
}
//��ȡĳһ��
shared_ptr<XM_Excel::Row> Write_Sheet::row(uint32_t rowNum) 
{
	if (writesheet == NULL)
	{
		return shared_ptr<XM_Excel::Row>();
	}
	if (rowNum > writesheet->maxRow)
	{
		return shared_ptr<XM_Excel::Row>();
	}
	//todo ��ǰֻ������str��������Ҫ����ʵ�ʵ����ʹ���value
	return shared_ptr<XM_Excel::Row>(new Write_Row(shared_from_this(), writesheet, rowNum));
}
//��ȡĳһ��
shared_ptr<XM_Excel::Col> Write_Sheet::col(uint32_t colNum) 
{
	if (writesheet == NULL)
	{
		return shared_ptr<XM_Excel::Col>();
	}
	if (colNum > writesheet->maxCol)
	{
		return shared_ptr<XM_Excel::Col>();
	}
	//todo ��ǰֻ������str��������Ҫ����ʵ�ʵ����ʹ���value
	return shared_ptr<XM_Excel::Col>(new Write_Col(shared_from_this(), writesheet, colNum));
}
uint32_t Write_Sheet::getRowMaxColNum(uint32_t rownum)
{
	if (writesheet == NULL)
	{
		return 0;
	}
	if (rownum >  writesheet->maxCol)
	{
		return 0;
	}
	return writesheet->NumCells();
}

uint32_t Write_Sheet::getColMaxRowNum(uint32_t colnum)
{
	if (writesheet == NULL)
	{
		return 0;
	}
	return  writesheet->minCol;
}


