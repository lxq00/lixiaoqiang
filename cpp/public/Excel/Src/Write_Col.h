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
	shared_ptr<XM_Excel::Cell> setData(uint32_t rowNum, const shared_ptr<XM_Excel::Value>& val);
	shared_ptr<XM_Excel::Cell> setData(uint32_t rowNum, const XM_Excel::Value& val);
	//--------------���򣬵��е�ָ���кϲ�, ��ʼ���кţ��������к�
	shared_ptr<XM_Excel::Range> range(uint32_t startRowNum, uint32_t stopRowNum);

	//--------------��ʾ������,���е�������ʾ
	bool show(bool showflag);
	//--------------��ǰ�е��к�
	uint32_t colNum();
	//--------------��ǰ�е���������
	uint32_t maxRowNum();

	//�п�
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