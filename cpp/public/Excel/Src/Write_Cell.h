#ifndef __WRITE_CELL_H__
#define __WRITE_CELL_H__
#include "Excel/Excel.h"
#include "xlslib.h"

using namespace xlslib_core;

namespace Public {
namespace Excel {


class Write_Cell :public XM_Excel::Cell
{
public:
	Write_Cell(const shared_ptr<XM_Excel::Sheet>& _worksheet, uint32_t m_rowNum, uint32_t m_colNum, xlslib_core::cell_t* m_cell);
	~Write_Cell();

	///�к�
	unsigned int rowNum();
	///�к�
	unsigned int colNum();

	//--------------�������		
	bool font(const shared_ptr<XM_Excel::Font>& _font);

	//--------------�����ɫ
	bool fill(const XM_Excel::Color& _color);
	bool fill(const shared_ptr<XM_Excel::Color>& _color);

	//--------------��ʾ��ʽ
	bool format(const XM_Excel::Format& _fmt);
	bool format(const shared_ptr<XM_Excel::Format>& _fmt);

	//--------------��Ԫ��߿�
	bool side(const XM_Excel::Side& _sd);
	bool side(const shared_ptr<XM_Excel::Side>& _sd);
public:
	xlslib_core::cell_t*		m_cell;
private:
	shared_ptr<XM_Excel::Sheet> worksheet;
	
	uint32_t m_rowNum;
	uint32_t m_colNum;
};


}
}


#endif //__WRITE_CELL_H__