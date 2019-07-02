#ifndef __READ_ROW_H__
#define __READ_ROW_H__
#include "Read_Cell.h"
#include "Read_Sheet.h"

namespace Public {
namespace Excel {

class Read_Row :public WorkBook::Row
{
public:
	Read_Row(const shared_ptr<WorkBook::Sheet>& sheet, uint32_t m_rowNum);
	~Read_Row();
	//获取某一行中某一列的表格 
	shared_ptr<WorkBook::Cell> cell(uint32_t colNum);

	//--------------数据相关
	///获取数据,内容
	Value data(uint32_t colNum) const ;
	//--------------当前行最大的列数
	uint32_t maxColNum();

	//--------------当前行的行号
	uint32_t rowNum();
private:
	shared_ptr<WorkBook::Sheet> sheet;
	uint32_t m_rowNum;
};

}
}

#endif //__READ_ROW_H__