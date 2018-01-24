﻿#ifndef __READSHEET_H__
#define __READSHEET_H__
#include "Excel/Excel.h"
#include "Read_Row.h"
#include "Read_Col.h"

namespace Public {
namespace Excel {

	class  Read_Sheet :public  XM_Excel::Sheet, public enable_shared_from_this<XM_Excel::Sheet>
	{
	public:
		Read_Sheet(const shared_ptr<XM_Excel::WorkBook>& workbook, xlsWorkSheet *pWorkSheet, const std::string &name);
		~Read_Sheet();
		std::string name();
		//获取某一行
		shared_ptr<XM_Excel::Row> row(uint32_t rowNum);
		//获取某一列
		shared_ptr<XM_Excel::Col> col(uint32_t colNum);
		//获取某一列中某一行的表格 
		shared_ptr<XM_Excel::Cell> cell(uint32_t rowNum, uint32_t colNum);
		//获取数据,内容
		shared_ptr<XM_Excel::Value> data(uint32_t rowNum, uint32_t colNum) const;

		//--------------最大的行数
		uint32_t maxRowNum();
		//--------------最大的列数
		uint32_t maxColNum();

		uint32_t getRowMaxColNum(uint32_t rownum);
		uint32_t getColMaxRowNum(uint32_t colnum);
	private:
		xlsWorkSheet *pWorkSheet;
		shared_ptr<XM_Excel::WorkBook>	workbook;
		std::string sheetname;
	};

}
}

#endif //__READSHEET_H__