#ifndef __READCELL_H__
#define __READCELL_H__
#include "Excel/Excel.h"
#include "libxls/xls.h"
namespace Public {
namespace Excel {

	class Read_Cell :public WorkBook::Cell
	{
	public:
		Read_Cell(const shared_ptr<WorkBook::Sheet>& sheet,uint32_t m_rowNum, uint32_t m_colNum);
		~Read_Cell();
		//获取数据,内容
		shared_ptr<WorkBook::Value> data() const ;
		///行号
		unsigned int rowNum();
		///列号
		unsigned int colNum();
	private:
		shared_ptr<WorkBook::Sheet> sheet;
		uint32_t m_rowNum;
		uint32_t m_colNum;

	};
}
}


#endif //__READCELL_H__