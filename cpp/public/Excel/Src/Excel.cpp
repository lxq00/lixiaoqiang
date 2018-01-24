#include "Excel/Excel.h"
#include "Read_Workbook.h"
#include "Write_WorkBook.h"

namespace Xunmei {
namespace Excel {

shared_ptr<XM_Excel::WorkBook> XM_Excel::read(const std::string& xlsfile)
{
	return Read_WorkBook::read(xlsfile);
}

shared_ptr<XM_Excel::WorkBook> XM_Excel::create(const std::string& xlsfile)
{
	return Write_WorkBook::create(xlsfile);
}


}
}

