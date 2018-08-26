#ifndef __WRITE_WORKBOOK_H__
#define __WRITE_WORKBOOK_H__

#include "Excel/Excel.h"
#include "xlslib/workbook.h"


using namespace xlslib_core;
namespace Public {
namespace Excel {
		

class Write_WorkBook :public WorkBook, public enable_shared_from_this<WorkBook>
{
public:
	Write_WorkBook(const shared_ptr<xlslib_core::workbook>& _pWorkbook, const std::string& xlsfile);
	~Write_WorkBook();
	//创建一个xls文件
	static shared_ptr<WorkBook> create(const std::string& xlsfile);
	//创建一个sheet
	shared_ptr<WorkBook::Sheet> addSheet(const std::string& name);

	uint8_t getColorIndex(const WorkBook::Color& color);
public:
	const std::string xlsfile;
	shared_ptr<xlslib_core::workbook> pWorkbook;

	unsigned8_t			colorindex;
	std::map<std::string, uint8_t>  colormap;
};

}
}


#endif //__WRITE_WORKBOOK_H__