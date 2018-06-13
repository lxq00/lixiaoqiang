#include "Read_Workbook.h"

namespace Public {
namespace Excel {
	
shared_ptr<WorkBook> Read_WorkBook::read(const std::string& xlsfile)
{
	xlsWorkBook *pWorkBook = xls_open((char*)xlsfile.c_str(), "UTF-8");
	if (pWorkBook == NULL)
	{
		return shared_ptr<WorkBook>();
	}

	return shared_ptr<WorkBook>(new Read_WorkBook(pWorkBook));
}

Read_WorkBook::Read_WorkBook(xlsWorkBook *_pWorkBook):pWorkBook(_pWorkBook)
{

}
Read_WorkBook::~Read_WorkBook()
{
	if (pWorkBook != NULL)
	{
		xls_close(pWorkBook);
		pWorkBook = NULL;
	}
}

uint32_t Read_WorkBook::sheetCount()
{
	if (pWorkBook == NULL)
	{
		return 0;
	}

	return xls_sheetCount(pWorkBook);
}

shared_ptr<WorkBook::Sheet> Read_WorkBook::getSheet(uint32_t num)
{	
	if (pWorkBook == NULL)
	{
		return shared_ptr<WorkBook::Sheet>();
	}

	if (num >= pWorkBook->sheets.count)
	{
		return shared_ptr<WorkBook::Sheet>();
	}

	xlsWorkSheet *pWorkSheet = xls_getWorkSheet(pWorkBook, num);
	if (pWorkSheet == NULL)
	{
		return shared_ptr<WorkBook::Sheet>();
	}
	xls_parseWorkSheet(pWorkSheet);
	return shared_ptr<WorkBook::Sheet>(new Read_Sheet(shared_from_this(),pWorkSheet, (char*)pWorkBook->sheets.sheet[num].name));
}

}
}
