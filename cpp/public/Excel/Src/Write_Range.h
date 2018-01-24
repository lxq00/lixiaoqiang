#ifndef __WRITE_RANGE_H__
#define __WRITE_RANGE_H__
#include "Excel/Excel.h"
#include "xlslib.h"

namespace Public {
namespace Excel {
	
///excel�ж��ж���ѡ������
class Write_Range :public XM_Excel::Range
{
public:
	Write_Range(const shared_ptr<XM_Excel::Sheet>& _worksheet, uint32_t _startRowNum, uint32_t _startColNum, uint32_t _stopRowNum, uint32_t _stopColNum, xlslib_core::range* _range);
	~Write_Range();

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

	////��ȡĳһ��
	//shared_ptr<XM_Excel::Row> row(uint32_t rowNum);
	////--------------��ʾ������,�������������ʾ
	void hidden(bool hidden_opt);

	//--------------�ϲ�,ָ����Ԫ��
	bool merge();

public:
	shared_ptr<XM_Excel::Sheet> worksheet;
	xlslib_core::range* _range;

	uint32_t startRowNum;
	uint32_t startColNum;
	uint32_t stopRowNum;
	uint32_t stopColNum;
};

}
}

#endif //__WRITE_RANGE_H__