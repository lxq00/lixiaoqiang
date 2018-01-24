#ifndef __WRITE_WORKBOOK_H__
#define __WRITE_WORKBOOK_H__

#include "Excel/Excel.h"
#include "xlslib/workbook.h"


using namespace xlslib_core;
namespace Xunmei {
namespace Excel {
		

class Write_WorkBook :public XM_Excel::WorkBook, public enable_shared_from_this<XM_Excel::WorkBook>
{
public:
	Write_WorkBook(const shared_ptr<xlslib_core::workbook>& _pWorkbook, const std::string& xlsfile);
	~Write_WorkBook();
	//����һ��xls�ļ�
	static shared_ptr<XM_Excel::WorkBook> create(const std::string& xlsfile);
	//����һ��sheet
	shared_ptr<XM_Excel::Sheet> addSheet(const std::string& name);

	uint8_t getColorIndex(const XM_Excel::Color& color);
public:
	const std::string xlsfile;
	shared_ptr<xlslib_core::workbook> pWorkbook;

	unsigned8_t			colorindex;
	std::map<std::string, uint8_t>  colormap;
};

}
}


#endif //__WRITE_WORKBOOK_H__