#ifndef __WRITE_SHEET_H__
#define __WRITE_SHEET_H__
#include "Excel/Excel.h"
#include "xlslib.h"

namespace Xunmei {
namespace Excel {

class  Write_Sheet :public  XM_Excel::Sheet, public enable_shared_from_this<XM_Excel::Sheet>
{
public:
	static wstring c2w(const string& str);
public:
	Write_Sheet(const shared_ptr<XM_Excel::WorkBook>& _workbook, xlslib_core::worksheet* _writesheet, const std::string& _sheetname);
	~Write_Sheet();

	//��ȡĳһ��
	shared_ptr<XM_Excel::Row> row(uint32_t rowNum);
	//��ȡĳһ��
	shared_ptr<XM_Excel::Col> col(uint32_t colNum);
	std::string name();

	///�������ݣ�����
	shared_ptr<XM_Excel::Cell> setData(uint32_t rowNum, uint32_t colNum, const shared_ptr<XM_Excel::Value>& val) /*{ return shared_ptr<XM_Excel::Cell>(); }*/;
	shared_ptr<XM_Excel::Cell> setData(uint32_t rowNum, uint32_t colNum, const XM_Excel::Value& val) /*{ return shared_ptr<XM_Excel::Cell>(); }*/;


	
	//--------------��������
	uint32_t maxRowNum();
	//--------------��������
	uint32_t maxColNum();
	
	//��ȡĳһ����ĳһ�еı�� 
	shared_ptr<XM_Excel::Cell> cell(uint32_t rowNum, uint32_t colNum);
	uint32_t getRowMaxColNum(uint32_t rownum);
	uint32_t getColMaxRowNum(uint32_t colnum);

	bool font(const shared_ptr<XM_Excel::Font>& font, const std::string& name);

	//--------------�����ɫ
	bool fill(const XM_Excel::Color& color);
	//bool fill(uint8_t _r, uint8_t _g, uint8_t _b);
	bool fill(const shared_ptr<XM_Excel::Color>& color);

	//--------------��ʾ��ʽ
	bool format(const XM_Excel::Format& fmt);
	bool format(const shared_ptr<XM_Excel::Format>& fmt);

	//--------------�ϲ�
	shared_ptr<XM_Excel::Range> range(uint32_t startRowNum, uint32_t startColNum, uint32_t stopRowNum, uint32_t stopColNum);

	//--------------Ĭ�Ͽ���
	bool defaultRowHeight(uint16_t height); // sets column widths to 1/256 x width of "0"
	bool defaultColwidth(uint16_t width);  // in points (Excel uses twips, 1/20th of a point, but xlslib didn't)

public:
	uint8_t s_r;
	uint8_t s_g;
	uint8_t s_b;
	uint16_t m_height;
	uint16_t m_width;

	std::string	                   sheetname;
	xlslib_core::worksheet* writesheet;
	shared_ptr<XM_Excel::WorkBook> workbook;

};

}
}

#endif //__WRITE_SHEET_H__
