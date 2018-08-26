#ifndef __READ_WORKBOOK_H__
#define __READ_WORKBOOK_H__

#include "Excel/Excel.h"
#include "Read_Sheet.h"

namespace Public {
namespace Excel {

class Read_WorkBook :public WorkBook,public enable_shared_from_this<WorkBook>
{
public:
	static shared_ptr<WorkBook> read(const std::string& xlsfile);

	Read_WorkBook(xlsWorkBook *pWorkBook);
	virtual ~Read_WorkBook();
	virtual shared_ptr<WorkBook::Sheet> getSheet(uint32_t num);
	virtual uint32_t sheetCount();
private:
	xlsWorkBook *pWorkBook;
	
};

}
}


#endif //__READ_WORKBOOK_H__