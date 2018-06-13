
#include "Read_Sheet.h"

namespace Public {
namespace Excel {

	Read_Sheet::Read_Sheet(const shared_ptr<WorkBook>& _workbook, xlsWorkSheet *_pWorkSheet, const std::string &name):pWorkSheet(_pWorkSheet),workbook(_workbook), sheetname(name)
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
	shared_ptr<WorkBook::Row> Read_Sheet::row(uint32_t rowNum)
	{
		if (pWorkSheet == NULL)
		{
			return shared_ptr<WorkBook::Row>();
		}
		if ((int)rowNum > pWorkSheet->rows.lastrow + 1)
		{
			return shared_ptr<WorkBook::Row>();
		}
		//todo 当前只是用了str，后续需要根据实际的类型创建value
		return shared_ptr<WorkBook::Row>(new Read_Row(shared_from_this(), rowNum));

	}
	//获取某一列
	shared_ptr<WorkBook::Col> Read_Sheet::col(uint32_t colNum)
	{
		if (pWorkSheet == NULL)
		{
			return shared_ptr<WorkBook::Col>();
		}
		if ((int)colNum > pWorkSheet->rows.lastcol+1)
		{
			return shared_ptr<WorkBook::Col>();
		}
		//todo 当前只是用了str，后续需要根据实际的类型创建value
		return shared_ptr<WorkBook::Col>(new Read_Col(shared_from_this(), colNum));

	}
	//获取某一列中某一行的表格 
	shared_ptr<WorkBook::Cell> Read_Sheet::cell(uint32_t rowNum, uint32_t colNum)
	{
		if (pWorkSheet == NULL)
		{
			return shared_ptr<WorkBook::Cell>();
		}
		if ((int)rowNum > pWorkSheet->rows.lastrow + 1)
		{
			return shared_ptr<WorkBook::Cell>();
		}
		if ((int)colNum > (int)pWorkSheet->rows.row[rowNum].cells.count)
		{
			return shared_ptr<WorkBook::Cell>();
		}
		return shared_ptr<WorkBook::Cell>(new Read_Cell(shared_from_this(), rowNum, colNum));

	}
	///获取数据,内容
	shared_ptr<WorkBook::Value> Read_Sheet::data(uint32_t rowNum, uint32_t colNum) const
	{
		if (pWorkSheet == NULL)
		{
			return shared_ptr<WorkBook::Value>();
		}

		if ((int)rowNum >= pWorkSheet->rows.lastrow + 1)
		{
			return shared_ptr<WorkBook::Value>();
		}

		if ((int)colNum >= (int)pWorkSheet->rows.row[rowNum].cells.count)
		{
			return shared_ptr<WorkBook::Value>();
		}

		shared_ptr<WorkBook::Value> val;

		if (pWorkSheet->rows.row[rowNum].cells.cell[colNum].id == 638)//数字 638
		{
			if (pWorkSheet->rows.row[rowNum].cells.cell[colNum].d - (int)pWorkSheet->rows.row[rowNum].cells.cell[colNum].d == 0)
			{
				val = make_shared<WorkBook::Value>((int)pWorkSheet->rows.row[rowNum].cells.cell[colNum].d);
			}
			else
			{
				val = make_shared<WorkBook::Value>(pWorkSheet->rows.row[rowNum].cells.cell[colNum].d);
			}
		}
		else if (pWorkSheet->rows.row[rowNum].cells.cell[colNum].id == 513 || pWorkSheet->rows.row[rowNum].cells.cell[colNum].str == NULL) //bool 513
		{
			if (pWorkSheet->rows.row[rowNum].cells.cell[colNum].d > 0.0)
			{
				val = make_shared<WorkBook::Value>(true);
			}
			else
			{
				val = make_shared<WorkBook::Value>("");
			}
		}
		else
		{
			const char* excelstr = (const char*)pWorkSheet->rows.row[rowNum].cells.cell[colNum].str;
			val = make_shared<WorkBook::Value>(String::utf82ansi(excelstr == NULL ? "" : excelstr));
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