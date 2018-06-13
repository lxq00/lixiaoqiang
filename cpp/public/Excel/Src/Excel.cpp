#include "Excel/Excel.h"
#include "Read_Workbook.h"
#include "Write_WorkBook.h"

namespace Public {
namespace Excel {

shared_ptr<WorkBook> WorkBook::read(const std::string& xlsfile)
{
	return Read_WorkBook::read(xlsfile);
}

shared_ptr<WorkBook> WorkBook::create(const std::string& xlsfile)
{
	return Write_WorkBook::create(xlsfile);
}


}
}

