#ifndef __READ_COL_H__
#define __READ_COL_H__
#include "Read_Cell.h"
#include "Read_Sheet.h"

namespace Xunmei {
namespace Excel {

class Read_Col :public XM_Excel::Col
{
public:
	Read_Col(const shared_ptr<XM_Excel::Sheet>& sheet, uint32_t m_colNum);
	~Read_Col();
	//获取某一列中某一行的表格 
	shared_ptr<Read_Cell> cell(uint32_t rowNum);

	//--------------数据相关
	///获取数据,内容
	virtual shared_ptr<XM_Excel::Value> data(uint32_t rowNum) const;
	//--------------当前列的最大的列号
	uint32_t colNum();

	//--------------当前列的最大的行数
	uint32_t maxRowNum();
private:
	shared_ptr<XM_Excel::Sheet> sheet;
	uint32_t m_colNum;
};

}
}

#endif //__READ_COL_H__