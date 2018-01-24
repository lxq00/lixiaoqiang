#ifndef __WRITE_COL_H__
#define __WRITE_COL_H__
#include "Excel/Excel.h"
#include "xlslib.h"


namespace Public {
namespace Excel {
class Write_Col :public XM_Excel::Col
{
public:
	Write_Col(const shared_ptr<XM_Excel::Sheet>& _worksheet, xlslib_core::worksheet* writesheet, uint32_t m_colNum);
	~Write_Col();
	//--------------字体相关		
	bool font(const shared_ptr<XM_Excel::Font>& font);

	//--------------填充颜色
	bool fill(const XM_Excel::Color& color);
	bool fill(const shared_ptr<XM_Excel::Color>& color);
	//--------------显示格式
	bool format(const XM_Excel::Format& _fmt);
	bool format(const shared_ptr<XM_Excel::Format>& _fmt);
	//--------------单元格边框
	bool side(const XM_Excel::Side& _sd);
	bool side(const shared_ptr<XM_Excel::Side>& _sd);
	///设置数据，内容
	shared_ptr<XM_Excel::Cell> setData(uint32_t rowNum, const shared_ptr<XM_Excel::Value>& val);
	shared_ptr<XM_Excel::Cell> setData(uint32_t rowNum, const XM_Excel::Value& val);
	//--------------区域，单行的指定行合并, 开始的行号，结束的行号
	shared_ptr<XM_Excel::Range> range(uint32_t startRowNum, uint32_t stopRowNum);

	//--------------显示，隐藏,整行的隐藏显示
	bool show(bool showflag);
	//--------------当前列的列号
	uint32_t colNum();
	//--------------当前列的最大的行数
	uint32_t maxRowNum();

	//列宽
	//uint32_t width();
	//bool setWidth(uint32_t height);
private:
	shared_ptr<XM_Excel::Sheet> worksheet;
	xlslib_core::worksheet* writesheet;
	uint32_t m_colNum;
	uint32_t m_width;
};

}
}

#endif //__WRITE_COL_H__