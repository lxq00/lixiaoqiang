#include "Read_Row.h"
#include "Read_Sheet.h"
namespace Public {
namespace Excel {

Read_Row::Read_Row(const shared_ptr<XM_Excel::Sheet>& _sheet,uint32_t _rowNum)
	: sheet(_sheet), m_rowNum(_rowNum)
{

}
Read_Row::~Read_Row()
{

}
//获取某一行中某一列的表格 
shared_ptr<Read_Cell> Read_Row::cell(uint32_t colNum) 
{ 

	return sheet->setData(m_rowNum, colNum, sheet->data(m_rowNum, colNum));
}

//--------------数据相关
///获取数据,内容
shared_ptr<XM_Excel::Value> Read_Row::data(uint32_t colNum) const 
{
	return sheet->data(m_rowNum, colNum);
}
//--------------当前行最大的列数
uint32_t Read_Row::maxColNum()
{
	Read_Sheet* readsheet = (Read_Sheet*)sheet.get();

	return readsheet->getRowMaxColNum(m_rowNum);
}

//--------------当前行的行号
uint32_t Read_Row::rowNum()
{
	return m_rowNum;
}

}
}