
#include "Read_Cell.h"

namespace Public {
namespace Excel {

	Read_Cell::Read_Cell(const shared_ptr<WorkBook::Sheet>& _sheet, uint32_t _rowNum, uint32_t _colNum)
		: sheet(_sheet),m_rowNum(_rowNum), m_colNum(_colNum)
	{

	}
	Read_Cell::~Read_Cell()
	{
	}
	///获取数据,内容
	shared_ptr<WorkBook::Value> Read_Cell::data() const
	{
		return sheet->data(m_rowNum, m_colNum);
	}

	///行号
	unsigned int Read_Cell::rowNum()
	{
		return m_rowNum;
	}
	///列号
	unsigned int Read_Cell::colNum() 
	{
		return m_colNum;
	}

}
}
