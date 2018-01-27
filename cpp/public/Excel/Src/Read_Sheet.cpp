
#include "Read_Sheet.h"

namespace Public {
namespace Excel {

	Read_Sheet::Read_Sheet(const shared_ptr<XM_Excel::WorkBook>& _workbook, xlsWorkSheet *_pWorkSheet, const std::string &name):pWorkSheet(_pWorkSheet),workbook(_workbook), sheetname(name)
	{
		
	}
	Read_Sheet::~Read_Sheet()
	{
		if (pWorkSheet != NULL)
		{
			xls_close_WS(pWorkSheet);
			pWorkSheet = NULL;
		}
	}
	std::string Read_Sheet::name()
	{
		return sheetname;
	}
	//获取某一行
	shared_ptr<XM_Excel::Row> Read_Sheet::row(uint32_t rowNum)
	{
		if (pWorkSheet == NULL)
		{
			return shared_ptr<XM_Excel::Row>();
		}
		if ((int)rowNum > pWorkSheet->rows.lastrow + 1)
		{
			return shared_ptr<XM_Excel::Row>();
		}
		//todo 当前只是用了str，后续需要根据实际的类型创建value
		return shared_ptr<XM_Excel::Row>(new Read_Row(shared_from_this(), rowNum));

	}
	//获取某一列
	shared_ptr<XM_Excel::Col> Read_Sheet::col(uint32_t colNum)
	{
		if (pWorkSheet == NULL)
		{
			return shared_ptr<XM_Excel::Col>();
		}
		if ((int)colNum > pWorkSheet->rows.lastcol+1)
		{
			return shared_ptr<XM_Excel::Col>();
		}
		//todo 当前只是用了str，后续需要根据实际的类型创建value
		return shared_ptr<XM_Excel::Col>(new Read_Col(shared_from_this(), colNum));

	}
	//获取某一列中某一行的表格 
	shared_ptr<XM_Excel::Cell> Read_Sheet::cell(uint32_t rowNum, uint32_t colNum)
	{
		if (pWorkSheet == NULL)
		{
			return shared_ptr<XM_Excel::Cell>();
		}
		if ((int)rowNum > pWorkSheet->rows.lastrow + 1)
		{
			return shared_ptr<XM_Excel::Cell>();
		}
		if ((int)colNum > (int)pWorkSheet->rows.row[rowNum].cells.count)
		{
			return shared_ptr<XM_Excel::Cell>();
		}
		return shared_ptr<XM_Excel::Cell>(new Read_Cell(shared_from_this(), rowNum, colNum));

	}
	///获取数据,内容
	shared_ptr<XM_Excel::Value> Read_Sheet::data(uint32_t rowNum, uint32_t colNum) const
	{
		if (pWorkSheet == NULL)
		{
			return shared_ptr<XM_Excel::Value>();
		}

		if ((int)rowNum >= pWorkSheet->rows.lastrow + 1)
		{
			return shared_ptr<XM_Excel::Value>();
		}

		if ((int)colNum >= (int)pWorkSheet->rows.row[rowNum].cells.count)
		{
			return shared_ptr<XM_Excel::Value>();
		}

		shared_ptr<XM_Excel::Value> val;

		if (pWorkSheet->rows.row[rowNum].cells.cell[colNum].id == 638)//数字 638
		{
			if (pWorkSheet->rows.row[rowNum].cells.cell[colNum].d - (int)pWorkSheet->rows.row[rowNum].cells.cell[colNum].d == 0)
			{
				val = new XM_Excel::Value((int)pWorkSheet->rows.row[rowNum].cells.cell[colNum].d);
			}
			else
			{
				val = new XM_Excel::Value(pWorkSheet->rows.row[rowNum].cells.cell[colNum].d);
			}
		}
		else if (pWorkSheet->rows.row[rowNum].cells.cell[colNum].id == 513 || pWorkSheet->rows.row[rowNum].cells.cell[colNum].str == NULL) //bool 513
		{
			if (pWorkSheet->rows.row[rowNum].cells.cell[colNum].d > 0.0)
			{
				val = new XM_Excel::Value(true);
			}
			else
			{
				val = new XM_Excel::Value("");
			}
		}
		else
		{
			const char* excelstr = (const char*)pWorkSheet->rows.row[rowNum].cells.cell[colNum].str;
			val = new XM_Excel::Value(excelstr);
#ifndef WIN32
			val = new XM_Excel::Value(String::utf82ansi(excelstr));
#endif
		}

		return val;
	}
	//--------------最大的行数
	uint32_t Read_Sheet::maxRowNum()
	{
		if (pWorkSheet == NULL)
		{
			return -1;
		}
		//实际excel的行数为lastrow+1
		return pWorkSheet->rows.lastrow + 1;
	}
	//--------------最大的列数
	uint32_t Read_Sheet::maxColNum()
	{
		if (pWorkSheet == NULL)
		{
			return -1;
		}

		return pWorkSheet->rows.lastcol;
	}

	uint32_t Read_Sheet::getRowMaxColNum(uint32_t rownum)
	{
		if (pWorkSheet == NULL)
		{
			return -1;
		}

		if ((int)rownum > pWorkSheet->rows.lastrow + 1)
		{
			return -1;
		}
		return pWorkSheet->rows.row[rownum].cells.count;
	}

	uint32_t Read_Sheet::getColMaxRowNum(uint32_t colnum)
	{
		if (pWorkSheet == NULL)
		{
			return -1;
		}

		return pWorkSheet->rows.lastrow + 1;

	}


}
}