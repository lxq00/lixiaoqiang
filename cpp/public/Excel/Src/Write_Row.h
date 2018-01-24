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

	//--------------�������		
	bool font(const shared_ptr<XM_Excel::Font>& font);

	//--------------�����ɫ
	bool fill(const XM_Excel::Color& color);
	bool fill(const shared_ptr<XM_Excel::Color>& color);
	//--------------��ʾ��ʽ
	bool format(const XM_Excel::Format& _fmt);
	bool format(const shared_ptr<XM_Excel::Format>& _fmt);
	//--------------��Ԫ��߿�
	bool side(const XM_Excel::Side& _sd);
	bool side(const shared_ptr<XM_Excel::Side>& _sd);
	///�������ݣ�����
	shared_ptr<XM_Excel::Cell> setData(uint32_t colNum, const shared_ptr<XM_Excel::Value>& val);
	shared_ptr<XM_Excel::Cell> setData(uint32_t colNum, const XM_Excel::Value& val);
	//--------------���򣬵��е�ָ���кϲ�, ��ʼ���кţ��������к�
	shared_ptr<XM_Excel::Range> range(uint32_t startColNum, uint32_t stopColNum);

	//--------------��ʾ������,���е�������ʾ
	bool show(bool showflag);
	//--------------��ǰ����������
	uint32_t maxColNum();
	//--------------��ǰ�е��к�
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