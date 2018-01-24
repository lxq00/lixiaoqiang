#ifndef __WRITE_ROW_H__
#define __WRITE_ROW_H__
#include "Excel/Excel.h"
#include "xlslib.h"

namespace Public {
namespace Excel {

class Write_Row :public XM_Excel::Row
{
public:
	Write_Row(const shared_ptr<XM_Excel::Sheet>& _worksheet, xlslib_core::worksheet* writesheet, uint32_t m_rowNum);
	~Write_Row();

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
	shared_ptr<XM_Excel::Cell> setData(uint32_t colNum, const shared_ptr<XM_Excel::Value>& val);
	shared_ptr<XM_Excel::Cell> setData(uint32_t colNum, const XM_Excel::Value& val);
	//--------------区域，单行的指定行合并, 开始的行号，结束的行号
	shared_ptr<XM_Excel::Range> range(uint32_t startColNum, uint32_t stopColNum);

	//--------------显示，隐藏,整行的隐藏显示
	bool show(bool showflag);
	//--------------当前行最大的列数
	uint32_t maxColNum();
	//--------------当前行的行号
	uint32_t rowNum();

	uint32_t height();
	bool setHeight(uint32_t height);
private:
	shared_ptr<XM_Excel::Sheet> worksheet;
	xlslib_core::worksheet* writesheet;
	uint32_t m_rowNum;
	uint32_t m_height;
};

}
}

#endif //__WRITE_ROW_H__