#include "Read_Sheet.h"
#include "Read_Col.h"

namespace Public {
namespace Excel {

Read_Col::Read_Col(const shared_ptr<WorkBook::Sheet>& _sheet, uint32_t _colNum)
	: sheet(_sheet), m_colNum(_colNum)
{

}
Read_Col::~Read_Col()
{

}
//获取某一列中某一行的表格 
shared_ptr<WorkBook::Cell> Read_Col::cell(uint32_t rowNum)
{
	return sheet->setData(rowNum, m_colNum, sheet->data(rowNum, m_colNum));
}

//--------------数据相关
///获取数据,内容
shared_ptr<WorkBook::Value> Read_Col::data(uint32_t rowNum) const
{
	return sheet->data(rowNum, m_colNum);
}
//--------------当前列的最大的列号
uint32_t Read_Col::colNum()
{
	return m_colNum;
}

//--------------当前列的最大的行数
uint32_t Read_Col::maxRowNum()
{
	Read_Sheet* readsheet = (Read_Sheet*)sheet.get();

	return readsheet->getColMaxRowNum(m_colNum);



}
}
}