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
	//��ȡĳһ����ĳһ�еı�� 
	shared_ptr<Read_Cell> cell(uint32_t rowNum);

	//--------------�������
	///��ȡ����,����
	virtual shared_ptr<XM_Excel::Value> data(uint32_t rowNum) const;
	//--------------��ǰ�е������к�
	uint32_t colNum();

	//--------------��ǰ�е���������
	uint32_t maxRowNum();
private:
	shared_ptr<XM_Excel::Sheet> sheet;
	uint32_t m_colNum;
};

}
}

#endif //__READ_COL_H__