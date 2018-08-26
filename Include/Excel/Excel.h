#ifndef __Public_EXCEL_H__
#define __Public_EXCEL_H__
#include "Excel_Def.h"

namespace Public {
namespace Excel {

//excel 的工作簿
class EXCEL_API WorkBook
{
public:
	//excel中的颜色
	class EXCEL_API Color
	{
	public:
		Color();
		Color(const Color& color);
		Color(uint8_t _r, uint8_t _g, uint8_t _b);
		virtual ~Color();

		Color& operator = (const Color& val);
	public:
		struct ColorInternal;
		ColorInternal* internal;
	};

	//excel中的字体
	class EXCEL_API Font
	{
	public:
		Font();
		Font(const Font& font);
		virtual ~Font();

		Font& operator = (const Font& val);

		static shared_ptr<Font> create(const std::string& name);

		//设置字体大小
		bool setSize(uint32_t size);
		//字体加粗
		bool setBold();
		//字体下划线
		bool setUnderline();
		//设置颜色
		bool setColor(const Color& color);
		bool setColor(const shared_ptr<Color>& color);
	public:
		struct FontInternal;
		FontInternal* internal;
	};

	//excel中的显示格式
	class EXCEL_API Format
	{
	public:
		Format();
		Format(const Format& format);
		Format& operator = (const Format& val);
		virtual ~Format();

		//显示显示格式
		bool setFormat(FomatType type);

		//设置对齐方式
		bool setAlign(ALIGN_X_Type xalign, ALIGN_Y_Type yalign);

		//设置显示方式
		bool setTxtori(TxtoriType type);
	public:
		struct FormatInternal;
		FormatInternal* internal;
	};



	//excel中的单元格线
	class EXCEL_API Side
	{
	public:
		//设置边框显示类型,参见SIDE_BOTTOM
		Side(int val, const shared_ptr<Color>& sideColor);
		Side(int val, const Color& sideColor);
		Side(const Side& side);
		Side& operator = (const Side& val);
		~Side();
		//设置边框显示类型,参见SIDE_BOTTOM
		//		bool setType(xlslib_core::border_side_t side, xlslib_core::border_style_t style);
		/*virtual bool borderstyle(xlslib_core::border_side_t side, xlslib_core::border_style_t style);
		virtual bool bordercolor(xlslib_core::border_side_t side, unsigned8_t color);
		*/
	public:
		struct LineInternal;
		LineInternal* internal;
	};

	//excel中的某一个单元格
	class EXCEL_API Cell
	{
	public:
		Cell() {}
		virtual ~Cell() {}

		//--------------数据相关
		///获取数据,内容
		virtual shared_ptr<Value> data() const { return shared_ptr<Value>(); };

		//--------------行数相关
		///行号
		virtual unsigned int rowNum() = 0;
		///列号
		virtual unsigned int colNum() = 0;

		//--------------字体相关		
		virtual bool font(const shared_ptr<Font>& font) { return false; }

		//--------------填充颜色
		virtual bool fill(const Color& color) { return false; }
		virtual bool fill(const shared_ptr<Color>& color) { return false; }

		//--------------显示格式
		virtual bool format(const Format& fmt) { return false; }
		virtual bool format(const shared_ptr<Format>& fmt) { return false; }

		//--------------单元格边框
		virtual bool side(const Side& sd) { return false; }
		virtual bool side(const shared_ptr<Side>& sd) { return false; }
	};

	///excel中多行多列选择区域
	class EXCEL_API Range
	{
	public:
		Range() {}
		virtual ~Range() {}

		//--------------字体相关		
		virtual bool font(const shared_ptr<Font>& font) { return false; }

		//--------------填充颜色
		virtual bool fill(const Color& color) { return false; }
		virtual bool fill(const shared_ptr<Color>& color) { return false; }

		//--------------显示格式
		virtual bool format(const Format& fmt) { return false; }
		virtual bool format(const shared_ptr<Format>& fmt) { return false; }

		//--------------单元格边框
		virtual bool side(const Side& sd) { return false; }
		virtual bool side(const shared_ptr<Side>& sd) { return false; }

		////--------------显示，隐藏,整区域的隐藏显示
		//virtual void hidden(bool hidden_opt) {};

		//--------------合并,指定单元格
		virtual bool merge() { return false; }
	public:
		uint32_t startRowNum;
		uint32_t startColNum;
		uint32_t stopRowNum;
		uint32_t stopColNum;
	};
	//excel中的行,用于按行处理
	class EXCEL_API Row
	{
	public:
		Row() {}
		virtual ~Row() {}

		//--------------数据相关
		///获取数据,内容
		virtual shared_ptr<Value> data(uint32_t colNum) const { return shared_ptr<Value>(); };
		///设置数据，内容
		virtual shared_ptr<Cell> setData(uint32_t colNum, const shared_ptr<Value>& val) { return shared_ptr<Cell>(); };
		virtual shared_ptr<Cell> setData(uint32_t colNum, const Value& val) { return shared_ptr<Cell>(); };

		//--------------字体相关		
		virtual bool font(const shared_ptr<Font>& font) { return false; }

		//--------------填充颜色
		virtual bool fill(const Color& color) { return false; }
		virtual bool fill(const shared_ptr<Color>& color) { return false; }

		//--------------显示格式
		virtual bool format(const Format& fmt) { return false; }
		virtual bool format(const shared_ptr<Format>& fmt) { return false; }

		//--------------区域，单行的指定列合并, 开始的列号，结束的列号
		virtual shared_ptr<Range> range(uint32_t startColNum, uint32_t stopColNum) { return shared_ptr<Range>(); }

		//--------------显示，隐藏,整行的隐藏显示
		//	virtual bool show(bool showflag) { return false; }

		//--------------当前行最大的列数
		virtual uint32_t maxColNum() = 0;

		//--------------当前行的行号
		virtual uint32_t rowNum() = 0;

		//行高
		virtual uint32_t height() { return 0; }
		virtual bool setHeight(uint32_t height) { return false; }
	};

	//excel中的列，用于按列处理
	class EXCEL_API Col
	{
	public:
		Col() {}
		virtual ~Col() {}

		//--------------数据相关
		///获取数据,内容
		virtual shared_ptr<Value> data(uint32_t rowNum) const { return shared_ptr<Value>(); };
		///设置数据，内容
		virtual shared_ptr<Cell> setData(uint32_t rowNum, const shared_ptr<Value>& val) { return shared_ptr<Cell>(); };
		virtual shared_ptr<Cell> setData(uint32_t rowNum, const Value& val) { return shared_ptr<Cell>(); };

		//--------------字体相关		
		virtual bool font(const shared_ptr<Font>& font) { return false; }

		//--------------填充颜色
		virtual bool fill(const Color& color) { return false; }
		virtual bool fill(const shared_ptr<Color>& color) { return false; }

		//--------------显示格式
		virtual bool format(const Format& fmt) { return false; }
		virtual bool format(const shared_ptr<Format>& fmt) { return false; }

		//--------------区域，单行的指定行合并, 开始的行号，结束的行号
		virtual shared_ptr<Range> range(uint32_t startRowNum, uint32_t stopRowNum) { return shared_ptr<Range>(); }

		//--------------显示，隐藏,整行的隐藏显示
		//	virtual bool show(bool showflag) { return false; }

		//--------------当前列的列号
		virtual uint32_t colNum() = 0;

		//--------------当前列的最大的行数
		virtual uint32_t maxRowNum() = 0;

		//列宽
		//virtual uint32_t width() { return 0; }
		//virtual bool setWidth(uint32_t height) { return false; }
	};

	//excel中的sheet
	class EXCEL_API Sheet
	{
	public:
		Sheet() {}
		virtual ~Sheet() {}

		//sheet名称
		virtual std::string name() { return ""; }

		//--------------最大的行数
		virtual uint32_t maxRowNum() = 0;
		//--------------最大的列数
		virtual uint32_t maxColNum() = 0;

		//获取某一行
		virtual shared_ptr<Row> row(uint32_t rowNum) = 0;
		//获取某一列
		virtual shared_ptr<Col> col(uint32_t colNum) = 0;

		virtual shared_ptr<Cell> cell(uint32_t rowNum, uint32_t colNum) = 0;

		//--------------数据相关
		///获取数据,内容
		virtual shared_ptr<Value> data(uint32_t rowNum, uint32_t colNum) const { return shared_ptr<Value>(); };
		///设置数据，内容
		virtual shared_ptr<Cell> setData(uint32_t rowNum, uint32_t colNum, const shared_ptr<Value>& val) { return shared_ptr<Cell>(); };
		virtual shared_ptr<Cell> setData(uint32_t rowNum, uint32_t colNum, const Value& val) { return shared_ptr<Cell>(); };

		//--------------字体相关		
		virtual bool font(const shared_ptr<Font>& font) { return false; }

		//--------------填充颜色
		virtual bool fill(const Color& color) { return false; }
		virtual bool fill(const shared_ptr<Color>& color) { return false; }

		//--------------显示格式
		virtual bool format(const Format& fmt) { return false; }
		virtual bool format(const shared_ptr<Format>& fmt) { return false; }

		//--------------合并
		virtual shared_ptr<Range> range(uint32_t startRowNum, uint32_t startColNum, uint32_t stopRowNum, uint32_t stopColNum) { return shared_ptr<Range>(); }

		//--------------默认宽、高
		virtual bool defaultRowHeight(uint16_t height) { return false; } // sets column widths to 1/256 x width of "0"
		virtual bool defaultColwidth(uint16_t width) { return false; }  // in points (Excel uses twips, 1/20th of a point, but xlslib didn't)

	};

public:
	WorkBook() {}
	virtual ~WorkBook() {}

	//创建一个sheet
	virtual shared_ptr<Sheet> addSheet(const std::string& name) {return shared_ptr<Sheet>();}

	//获取一个sheet,num从0开始
	virtual shared_ptr<Sheet> getSheet(uint32_t num) { return shared_ptr<Sheet>(); }

	virtual uint32_t sheetCount() { return 0; }


	///读取一个xls文件
	static shared_ptr<WorkBook> read(const std::string& xlsfile);

	//创建一个xls文件
	static shared_ptr<WorkBook> create(const std::string& xlsfile);
};

	

}
}


#endif //__Public_EXCEL_H__
