#ifndef __READCELL_H__
#define __READCELL_H__
#include "Excel/Excel.h"
#include "libxls/xls.h"
namespace Xunmei {
namespace Excel {

	class Read_Cell :public XM_Excel::Cell
	{
	public:
		Read_Cell(const shared_ptr<XM_Excel::Sheet>& sheet,uint32_t m_rowNum, uint32_t m_colNum);
		~Read_Cell();
		//获取数据,内容
		shared_ptr<XM_Excel::Value> data() const ;
		///行号
		unsigned int rowNum();
		///列号
		unsigned int colNum();
	private:
		shared_ptr<XM_Excel::Sheet> sheet;
		uint32_t m_rowNum;
		uint32_t m_colNum;

	};
}
}


#endif //__READCELL_H__